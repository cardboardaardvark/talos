/* FIXME When changes are made to this file dependent things are not relinked */

/* Kernel entry point */
ENTRY(boot)

MEMORY
{
    /* Place the kernel starting at 1 MiB. This will not work with UEFI. */
    KERNEL : ORIGIN = 1M, LENGTH = 4096M - 2M
    SHARED : ORIGIN = 4096M - 1M, LENGTH = 1M
}

SECTIONS
{
    /* Get the multiboot header as early as possible */
    .multiboot : ALIGN(4K)
    {
        *(.multiboot)

        _shared_start_physical = ALIGN(4K);
    } >KERNEL

    .shared_text : ALIGN(4K)
    {
        _shared_start_virtual = ALIGN(4K);

        ../../cpu/x86/libtalos-cpu-x86-trampoline.a(.text)

        _shared_end_virtual = ALIGN(4K);
    } >SHARED AT>KERNEL

    .text : ALIGN(4K)
    {
        _shared_end_physical = ALIGN(4K);

        *(.text)
    } >KERNEL

    .rodata : ALIGN(4K)
    {
        *(.rodata)
    } >KERNEL

    .data : ALIGN(4K)
    {
        *(.data)
    } >KERNEL

    .bss : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    } >KERNEL

    _link_end = ALIGN(4K);
}
