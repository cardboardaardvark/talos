#include <hal/interrupt.hpp>

#include "logging.hpp"
#include "terminate.hpp"

namespace libk
{

[[noreturn]] void halt() noexcept
{
    hal::enable_interrupts(false);

    libk::write_log("Halting: ");

    while (true)
    {
        libk::write_log(".");
        hal::wait();
    }
}

} // namespace libk
