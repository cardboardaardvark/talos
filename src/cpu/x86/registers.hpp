#pragma once

#include <cstdint>

#define CR0_ENABLE_PROTECTED_FLAG (1 << 0) // Protected mode
#define CR0_ENABLE_PAGING_FLAG (1 << 31)

#define FLAGS_INTERRUPT_ENABLE (1 << 9)

namespace cpu
{

namespace x86
{

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
