#pragma once

#include <cstddef>
#include <type_traits>
#include <new>

#include <hal/memory.hpp>

namespace libk
{

typedef struct
{
    size_t total_pages = 0;
    size_t available_pages = 0;
} physical_memory_status_t;

typedef struct
{
    // given as number of bytes
    size_t heap_size = 0;
} virtual_memory_status_t;

typedef struct
{
    physical_memory_status_t physical;
    virtual_memory_status_t paged;
} memory_status_t;

class DisableInteruptsPaging
{
    private:
    const bool paging_was_enabled;
    const bool interrupts_were_enabled;

    public:
    DisableInteruptsPaging() noexcept;
    DisableInteruptsPaging(const DisableInteruptsPaging&) = delete;
    DisableInteruptsPaging& operator=(const DisableInteruptsPaging&) = delete;
    ~DisableInteruptsPaging() noexcept;
};

template <typename T = void *>
T alloc_physical()
{
    auto address = reinterpret_cast<T>(hal::alloc_physical());

    if (address == nullptr) throw std::bad_alloc();

    return address;
}

memory_status_t memory_status() noexcept;

void free_physical(void *page) noexcept;
void clear_memory(void *address, std::size_t bytes) noexcept;
bool is_page_aligned(const void *address) noexcept;
void * align_page(void *address) noexcept;
const void * align_page(const void * address) noexcept;

void map_virtual_page(hal::page_directory_t directory, const void *virtual_page, const void *physical_page, hal::page_flags_t flags) noexcept;
void map_virtual_page(hal::page_directory_t directory, std::uintptr_t virtual_page, std::uintptr_t physical_page, hal::page_flags_t flags) noexcept;
void map_identity_page(hal::page_directory_t directory, const void *physical_page, hal::page_flags_t flags) noexcept;
void map_identity_page(hal::page_directory_t directory, std::uintptr_t physical_page, hal::page_flags_t flags) noexcept;
void map_guard_page(hal::page_directory_t directory, const void *virtual_page) noexcept;
void map_guard_page(hal::page_directory_t directory, std::uintptr_t virtual_page) noexcept;
void unmap_virtual_page(hal::page_directory_t directory, const void *virtual_page) noexcept;
void unmap_virtual_page(hal::page_directory_t directory, std::uintptr_t virtual_page) noexcept;
void flush_virtual_page(const void *page) noexcept;
void flush_virtual_page(uintptr_t page) noexcept;

}
