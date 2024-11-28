#pragma once

#include <cstddef>

#include <hal/memory.hpp>

namespace abi
{

void init_heap(void *heap_start) noexcept;
const void * start_of_heap() noexcept;
const void * end_of_heap() noexcept;

} // namespace abi
