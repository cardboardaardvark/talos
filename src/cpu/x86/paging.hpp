#pragma once

#include <cstdint>

namespace cpu
{

namespace x86
{

// TODO Make page_directory_t into a structure
// that has a mutex and the page directory entry storage.

const size_t num_page_entries = 1024;
const uint32_t page_flag_mask = 0xfff;
const uint32_t page_frame_mask = 0xfffff000;
const uint32_t page_offset_mask = 0xfff;

using page_entry_flags_t = uint16_t;
const page_entry_flags_t page_entry_flag_none = 0;
const page_entry_flags_t page_entry_flag_present = 1 << 0;
const page_entry_flags_t page_entry_flag_rw = 1 << 1;
const page_entry_flags_t page_entry_flag_user = 1 << 2;
const page_entry_flags_t page_entry_flag_inuse = 1 << 9;
// True if the physical address for the page table entry
// was allocated.
const page_entry_flags_t page_entry_flag_allocated = 1 << 10;

using page_table_entry_t = uint32_t;
using page_table_t = page_table_entry_t *;
const uint32_t page_table_entry_mask = 0x3ff000;
const uint32_t page_table_entry_shift = 12;

using page_directory_entry_t = uint32_t;
using page_directory_t = page_directory_entry_t *;
const uint32_t page_directory_entry_mask = 0xffc00000;
const uint32_t page_directory_entry_shift = 22;

using page_table_visitor_t = bool (*)(const page_directory_entry_t *directory, const page_table_entry_t *table, void *args);

void dump_page_directory(const page_directory_t directory);
void visit_page_tables(const page_directory_t directory, const page_table_visitor_t visitor, void *visitor_args);

} // namespace x86

} // cpu
