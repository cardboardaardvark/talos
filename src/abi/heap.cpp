#include <cerrno>
#include <cstddef>
#include <unistd.h>

#include <libk/assert.hpp>
#include <libk/error.hpp>
#include <libk/logging.hpp>
#include <libk/memory.hpp>
#include <libk/mutex.hpp>
#include <libk/util.hpp>

#include "heap.hpp"

// Maximum number of pages to extend by when using
// an array of pointers allocated on the stack.
#define STACK_ALLOCATE_MAX 128
#define ALLOCATION_LIST_MAX_PAGES ((PAGE_SIZE - sizeof(struct allocation_list_s*) - sizeof(size_t)) / sizeof(void*))

namespace abi
{

struct allocation_list_s
{
    allocation_list_s* next = nullptr;
    size_t page_count = 0;

    void* pages[ALLOCATION_LIST_MAX_PAGES];
};

using allocation_list_t = struct allocation_list_s;

static_assert(sizeof(allocation_list_t) <= PAGE_SIZE);

libk::spin_mutex_t heap_mutex;
void *heap_start = nullptr; // The start of the heap
void *heap_end = nullptr; // The current program break
void *heap_next_allocation = nullptr; // The end of the mapped address space

static inline void extend_heap_allocated(void *physical_page) noexcept
{
    assert(hal::kernel_page_directory != nullptr);

    auto old_allocated = heap_next_allocation;

    heap_next_allocation = reinterpret_cast<void *>(reinterpret_cast<std::uintptr_t>(heap_next_allocation) + PAGE_SIZE);
    libk::map_virtual_page(hal::kernel_page_directory, old_allocated, physical_page, PAGE_FLAG_PRESENT | PAGE_FLAG_RW);
}

static bool extend_using_stack(std::size_t num_pages) noexcept
{
    assert_locked_mutex(heap_mutex);
    assert(num_pages <= STACK_ALLOCATE_MAX);

    void * page_list[STACK_ALLOCATE_MAX];
    std::size_t num_allocated = 0;

    libk::Finally cleanup_guard([&] {
        for (std::size_t page_num = 0; page_num < num_allocated; num_allocated++) {
            libk::free_physical(page_list[page_num]);
        }
    });

    for(; num_allocated < num_pages; num_allocated++) {
        auto physical_page = hal::alloc_physical();

        if (physical_page == nullptr) return false;

        page_list[num_allocated] = physical_page;
    }

    assert(num_allocated == num_pages);

    for (std::size_t page_num = 0; page_num < num_allocated; page_num++) {
        extend_heap_allocated(page_list[page_num]);
    }

    cleanup_guard.disarm();
    return true;
}

static inline allocation_list_t * make_allocation_list_member() noexcept
{
    return reinterpret_cast<allocation_list_t *>(hal::alloc_physical());
}

static inline bool add_page(allocation_list_t* list) noexcept
{
    assert(list->page_count < ALLOCATION_LIST_MAX_PAGES);

    auto page = hal::alloc_physical();

    if (page == nullptr) return false;

    list->pages[list->page_count] = page;
    list->page_count++;

    return true;
}

static bool extend_using_pages(size_t num_pages) noexcept
{
    assert_locked_mutex(heap_mutex);

    allocation_list_t *list_head = make_allocation_list_member();
    allocation_list_t *list_current = list_head;

    if (list_head == nullptr) return false;

    libk::DisableInteruptsPaging paging_guard;

    libk::Finally cleanup_members([&] {
        list_current = list_head;

        while (list_current != nullptr) {
            auto next = list_current->next;

            hal::free_physical(list_current);
            list_current = next;
        }
    });

    libk::Finally cleanup_pages([&] {
        for (list_current = list_head; list_current != nullptr; list_current = list_current->next) {
            for (size_t i = 0; i < list_current->page_count; i++) {
                hal::free_physical(list_current->pages[i]);
            }
        }
    });

#ifndef NDEBUG
    size_t num_allocated = 0;
#endif

    // Allocate all the pages that will be needed
    for (size_t i = 0; i < num_pages; i++) {
        if (list_current->page_count >= ALLOCATION_LIST_MAX_PAGES) {
            auto next = make_allocation_list_member();

            if (next == nullptr) return false;

            assert(list_current->next == nullptr);
            list_current->next = next;
            list_current = next;
        }

        if (! add_page(list_current)) return false;

#ifndef NDEBUG
        num_allocated++;
#endif
    }

    assert(num_allocated == num_pages);

#ifndef NDEBUG
    size_t num_mapped = 0;
#endif

    for (list_current = list_head; list_current != nullptr; list_current = list_current->next) {
        for (size_t i = 0; i < list_current->page_count; i++) {
            extend_heap_allocated(list_current->pages[i]);

#ifndef NDEBUG
            num_mapped++;
#endif
        }
    }

    assert(num_mapped == num_pages);

    cleanup_pages.disarm();
    return true;
}

static bool extend_address_space(std::size_t num_pages) noexcept
{
    assert_locked_mutex(heap_mutex);
    assert(libk::is_page_aligned(heap_next_allocation));

    // No need to do anything if it is not possible to get the number
    // of physical pages needed.
    if (hal::available_physical_pages() < num_pages) return false;

    if (num_pages <= STACK_ALLOCATE_MAX) return extend_using_stack(num_pages);

    return extend_using_pages(num_pages);
}

static void * increase_break(ptrdiff_t increment)
{
    assert_locked_mutex(heap_mutex);

    auto new_end = reinterpret_cast<void *>(reinterpret_cast<std::uintptr_t>(heap_end) + increment);
    auto old_end = heap_end;

    if (new_end > heap_next_allocation) {
        size_t need_bytes = reinterpret_cast<uintptr_t>(new_end) - reinterpret_cast<uintptr_t>(heap_next_allocation);
        size_t need_pages = need_bytes / PAGE_SIZE;

        if (need_bytes % PAGE_SIZE != 0) need_pages++;

        if (! extend_address_space(need_pages)) {
            errno = ENOMEM;
            return reinterpret_cast<void *>(-1);
        }
    }

    assert(new_end <= heap_next_allocation);
    heap_end = new_end;

    return old_end;
}

static void * decrease_break(ptrdiff_t decrement)
{
    assert_locked_mutex(heap_mutex);
    assert(libk::is_page_aligned(heap_next_allocation));
    assert(hal::kernel_page_directory != nullptr);

    auto new_end = reinterpret_cast<uintptr_t>(heap_end) - decrement;

    if (new_end < reinterpret_cast<uintptr_t>(heap_start)) {
        errno = EINVAL;
        return reinterpret_cast<void *>(-1);
    }

    auto need_bytes = reinterpret_cast<uintptr_t>(heap_next_allocation) - new_end;
    auto need_pages = need_bytes / PAGE_SIZE;

    for (size_t i = 0; i < need_pages; i++) {
        heap_next_allocation = reinterpret_cast<void *>(reinterpret_cast<intptr_t>(heap_next_allocation) - PAGE_SIZE);
        if (! hal::unmap_virtual_page(hal::kernel_page_directory, heap_next_allocation)) libk::panic("Expected kernel virtual address 0x%p to have been mapped\n", heap_next_allocation);
    }

    auto old_end = heap_end;
    heap_end = reinterpret_cast<void *>(new_end);

    return old_end;
}

extern "C" void * sbrk([[maybe_unused]] ptrdiff_t change)
{
    libk::SpinLock lock(heap_mutex);

    assert(heap_start != nullptr);
    assert(heap_end != nullptr);
    assert(heap_next_allocation != nullptr);
    assert(hal::kernel_page_directory != nullptr);

    if (change > 0) return increase_break(change);
    else if (change < 0) return decrease_break(change * -1);

    return heap_end;
}

void init_heap(void *heap_init) noexcept
{
    assert(libk::is_page_aligned(heap_start));

    libk::SpinLock lock(heap_mutex);

    assert(heap_start== nullptr);
    assert(heap_end == nullptr);
    assert(heap_next_allocation == nullptr);

    heap_next_allocation = heap_end = heap_start = heap_init;
}

} // namespace abi

namespace hal
{

using namespace abi;

size_t heap_size() noexcept
{
    libk::SpinLock lock(heap_mutex);

    return reinterpret_cast<uintptr_t>(heap_next_allocation) - reinterpret_cast<uintptr_t>(heap_start);
}

} // namespace hal
