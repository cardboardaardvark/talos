#include <initializer_list>
#include <string.h>

#include <abi/heap.hpp>
#include <config-memory.hpp>
#include <cpu/x86/gdt.hpp>
#include <cpu/x86/idt.hpp>
#include <cpu/x86/memory.hpp>
#include <cpu/x86/paging.hpp>
#include <cpu/x86/registers.hpp>
#include <driver/acpi/acpi.hpp>
#include <driver/boot/multiboot/multiboot1.hpp>
#include <hal/interrupt.hpp>
#include <hal/memory.hpp>
#include <libk/assert.hpp>
#include <libk/error.hpp>
#include <libk/logging.hpp>
#include <libk/memory.hpp>
#include <libk/terminate.hpp>
#include <kernel/timer.hpp>

#include "console.hpp"
#include "crt0.hpp"
#include "elf.hpp"
#include "link.hpp"
#include "serial.hpp"

namespace platform
{

namespace ibmpc
{

namespace acpi = driver::acpi;
namespace multiboot = driver::boot::multiboot;
namespace x86 = cpu::x86;

typedef struct {
    const void *kernel_end = nullptr;
    std::size_t num_pages = 0;

} physical_page_visitor_args_t;

static size_t total_physical_pages = 0;

static void count_physical_pages(const multiboot::v1_memory_map_t *map, void *arg)
{
    assert(arg != nullptr);

    auto info = reinterpret_cast<physical_page_visitor_args_t *>(arg);

    if (map->type != multiboot::v1_mmap_type_available) return;

    // The address field from multiboot is 64 bits but this is a 32 bit
    // machine. Make sure the address will actually fit into a pointer.
    assert(map->address == (map->address & 0xFFFFFFFF));
    assert(map->address % hal::page_size == 0);

    std::uintptr_t region_start = map->address;
    std::uintptr_t region_end = region_start + map->length;

    libk::printf("\tMemory region: 0x%p - 0x%p\n", region_start, region_end);

    for(size_t offset = 0; region_start + offset < region_end; offset += hal::page_size) {
        std::uintptr_t address = region_start + offset;

        if (address > reinterpret_cast<uintptr_t>(info->kernel_end)) {
            info->num_pages++;
        }
    }
}

// TODO Sigh, the code duplication is quite bad. This needs to also be made into
// a visitor.
static void load_physical_pages(const multiboot::v1_memory_map_t *map, void *arg)
{
    assert(arg != nullptr);

    auto info = reinterpret_cast<physical_page_visitor_args_t *>(arg);

    if (map->type != multiboot::v1_mmap_type_available) return;

    // The address field from multiboot is 64 bits but this is a 32 bit
    // machine. Make sure the address will actually fit into a pointer.
    assert(map->address == (map->address & 0xFFFFFFFF));
    assert(map->address % hal::page_size == 0);

    std::uintptr_t region_start = map->address;
    std::uintptr_t region_end = region_start + map->length;

    for(size_t offset = 0; region_start + offset < region_end; offset += hal::page_size) {
        std::uintptr_t address = region_start + offset;

        if (address > reinterpret_cast<uintptr_t>(info->kernel_end)) {
            libk::free_physical(reinterpret_cast<void *>(address));
        }
    }
}

// Returns the new address of the end of the kernel memory
static void * init_physical_pages(uint64_t length, uint64_t map_address)
{
    assert(libk::is_page_aligned(&_link_end));

    auto kernel_end = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(&_link_end) + hal::page_size);

    libk::printf("kernel_end=0x%p map_address=0x%p length=0x%p\n", kernel_end, map_address, length);
    x86::set_physical_page_list_address(kernel_end);

    physical_page_visitor_args_t visitor_args = {
        .kernel_end = kernel_end,
    };

    // The first count slightly overcounts the number of pages that will be available
    multiboot::visit_v1_memory_map(map_address, length, count_physical_pages, &visitor_args);

    // The second count gets slightly less available pages because it now accounts for the physical pages
    // being used by the available physical page list
    kernel_end = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(kernel_end) + visitor_args.num_pages * sizeof(x86::physical_page_entry_t));
    visitor_args.kernel_end = kernel_end;
    multiboot::visit_v1_memory_map(map_address, length, load_physical_pages, &visitor_args);

    total_physical_pages = hal::available_physical_pages();

    libk::printf("Found %u physical pages; new kernel end: 0x%p\n", visitor_args.num_pages, kernel_end);
    libk::printf("Available physical memory: %u KiB\n", hal::available_physical_pages() * hal::page_size / 1024);

    return kernel_end;
}

// Returns the address of the physical page holding the page directory.
static hal::page_directory_t identity_map_kernel_memory(hal::page_directory_t page_directory, void *kernel_end)
{
    // Protected mode has to be on to use paging and the boot loader
    // should have already done this.
    assert(x86::read_cr0() & x86::enable_protected_flag);
    assert(kernel_end != nullptr);
    assert(libk::is_page_aligned(kernel_end));

    const auto map_start = 0;

    libk::printf("Identity mapping from 0x%p to 0x%p\n", map_start, kernel_end);

    // Identity map everything from the start of memory to the end of the kernel in memory.
    for (uintptr_t address = map_start; address <= reinterpret_cast<uintptr_t>(kernel_end); address += hal::page_size) {
        libk::map_identity_page(page_directory, address, hal::page_flag_present | hal::page_flag_rw);
    }

    return page_directory;
}

static void map_shared_virtual(hal::page_directory_t page_directory)
{
    assert(page_directory != nullptr);
    assert(libk::is_page_aligned(&_shared_start_physical));
    assert(libk::is_page_aligned(&_shared_end_physical));
    assert(libk::is_page_aligned(&_shared_start_virtual));
    assert(libk::is_page_aligned(&_shared_end_virtual));

    auto shared_start_physical = reinterpret_cast<uintptr_t>(&_shared_start_physical);
    auto shared_end_physical = reinterpret_cast<uintptr_t>(&_shared_end_physical);
    auto shared_start_virtual = reinterpret_cast<uintptr_t>(&_shared_start_virtual);
    auto shared_end_virtual = reinterpret_cast<uintptr_t>(&_shared_end_virtual);
    auto shared_offset = shared_start_virtual - shared_start_physical;

    assert((shared_end_physical + shared_offset) == shared_end_virtual);

    libk::printf("Mapping shared area 0x%p:0x%p -> 0x%p:0x%p\n", shared_start_physical, shared_end_physical - 1, shared_start_virtual, shared_end_virtual - 1);

    for (auto physical_page = shared_start_physical; physical_page < shared_end_physical; physical_page += hal::page_size) {
        auto virtual_page = physical_page + shared_offset;
        libk::map_virtual_page(page_directory, virtual_page, physical_page, hal::page_flag_present | hal::page_flag_rw);
    }
}

static void init_guard_pages(hal::page_directory_t page_directory, void *kernel_guard_page)
{
    assert(page_directory != nullptr);
    assert(reinterpret_cast<uintptr_t>(nullptr) == 0);
    assert(libk::is_page_aligned(&_stack_bottom_guard));
    assert(libk::is_page_aligned(&_stack_top_guard));

    // Set 0x0 as a guard page so attempting to dereference a null pointer causes a GPF.
    libk::map_guard_page(page_directory, &_stack_bottom_guard);
    libk::map_guard_page(page_directory, &_stack_top_guard);
    libk::map_guard_page(page_directory, kernel_guard_page);
}

static void * init_paging(void *kernel_end)
{
    auto kernel_guard_page = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(kernel_end) + hal::page_size);
    auto kernel_heap = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(kernel_guard_page) + hal::page_size);
    auto page_directory = reinterpret_cast<hal::page_directory_t>(hal::alloc_physical());

    if (page_directory == nullptr) libk::panic("Could not allocate a page directory table\n");

    identity_map_kernel_memory(page_directory, kernel_end);
    init_guard_pages(page_directory, kernel_guard_page);
    map_shared_virtual(page_directory);

    hal::set_kernel_page_directory(page_directory);

    return kernel_heap;
}

static void init_status()
{
    update_status_memory();
    kernel::start_timer({ .repeat = 100, }, update_status_memory);

    update_status_clock();
    kernel::start_timer({ .repeat = 1000, }, update_status_clock);
}

// Invoked by crt0.asm
extern "C" void init(multiboot::v1_multiboot_magic_t multiboot_magic, const multiboot::v1_info_t *multiboot_info)
{
    x86::init_gdt();
    init_console();

    libk::print("IBM PC platform initializing\n");

    if (multiboot_magic != multiboot::v1_magic) libk::panic("Invalid multiboot 1 magic number: 0x%x\n", multiboot_magic);

    multiboot::dump_v1_info(multiboot_info);

    auto kernel_end = libk::align_page(init_physical_pages(multiboot_info->mmap_length, multiboot_info->mmap_address));
    auto kernel_heap = init_paging(kernel_end);

    hal::enable_paging(true);
    x86::init_idt();
    abi::init_heap(kernel_heap);
    init_status();

    // acpi::initialize_subsystem();
    // acpi::initialize_tables();

    libk::print("Platform initialization complete\n");
}

} // ibmpc

} // platform

namespace hal
{

size_t total_physical_pages() noexcept
{
    return platform::ibmpc::total_physical_pages;
}

} // namespace hal
