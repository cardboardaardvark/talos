#include <cstdlib>
#include <cstdarg>

#include <cpu/x86/io.hpp>
#include <libk/error.hpp>
#include <libk/logging.hpp>

[[noreturn]] static void not_implemented(const char *function_name)
{
    libk::panic("%s() is not yet implemented\n", function_name);
}

extern "C"
{

#include <acpi.h>

ACPI_STATUS AcpiOsInitialize()
{
    return AE_OK;
}

ACPI_STATUS AcpiOsTerminate()
{
    return AE_OK;
}

ACPI_PHYSICAL_ADDRESS AcpiOsGetRootPointer()
{
    ACPI_PHYSICAL_ADDRESS addr = 0;

    AcpiFindRootPointer(&addr);

    return addr;
}

ACPI_STATUS AcpiOsPredefinedOverride(const ACPI_PREDEFINED_NAMES *PredefinedObject, ACPI_STRING *NewValue)
{
    *NewValue = NULL;

    return AE_OK;
}

ACPI_STATUS AcpiOsTableOverride(ACPI_TABLE_HEADER *ExistingTable, ACPI_TABLE_HEADER **NewTable)
{
    *NewTable = NULL;

    return AE_OK;
}

void * AcpiOsMapMemory(ACPI_PHYSICAL_ADDRESS PhysicalAddress, ACPI_SIZE Length)
{
    not_implemented("AcpiOsMapMemory");
}

void AcpiOsUnmapMemory(void *where, ACPI_SIZE length)
{
    not_implemented("AcpiOsUnmapMemory");
}

ACPI_STATUS AcpiOsGetPhysicalAddress(void *LogicalAddress, ACPI_PHYSICAL_ADDRESS *PhysicalAddress)
{
    not_implemented("AcpiOsGetPhysicalAddress");
}

void * AcpiOsAllocate(ACPI_SIZE size)
{
    return malloc(size);
}

void AcpiOsFree(void *memory)
{
    free(memory);
}

BOOLEAN AcpiOsReadable(void *Memory, ACPI_SIZE Length)
{
    not_implemented("AcpiOsReadable");
}

BOOLEAN AcpiOsWritable(void *Memory, ACPI_SIZE Length)
{
    not_implemented("AcpiOsWritable");
}

ACPI_THREAD_ID AcpiOsGetThreadId()
{
    not_implemented("AcpiOsGetThreadId");
}

ACPI_STATUS AcpiOsExecute(ACPI_EXECUTE_TYPE Type, ACPI_OSD_EXEC_CALLBACK Function, void *Context)
{
    not_implemented("AcpiOsExecute");
}

void AcpiOsSleep(UINT64 Milliseconds)
{
    not_implemented("AcpiOsSleep");
}

void AcpiOsStall(UINT32 Microseconds)
{
    not_implemented("AcpiOsStall");
}

ACPI_STATUS AcpiOsCreateMutex (ACPI_MUTEX *OutHandle)
{
    not_implemented("AcpiOsCreateMutex");
}

void AcpiOsDeleteMutex(ACPI_MUTEX Handle)
{
    not_implemented("AcpiOsDeleteMutex");
}

ACPI_STATUS AcpiOsAcquireMutex(ACPI_MUTEX Handle, UINT16 Timeout)
{
    not_implemented("AcpiOsAcquireMutex");
}

void AcpiOsReleaseMutex(ACPI_MUTEX Handle)
{
    not_implemented("AcpiOsReleaseMutex");
}

ACPI_STATUS AcpiOsCreateSemaphore(UINT32 MaxUnits, UINT32 InitialUnits, ACPI_SEMAPHORE *OutHandle)
{
    not_implemented("AcpiOsCreateSemaphore");
}

ACPI_STATUS AcpiOsDeleteSemaphore(ACPI_SEMAPHORE Handle)
{
    not_implemented("AcpiOsDeleteSemaphore");
}

ACPI_STATUS AcpiOsWaitSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units, UINT16 Timeout)
{
    not_implemented("AcpiOsWaitSemaphore");
}

ACPI_STATUS AcpiOsSignalSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units)
{
    not_implemented("AcpiOsSignalSemaphore");
}

ACPI_STATUS AcpiOsCreateLock(ACPI_SPINLOCK *OutHandle)
{
    not_implemented("AcpiOsCreateLock");
}

void AcpiOsDeleteLock(ACPI_HANDLE Handle)
{
    not_implemented("AcpiOsDeleteLock");
}

ACPI_CPU_FLAGS AcpiOsAcquireLock(ACPI_SPINLOCK Handle)
{
    not_implemented("AcpiOsAcquireLock");
}

void AcpiOsReleaseLock(ACPI_SPINLOCK Handle, ACPI_CPU_FLAGS Flags)
{
    not_implemented("AcpiOsReleaseLock");
}

ACPI_STATUS AcpiOsInstallInterruptHandler(UINT32 InterruptLevel, ACPI_OSD_HANDLER Handler, void *Context)
{
    not_implemented("AcpiOsInstallInterruptHandler");
}

ACPI_STATUS AcpiOsRemoveInterruptHandler(UINT32 InterruptNumber, ACPI_OSD_HANDLER Handler)
{
    not_implemented("AcpiOsRemoveInterruptHandler");
}

ACPI_STATUS AcpiOsReadPciConfiguration (ACPI_PCI_ID *PciId, UINT32 Register, UINT64 *Value, UINT32 Width)
{
    not_implemented("AcpiOsReadPciConfiguration");
}

ACPI_STATUS AcpiOsWritePciConfiguration (ACPI_PCI_ID *PciId, UINT32 Register, UINT64 Value, UINT32 Width)
{
    not_implemented("AcpiOsWritePciConfiguration");
}

UINT64 AcpiOsGetTimer (void)
{
    not_implemented("AcpiOsGetTimer");
}

ACPI_STATUS AcpiOsPhysicalTableOverride (ACPI_TABLE_HEADER *ExistingTable, ACPI_PHYSICAL_ADDRESS *NewAddress, UINT32 *NewTableLength)
{
    not_implemented("AcpiOsPhysicalTableOverride");
}

void ACPI_INTERNAL_VAR_XFACE AcpiOsPrintf (const char *Format, ...)
{
    va_list args;

    va_start(args, Format);
    libk::vprintf(Format, args);
    va_end(args);
}

void AcpiOsVprintf (const char *Format, va_list Args)
{
    libk::vprintf(Format, Args);
}

ACPI_STATUS AcpiOsEnterSleep (UINT8 SleepState, UINT32 RegaValue, UINT32 RegbValue)
{
    not_implemented("AcpiOsEnterSleep");
}

ACPI_STATUS AcpiOsReadMemory (ACPI_PHYSICAL_ADDRESS Address, UINT64 *Value, UINT32 Width)
{
    not_implemented("AcpiOsReadMemory");
}

ACPI_STATUS AcpiOsWriteMemory (ACPI_PHYSICAL_ADDRESS Address, UINT64 Value, UINT32 Width)
{
    not_implemented("AcpiOsWriteMemory");
}

ACPI_STATUS AcpiOsReadPort (ACPI_IO_ADDRESS Address, UINT32 *Value, UINT32 Width)
{
    if (Width == 8) {
        *Value = cpu::x86::inb(Address);
    } else {
        libk::panic("AcpiOsReadPort() IO port read size %u is not implemented\n", Width);
    }

    return AE_OK;
}

ACPI_STATUS AcpiOsWritePort (ACPI_IO_ADDRESS Address, UINT32 Value, UINT32 Width)
{
    if (Width == 8) {
        cpu::x86::outb(Address, Value & 0xFF);
    } else {
        libk::panic("AcpiOsWritePort() IO port write size %u is not implemented", Width);
    }

    return AE_OK;
}

void AcpiOsWaitEventsComplete (void)
{
    not_implemented("AcpiOsWaitEventsComplete");
}

ACPI_STATUS AcpiOsSignal (UINT32 Function, void *Info)
{
    not_implemented("AcpiOsSignal");
}



} // extern "C"
