#include <libk/assert.hpp>
#include <libk/logging.hpp>
#include <libk/memory.hpp>
#include <platform/ibmpc/link.hpp>

#include "paging.hpp"
#include "registers.hpp"

namespace cpu
{

namespace x86
{

static inline size_t directory_entry_index(const uintptr_t virtual_page) noexcept
{
    return (virtual_page & page_directory_entry_mask) >> page_directory_entry_shift;
}

static inline size_t directory_entry_index(const void* virtual_page) noexcept
{
    return directory_entry_index(reinterpret_cast<uint32_t>(virtual_page));
}

static inline size_t table_entry_index(const uintptr_t virtual_page) noexcept
{
    return (virtual_page & page_table_entry_mask) >> page_table_entry_shift;
}

static inline size_t table_entry_index(const void* virtual_page) noexcept
{
    return table_entry_index(reinterpret_cast<uint32_t>(virtual_page));
}

static void create_page_directory_table(page_directory_t directory, const uintptr_t virtual_page)
{
    assert(libk::is_page_aligned(virtual_page));

    auto new_table = libk::alloc_physical<uint32_t*>();
    auto directory_index = directory_entry_index(virtual_page);

    // Yes pages from alloc_physical() are supposed to be page aligned
    // and a page aligned address will never have bits true in the page offset.
    // Might as well double check it with the assert() and leave a note that
    // the assignment assumes the page offset is actually 0.
    assert((reinterpret_cast<uint32_t>(new_table) & page_offset_mask) == 0);
    directory[directory_index] = reinterpret_cast<uint32_t>(new_table) | page_entry_flag_present | page_entry_flag_rw | page_entry_flag_user;
}

static void create_page_directory_table(page_directory_t directory, const void *virtual_page)
{
    create_page_directory_table(directory, reinterpret_cast<uintptr_t>(virtual_page));
}

static void * find_free_virtual_page(page_directory_t directory)
{
    uintptr_t search_start = reinterpret_cast<uintptr_t>(&platform::ibmpc::_shared_start_virtual);
    uintptr_t search_end = reinterpret_cast<uintptr_t>(hal::next_heap_allocation());

    for (uintptr_t address = search_start; address >= search_end; address -= hal::page_size) {
        auto directory_index = directory_entry_index(address);
        auto table_index = table_entry_index(address);

        if (! (directory[directory_index] & page_entry_flag_present)) {
            create_page_directory_table(directory, address);
        }

        auto table = reinterpret_cast<const page_table_entry_t *>(directory[directory_index] & page_frame_mask);

        if (! (table[table_index] & page_entry_flag_inuse)) {
            libk::printf("Found free virtual page: 0x%x\n", address);
            return reinterpret_cast<void *>(address);
        }
    }

    return nullptr;
}

// The visitor will be called for each page directory entry with the table entry set to the null pointer.
// If the page directory has the entry present flag set then the visitor will be called for each table entry as well.
// The visitor will only be called again if it returns the true value from each visit.
void visit_page_tables(const page_directory_t directory, const page_table_visitor_t visitor, void *visitor_args)
{
    assert(directory != nullptr);

    for (size_t directory_index = 0; directory_index < num_page_entries; directory_index++) {
        if (! visitor(&directory[directory_index], nullptr, visitor_args)) return;

        if (directory[directory_index] & page_entry_flag_present) {
            auto table = reinterpret_cast<const page_table_entry_t *>(directory[directory_index] & page_frame_mask);

            for (size_t table_index = 0; table_index < num_page_entries; table_index++) {
                if (! visitor(&directory[directory_index], &table[table_index], visitor_args)) return;
            }
        }
    }
}

bool dump_page_directory_visitor(const page_directory_entry_t *directory_entry, const page_table_entry_t *table_entry, void *)
{
    if (! (*directory_entry & page_entry_flag_present)) return true;

    if (table_entry == nullptr) {
        libk::printf("Page directory entry: table_address=0x%p flags=0x%x\n", *directory_entry & page_frame_mask, *directory_entry & page_flag_mask);
    } else if (*table_entry & page_entry_flag_present) {
        libk::printf("  Table entry: physical_address=0x%p flags=0x%x\n", *table_entry & page_frame_mask, *table_entry & page_flag_mask);
    }

    return true;
}

void dump_page_directory(const page_directory_t directory)
{
    visit_page_tables(directory, dump_page_directory_visitor, nullptr);
}

} // namespace x86

} // namespace cpu

namespace hal
{

using namespace cpu::x86;

const page_directory_t kernel_page_directory = nullptr;

void set_kernel_page_directory(page_directory_t directory) noexcept
{
    assert(directory != nullptr);

    if (kernel_page_directory != nullptr) libk::panic("Attempt to double initialize kernel global page directory\n");

    // You should never ever ever cast away constness. But also. This needs to be
    // set to a value that is determined after the program begins running. And after
    // it is set once it should never be changed again. It's a good idea to set it const
    // so other parts of the kernel can't accidently modify it as it is a global variable.
    // So casting away const seems like an acceptable evil.
    *const_cast<page_directory_t *>(&kernel_page_directory) = directory;

    write_cr3(reinterpret_cast<uint32_t>(directory));
}

void enable_paging(bool enabled) noexcept
{
    auto cr0 = read_cr0();

    if (enabled) {
        cr0 |= enable_paging_flag;
    } else {
        cr0 &= ~enable_paging_flag;
    }

    write_cr0(cr0);
}

bool paging_enabled() noexcept
{
    return read_cr0() & enable_paging_flag;
}

void * map_physical_page(page_directory_t directory, const void *physical_page, page_flags_t flags) noexcept
{
    assert(libk::is_page_aligned(physical_page));
    // If the physical page exists in the free physical page list then an attempt to map it is surely a mistake
    assert(! physical_page_available(physical_page));

    auto virtual_page = find_free_virtual_page(directory);

    if (virtual_page == nullptr) return nullptr;

    libk::panic("here 0x%x\n", flags);
}

void map_virtual_page(page_directory_t directory, const void* virtual_page, const void* physical_page, page_flags_t hal_flags) noexcept
{
    assert(directory != nullptr);
    assert(libk::is_page_aligned(virtual_page));
    assert(libk::is_page_aligned(physical_page));
    // If the physical page exists in the free physical page list then an attempt to map it is surely a mistake
    assert(! physical_page_available(physical_page));

    page_entry_flags_t x86_flags = page_entry_flag_none;
    if (hal_flags & hal::page_flag_present) x86_flags |= page_entry_flag_present;
    if (hal_flags & hal::page_flag_rw) x86_flags |= page_entry_flag_rw;
    if (hal_flags & hal::page_flag_user) x86_flags |= page_entry_flag_user;
    if (hal_flags & hal::page_flag_allocated) x86_flags |= page_entry_flag_allocated;

    auto directory_index = directory_entry_index(virtual_page);
    auto table_index = table_entry_index(virtual_page);

    // Paging and interrupts must be disabled while the page directory / page tables are being
    // accessed because it involves using the contents of unmapped physical pages.
    libk::DisableInteruptsPaging temp_disabler;

    if (! (directory[directory_index] & page_entry_flag_present)) {
        create_page_directory_table(directory, virtual_page);
    }

    auto table = reinterpret_cast<uint32_t *>(directory[directory_index] & page_frame_mask);

    assert(! (table[table_index] & page_entry_flag_inuse));

    // Like the assignment to the page directory double check assumptions about
    // alignment and note the assumption related to the page offset.
    assert((reinterpret_cast<uint32_t>(physical_page) & page_offset_mask) == 0);
    table[table_index] = reinterpret_cast<uint32_t>(physical_page) | x86_flags | page_entry_flag_inuse;

    flush_virtual_page(virtual_page);
}

bool unmap_virtual_page(page_directory_t directory, const void* virtual_page) noexcept
{
    auto directory_index = directory_entry_index(virtual_page);
    auto table_index = table_entry_index(virtual_page);

    // Paging and interrupts must be disabled while the page directory / page tables are being
    // accessed because it involves using the contents of unmapped physical pages.
    libk::DisableInteruptsPaging temp_disabler;

    // Can't unmap a virtual address if there is no page directory entry for it
    if (! (directory[directory_index] & page_entry_flag_present)) return false;

    auto table = reinterpret_cast<uint32_t *>(directory[directory_index] & page_frame_mask);

    // Can't unmap a virtual address if the virtual address isn't mapped
    if (! (table[table_index] & page_entry_flag_inuse)) return false;

    if (table[table_index] & page_entry_flag_allocated) {
        auto physical_page = reinterpret_cast<void *>(table[table_index] & page_frame_mask);

        hal::free_physical(physical_page);
    }

    table[table_index] = 0;
    flush_virtual_page(virtual_page);

    return true;
}

void flush_virtual_page(const void *virtual_page) noexcept {
    assert(libk::is_page_aligned(virtual_page));

    __asm__ volatile("invlpg (%0)" ::"r" (virtual_page) : "memory");
}

} // namespace hal
