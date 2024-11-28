#include <hal/interrupt.hpp>

#include "util.hpp"

namespace libk
{

Finally::Finally(handler_t handler) noexcept
: m_armed(true), m_handler(handler)
{ }

Finally::~Finally() noexcept
{
    if (m_armed) fire();
}

bool Finally::armed() const noexcept
{
    return m_armed;
}

void Finally::disarm() noexcept
{
    m_armed = false;
}

void Finally::fire() noexcept
{
    if (m_armed) m_handler();
    m_armed = false;
}

DisableInterrupts::DisableInterrupts() noexcept
: m_need_enable(hal::interrupts_enabled())
{
    if (m_need_enable) hal::enable_interrupts(false);
}

DisableInterrupts::~DisableInterrupts() noexcept
{
    if (m_need_enable) hal::enable_interrupts(true);
}

} // namespace libk
