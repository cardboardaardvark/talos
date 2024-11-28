; FIXME When changes are made to this file dependent things are not relinked

; The stack size must be an integer multiple of the page size
%define STACK_SIZE 16384 ; 16 KiB

; Declare constants for the multiboot header.
MBALIGN  equ  1 << 0            ; align loaded modules on page boundaries
MEMINFO  equ  1 << 1            ; provide memory map
MBFLAGS  equ  MBALIGN | MEMINFO ; this is the Multiboot 'flag' field
MAGIC    equ  0x1BADB002        ; 'magic number' lets bootloader find the header
CHECKSUM equ -(MAGIC + MBFLAGS)   ; checksum of above, to prove we are multiboot

; Declare a multiboot header that marks the program as a kernel. These are magic
; values that are documented in the multiboot standard. The bootloader will
; search for this signature in the first 8 KiB of the kernel file, aligned at a
; 32-bit boundary. The signature is in its own section so the header can be
; forced to be within the first 8 KiB of the kernel file.
section .multiboot
    align 4

    dd MAGIC
    dd MBFLAGS
    dd CHECKSUM

section .bss
    ; The multiboot standard does not define the value of the stack pointer register
    ; (esp) and it is up to the kernel to provide a stack. This allocates room for a
    ; stack by carving out a region in bss and putting guard pages around it.
    ; The stack grows downwards on x86.
    ;
    ; TODO: Make the stack in its own section so it can be marked nobits,
    ; which means the kernel file is smaller because it does not contain an
    ; uninitialized stack.

    ; The stack on x86 must be 16-byte aligned according to the
    ; System V ABI standard and de-facto extensions. The compiler will assume the
    ; stack is properly aligned and failure to align the stack will result in
    ; undefined behavior.

    ; make sure the guard is page aligned and an entire page width
    global _stack_bottom_guard
    align PAGE_SIZE
    _stack_bottom_guard:
        resb PAGE_SIZE

    ; The stack needs to be 16 byte aligned which is taken care of
    ; with the guard page alignment. This is actually the end of the
    ; stack because the x86 stack grows down.
    stack_bottom:
        resb STACK_SIZE

    ; The begining of the stack.
    stack_top:

    ; The stack size must be an integer multiple of the page size
    ; so the guard page here is already aligned.
    global _stack_top_guard
    _stack_top_guard:
        resb PAGE_SIZE

; The linker script specifies boot as the entry point to the kernel and the
; bootloader will jump to this position once the kernel has been loaded. It
; doesn't make sense to return from this function as the bootloader is gone.
section .text
    global boot:function (boot.end - boot)
    boot:
        ; The bootloader has loaded us into 32-bit protected mode on a x86
        ; machine. Interrupts are disabled. Paging is disabled. The processor
        ; state is as defined in the multiboot standard.

        ; To set up a stack, we set the esp register to point to the top of our
        ; stack as it grows downwards on x86 systems.
        mov esp, stack_top

        ; Setup the stack for the call to the platform start() function before
        ; calling any other C/C++ functions so the values passed in from multiboot
        ; via registers are preserved.
        sub esp, 8 ; align stack
        push long ebx ; The multiboot information structure
        push long eax ; The multiboot magic value

        ; Call the compiler defined function to initialize globals set by
        ; functions / call global constructors
        extern _init
        call _init

        ; Using the stack setup before the call to _init as args to the platform
        ; init() function.
        extern init
        call init
        add esp, 16 ; move the stack to get rid of the arguments to init().

        ; Call the kernel start() function. The start() function is called from
        ; here instead of the platform init function so as much of the stack
        ; as possible is available.
        extern start
        call start

        ; skip calling _fini because the kernel should never return from start()
        ; extern _fini
        ; call _fini

        ; Best thing to do is shut off interrupts and halt in a loop though
        ; this should only ever be reached if the kernel really messes up.
        cli

    .hang:
        hlt
        jmp .hang

    .end:
