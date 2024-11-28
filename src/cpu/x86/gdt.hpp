#pragma once

#include <cstdint>

namespace cpu
{

namespace x86
{

typedef struct __attribute__((packed))
{
    std::uint16_t limit_low;
    std::uint16_t base_low;
    std::uint8_t base_middle;
    std::uint8_t access;
    std::uint8_t flags;
    std::uint8_t base_high;
} gdt_entry_t;

typedef struct __attribute__((packed))
{
    std::uint16_t limit;
    gdt_entry_t* base;
} gdt_table_t;

extern "C"
{

    extern gdt_table_t gdt_table;
    extern void install_gdt() noexcept;
}

void init_gdt() noexcept;

} // namespace x86

} // namespace cpu
