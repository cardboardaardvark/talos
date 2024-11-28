#include <libk/assert.hpp>
#include <libk/logging.hpp>
#include <libk/error.hpp>

#include "multiboot1.hpp"

namespace driver
{

namespace boot
{

namespace multiboot
{

void dump_v1_info(const v1_info_t *info) noexcept
{
    libk::print("Multiboot 1 info:\n");

    libk::printf("\tflags: 0x%lx\n", info->flags);

    if (info->flags & v1_memory_flag) {
        libk::printf("\tmemory info: low=%u kb high=%u kb\n", info->lower_memory, info->higher_memory);
    }

    if (info->flags & v1_cmdline_flag) {
        libk::printf("\tcommand line: %s\n", info->command_line);
    }

    if (info->flags & v1_bootloader_name_flag) {
        libk::printf("\tBoot loader: %s\n", info->boot_loader_name);
    }

    dump_v1_memory_map(info->mmap_address, info->mmap_length);
}

static void dump_v1_memory_map_visitor(const v1_memory_map_t *map, void *)
{
    if (map->type == 1) {
        // Make sure the 64 bit wide number that specifies the start of the
        // memory span is no more than 32 bits long or it can't be a valid
        // pointer for a 32 bit platformitecture.
        assert(map->address == (map->address & 0xFFFFFFFF));

        void *address = reinterpret_cast<void *>(map->address);
        libk::printf("\tsize=%llu kb address=0x%p\n", map->length / 1024, address);
    }
}

void dump_v1_memory_map(std::uint64_t map_address, std::uint64_t length) noexcept
{
    libk::printf("Multiboot memory map:\n");
    visit_v1_memory_map(map_address, length, dump_v1_memory_map_visitor, nullptr);
}

void visit_v1_memory_map(std::uint64_t map_address, std::uint64_t length, v1_memory_map_visitor_t visitor, void *arg)
{
    auto map = reinterpret_cast<v1_memory_map_t *>(map_address);
    size_t num_entries = length / sizeof(v1_memory_map_t);

    for (size_t i = 0; i < num_entries; i++) {
        visitor(&map[i], arg);
    }
}

} // namespace multiboot

} // namespace boot

} // namespace driver
