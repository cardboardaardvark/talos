#pragma once

#include <cstddef>

namespace cpu
{

namespace x86
{

// Start off with a struct in case more information
// should be stored with a physical page entry later.
typedef struct {
    void *address;
} physical_page_entry_t;

void set_physical_page_list_address(void *address) noexcept;

void * gpf_address() noexcept;

} // namespace x86

} // namespace cpu
