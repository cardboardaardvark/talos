#pragma once

#include <cstddef>

namespace hal
{

typedef void (*tick_handler_t)();

void enable_interrupts(bool enabled) noexcept;
bool interrupts_enabled() noexcept;

#ifndef NDEBUG
bool inside_isr() noexcept;
#endif

// Pause until an interrupt happens
void wait() noexcept;
// Continue from wait() if waiting there
void unwait() noexcept;

void set_tick_frequency(size_t hz) noexcept;
void set_tick_handler(tick_handler_t handler) noexcept;

} // namespace hal
