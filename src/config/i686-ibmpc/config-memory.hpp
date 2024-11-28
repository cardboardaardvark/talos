#pragma once

#include <cstdint>

#include <cpu/x86/paging.hpp>

namespace hal
{

using page_directory_t = cpu::x86::page_directory_t;

const size_t page_size = 4096;

} // namespace hal
