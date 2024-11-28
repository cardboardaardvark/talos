#pragma once

#include <cstdint>

namespace cpu
{

namespace x86
{

const unsigned int enable_interrupts_flag = 1 << 9;
const unsigned int enable_paging_flag = 1 << 31;
const unsigned int enable_protected_flag = 1 << 0;

extern "C"
{

uint32_t read_cr0() noexcept;
void write_cr0(std::uint32_t value) noexcept;
void * read_cr2() noexcept;
uint32_t read_cr3() noexcept;
void write_cr3(std::uint32_t value) noexcept;
uint32_t read_flags() noexcept;

} // extern "C"

} // namespace x86

} // namespace cpu
