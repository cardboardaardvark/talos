#include <stddef.h>
#include <stdint.h>

#include <libk/assert.hpp>
#include <libk/logging.hpp>
#include <libk/memory.hpp>

#include "gdt.hpp"

#define GDT_ENTRY_NULL 0
#define GDT_ENTRY_KCODE 1
#define GDT_ENTRY_KDATA 2
#define GDT_SIZE 3

namespace cpu
{

namespace x86
{

gdt_table_t gdt_table;
static gdt_entry_t gdt_entries[GDT_SIZE];

static void set_gdt_entry(size_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) noexcept
{
    assert(num < GDT_SIZE);

    gdt_entry_t *entry = &gdt_entries[num];

    entry->base_low = (base & 0xFFFF);
    entry->base_middle = (base >> 16) & 0xFF;
    entry->base_high = (base >> 24) & 0xFF;

    entry->limit_low = (limit & 0xFFFF);
    entry->flags = ((limit >> 16) & 0x0F);

    entry->flags |= (flags & 0xF0);
    entry->access = access;
}

void init_gdt() noexcept
{
    libk::clear_memory(&gdt_entries, sizeof(gdt_entries));

    // memory.asm assumes the layout of gdt_entries - if changes happen
    // here that file may need to be updated
    set_gdt_entry(GDT_ENTRY_NULL, 0, 0, 0, 0);
    set_gdt_entry(GDT_ENTRY_KCODE, 0, 0xFFFFF, 0x9A, 0xCF);
    set_gdt_entry(GDT_ENTRY_KDATA, 0, 0xFFFFF, 0x92, 0xCF);

    gdt_table.limit = sizeof(gdt_entry_t) * GDT_SIZE - 1;
    gdt_table.base = gdt_entries;

    install_gdt();
}

} // namespace x86

} // namespace cpu
