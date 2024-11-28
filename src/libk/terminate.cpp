#include <hal/interrupt.hpp>

#include "logging.hpp"
#include "terminate.hpp"

namespace libk
{

[[noreturn]] void halt() noexcept
{
    hal::enable_interrupts(false);

    libk::print("Halting: ");

    while (true)
    {
        libk::print(".");
        hal::wait();
    }
}

} // namespace libk
