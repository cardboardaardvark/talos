extern "C"
{

#include <acpi.h>

} // extern "C"

#include <hal/interrupt.hpp>
#include <hal/terminate.hpp>
#include <libk/error.hpp>

namespace hal
{

[[noreturn]] void poweroff()
{
    AcpiEnterSleepStatePrep(5);
    hal::enable_interrupts(false);
    AcpiEnterSleepState(5);

    libk::panic("Poweroff request failed\n");
}

} // namespace hal
