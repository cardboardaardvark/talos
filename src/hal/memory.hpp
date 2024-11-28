#pragma once

#include <cstddef>
#include <cstdint>

#include <config-memory.hpp>

#define PAGE_FLAG_NONE 0
#define PAGE_FLAG_USER (1 << 0)
#define PAGE_FLAG_RW (1 << 1)
#define PAGE_FLAG_PRESENT (1 << 2)

namespace hal
{

typedef std::uint_fast8_t page_flags_t;
typedef bool (page_directory_visitor_t)(page_directory_t, void *args);

extern "C" const page_directory_t kernel_page_directory;

void * alloc_physical() noexcept;
void free_physical(void *page) noexcept;
std::size_t available_physical_pages() noexcept;
std::size_t total_physical_pages() noexcept;
void set_kernel_page_directory(page_directory_t directory) noexcept;

void enable_paging(bool enabled) noexcept;
bool paging_enabled() noexcept;
void set_page_directory(page_directory_t directory) noexcept;
page_directory_t get_page_directory() noexcept;
void map_virtual_page(page_directory_t directory, const void *virtual_page, const void *physical_page, page_flags_t flags) noexcept;
bool unmap_virtual_page(page_directory_t directory, const void *virtual_page) noexcept;
void flush_virtual_page(const void *page) noexcept;
void flush_tlb() noexcept;
size_t heap_size() noexcept;

} // namespace hal
