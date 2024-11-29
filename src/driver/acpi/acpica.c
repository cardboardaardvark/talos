#include <platform/actalos.h>
#include <actypes.h>
#include <aclocal.h>
#include <acpixf.h>

void poweroff()
{
    AcpiEnterSleepStatePrep(5);
    // hal::enable_interrupts(false);
    AcpiEnterSleepState(5);
}
