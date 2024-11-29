#include <cstddef>

#include <config-memory.hpp>
#include <hal/memory.hpp>
#include <libk/assert.hpp>
#include <libk/logging.hpp>
#include <libk/memory.hpp>
#include <libk/mutex.hpp>

#include "memory.hpp"
#include "paging.hpp"
#include "registers.hpp"

static_assert(hal::page_size == PAGE_SIZE);

namespace cpu
{

namespace x86
{

libk::spin_mutex_t physical_page_mutex;
std::size_t num_physical_pages = 0;
physical_page_entry_t *physical_pages = nullptr;

void set_physical_page_list_address(void *address) noexcept
{
    libk::SpinLock lock(physical_page_mutex);

    assert(physical_pages == nullptr);

    physical_pages = reinterpret_cast<physical_page_entry_t *>(address);
}

void * gpf_address() noexcept
{
    return read_cr2();
}

} // namespace x86

} // namespace cpu

namespace hal
{

using namespace cpu::x86;

void * alloc_physical() noexcept
{
    libk::SpinLock lock(physical_page_mutex);

    assert(physical_pages != nullptr);

    if (num_physical_pages == 0) return nullptr;

    num_physical_pages--;

    auto physical_address = physical_pages[num_physical_pages].address;
    return physical_address;
}

void free_physical(void *page) noexcept
{
    {
        libk::DisableInteruptsPaging temp_disabler;

        libk::clear_memory(page, page_size);
    }

    libk::SpinLock lock(physical_page_mutex);

    assert(physical_pages != nullptr);

    physical_pages[num_physical_pages] = {
        .address = page,
    };

    num_physical_pages++;
}

std::size_t available_physical_pages() noexcept
{
    libk::SpinLock lock(physical_page_mutex);

    return num_physical_pages;
}

bool physical_page_available(const void *physical_page) noexcept
{
    assert(libk::is_page_aligned(physical_page));

    libk::SpinLock lock(physical_page_mutex);

    for (size_t i = 0; i < num_physical_pages; i++) {
        if (physical_pages[i].address == physical_page) return true;
    }

    return false;
}

} // namespace hal
