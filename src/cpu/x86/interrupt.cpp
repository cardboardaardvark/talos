#include <hal/interrupt.hpp>

#include "registers.hpp"

#define UNWAIT_INTERRUPT 50

namespace hal
{

using namespace cpu::x86;

void wait() noexcept
{
    __asm__ volatile ("hlt");
}

void unwait() noexcept
{
    if (hal::inside_isr()) return;

    __asm__ volatile
    (
        "int %[intnum]"
        :
        : [intnum] "i" (UNWAIT_INTERRUPT)
    );
}

void enable_interrupts(bool enabled) noexcept
{
    if (enabled) {
        __asm__ volatile ("sti");
    } else {
        __asm__ volatile ("cli");
    }
}

bool interrupts_enabled() noexcept
{
    return read_flags() & enable_interrupts_flag;
}

} // namespace hal
