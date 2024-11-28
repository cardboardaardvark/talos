#pragma once

#include <cstdint>

namespace driver
{

namespace boot
{

namespace multiboot
{

using v1_multiboot_magic_t = uint32_t;
const v1_multiboot_magic_t v1_magic = 0x2BADB002;

const unsigned int v1_memory_flag = 1 << 0;
const unsigned int v1_boot_device_flag = 1 << 1;
const unsigned int v1_cmdline_flag = 1 << 2;
const unsigned int v1_modules_flag = 1 << 3;
const unsigned int v1_aout_symbols_flag = 1 << 4;
const unsigned int v1_elf_symbols_flag = 1 << 5;
const unsigned int v1_bootloader_name_flag = 1 << 9;

const unsigned int v1_mmap_type_available = 1;
const unsigned int v1_mmap_type_acpi = 3;
const unsigned int v1_mmap_type_preserved = 4;
const unsigned int v1_mmap_type_unavailable = 5;

typedef struct
{
    std::uint32_t num;
    std::uint32_t size;
    std::uint32_t addr;
    std::uint32_t shndx;
} __attribute__((packed)) v1_aout_symbols_t;

typedef struct
{
    std::uint32_t num;
    std::uint32_t size;
    std::uint32_t addr;
    std::uint32_t shndx;
} __attribute((packed)) v1_elf_symbols_t;

typedef union
{
    v1_aout_symbols_t aout;
    v1_elf_symbols_t elf;
} v1_symbol_info_t;

typedef struct
{
    std::uint32_t size;
    std::uint64_t address;
    std::uint64_t length;
    std::uint32_t type;
} __attribute__((packed)) v1_memory_map_t;

typedef struct
{
    std::uint32_t flags;
    // Lower and upper memory in kilobytes
    std::uint32_t lower_memory;
    std::uint32_t higher_memory;
    std::uint32_t boot_device;
    const char* command_line;
    std::uint32_t modules_count;
    std::uint32_t modules_address;
    v1_symbol_info_t symbols;
    std::uint32_t mmap_length;
    std::uint32_t mmap_address;
    std::uint32_t drives_length;
    std::uint32_t drives_addr;
    std::uint32_t config_address;
    const char* boot_loader_name;
} __attribute__((packed)) v1_info_t;

typedef void (v1_memory_map_visitor_t)(const v1_memory_map_t *map, void *arg);

void dump_v1_info(const v1_info_t *info) noexcept;
void dump_v1_memory_map(std::uint64_t map_address, std::uint64_t length) noexcept;
void visit_v1_memory_map(std::uint64_t map_address, std::uint64_t length, v1_memory_map_visitor_t visitor, void *arg);

} // namespace multiboot

} // namespace boot

} // namespace driver
