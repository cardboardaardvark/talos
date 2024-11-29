extern "C"
{

#include <acpi.h>

} // extern "C"

#include <hal/interrupt.hpp>
#include <hal/terminate.hpp>
#include <libk/exceptions.hpp>
#include <libk/error.hpp>

namespace driver
{

namespace acpi
{

void initialize_subsystem()
{
    auto error = AcpiInitializeSubsystem();

    switch (error) {
        case AE_OK: return;
        case AE_ERROR: throw libk::RuntimeError("System is not capable of supporting ACPI mode");
        case AE_NO_MEMORY: throw libk::RuntimeError("ACPICA could not allocate memory");
    }
}

void initialize_tables()
{
    auto error = AcpiInitializeTables (NULL, 16, FALSE);

    switch (error) {
        case AE_OK: return;
        case AE_NOT_FOUND: throw libk::RuntimeError("A valid RSDP could not be located");
        case AE_NO_MEMORY: throw libk::RuntimeError("ACPICA could not allocate memory");
    }
}

} // namespace acpi

} // namespace driver

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
