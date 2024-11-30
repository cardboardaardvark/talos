#include <cstdint>
#include <cstring>
#include <list>
#include <map>

#include <hal/interrupt.hpp>
#include <hal/memory.hpp>
#include <libk/mutex.hpp>

#include "error.hpp"
#include "memory.hpp"
#include "mutex.hpp"

namespace libk
{

DisableInteruptsPaging::DisableInteruptsPaging() noexcept
: paging_was_enabled(hal::paging_enabled()), interrupts_were_enabled(hal::interrupts_enabled())
{
    if (interrupts_were_enabled) hal::enable_interrupts(false);
    if (paging_was_enabled) hal::enable_paging(false);
}

DisableInteruptsPaging::~DisableInteruptsPaging() noexcept
{
    if (paging_was_enabled) hal::enable_paging(true);
    if (interrupts_were_enabled) hal::enable_interrupts(true);
}

memory_status_t memory_status() noexcept
{
    return {
        .physical = {
            .total_pages = hal::total_physical_pages(),
            .available_pages = hal::available_physical_pages(),
        },
        .paged = {
            .heap_size = hal::heap_size(),
        },
    };
}

void free_physical(void *page) noexcept
{
    hal::free_physical(page);
}

void clear_memory(void *address, std::size_t bytes) noexcept
{
    std::memset(address, 0, bytes);
}

bool is_page_aligned(uintptr_t address) noexcept
{
    return address % hal::page_size == 0;
}

bool is_page_aligned(const void *address) noexcept
{
    return is_page_aligned(reinterpret_cast<std::uintptr_t>(address));
}

uintptr_t align_page(uintptr_t address) noexcept
{
    return address - address % hal::page_size;
}

void * align_page(void *address) noexcept
{
    return reinterpret_cast<void *>(align_page(reinterpret_cast<uint32_t>(address)));
}

const void * align_page(const void *address) noexcept
{
    return reinterpret_cast<const void *>(align_page(reinterpret_cast<uint32_t>(address)));
}

uintptr_t map_physical_page(hal::page_directory_t directory, std::uintptr_t physical_page, hal::page_flags_t flags) noexcept
{
    return reinterpret_cast<uintptr_t>(map_physical_page(directory, reinterpret_cast<void *>(physical_page), flags));
}

void * map_physical_page(hal::page_directory_t directory, const void *physical_page, hal::page_flags_t flags) noexcept
{
    return hal::map_physical_page(directory, physical_page, flags);
}

uintptr_t map_physical_address(hal::page_directory_t directory, std::uintptr_t physical_address, size_t length, hal::page_flags_t flags) noexcept
{
    return reinterpret_cast<uintptr_t>(hal::map_physical_address(directory, reinterpret_cast<const void *>(physical_address), length, flags));
}

void * map_physical_address(hal::page_directory_t directory, const void *physical_address, size_t length, hal::page_flags_t flags) noexcept
{
    return hal::map_physical_address(directory, physical_address, length, flags);
}

void map_virtual_page(hal::page_directory_t directory, const void *virtual_page, const void *physical_page, hal::page_flags_t flags) noexcept
{
    return hal::map_virtual_page(directory, virtual_page, physical_page, flags);
}

void map_virtual_page(hal::page_directory_t directory, std::uintptr_t virtual_page, std::uintptr_t physical_page, hal::page_flags_t flags) noexcept
{
    return hal::map_virtual_page(directory, reinterpret_cast<const void *>(virtual_page), reinterpret_cast<const void *>(physical_page), flags);
}

void map_identity_page(hal::page_directory_t directory, const void *physical_page, hal::page_flags_t flags) noexcept
{
    return map_virtual_page(directory, physical_page, physical_page, flags);
}

void map_identity_page(hal::page_directory_t directory, std::uintptr_t physical_page, hal::page_flags_t flags) noexcept
{
    return map_virtual_page(directory, physical_page, physical_page, flags);
}

void map_guard_page(hal::page_directory_t directory, const void *virtual_page) noexcept
{
    return map_virtual_page(directory, virtual_page, nullptr, hal::page_flag_none);
}

void map_guard_page(hal::page_directory_t directory, std::uintptr_t virtual_page) noexcept
{
    return map_virtual_page(directory, virtual_page, reinterpret_cast<uintptr_t>(nullptr), hal::page_flag_none);
}

// void unmap_virtual_page(hal::page_directory_t directory, const void *virtual_page) noexcept;
// void unmap_virtual_page(hal::page_directory_t directory, std::uintptr_t virtual_page) noexcept;
// void flush_virtual_page(const void *page) noexcept;
// void flush_virtual_page(uintptr_t page) noexcept;

} // namespace libk
