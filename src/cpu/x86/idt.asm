%define idt_size 256

extern handle_interrupt
extern kernel_page_directory

; Add 1 to eax for interrupt numbers that correspond to
; fault and abort. Assumes ecx has the interrupt number
; stored in it.
maybe_adjust_return:
    cmp ecx, 0
    je .adjust

    cmp ecx, 4
    jle .done

    cmp ecx, 20
    jge .done

    .adjust:
    add eax, 1

    .done:
    ret

idt_stub_common:
    ; get a backup of the processor state before calling into the kernel
    pusha ; push general purpose registers
    mov eax, cr3
    push eax ; store the page directory in use when the interrupt happened
    push ds
    push es
    push fs
    push gs

    ; make sure the kernel page directory will be used when executing the ISRs
    mov eax, [kernel_page_directory]
    mov ecx, cr3
    cmp eax, ecx
    je .skip_set_page_directory

    mov cr3, eax

    .skip_set_page_directory:

    ; Load the Kernel Data Segment descriptor
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; get the return address into a register
    mov eax, [esp + 15 * 4]
    ; get the interrupt number into a register
    mov ecx, [esp + 13 * 4]

    ; Advance the address of the return location for interrupt numbers that
    ; continue at the place the fault happened.
    call maybe_adjust_return

    ; put together a new frame pointer for GDB
    push eax
    push ebp
    mov ebp, esp

    ; make the address for the interrupt info struct point to the
    ; right place in the stack after the stack entries for the
    ; call frame
    lea eax, [esp + 8]
    push eax

    call handle_interrupt

    ; remove the call frame info and argument to handle_interrupt
    add esp, 12

    pop gs
    pop fs
    pop es
    pop ds

    ; use the original page directory if it is different from the kernel page directory
    pop eax
    mov ecx, [kernel_page_directory]
    cmp eax, ecx
    je .continue_return

    mov cr3, eax

    .continue_return:
    popa ; restore general purpose registers
    add esp, 8     ; Cleans up the pushed error code and ISR number
    iret           ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP!

%macro idt_err_stub 1
idt_stub_%+%1:
    push long %1
    jmp idt_stub_common
%endmacro

%macro idt_no_err_stub 1
idt_stub_%+%1:
    push long 0
    push long %1
    jmp idt_stub_common
%endmacro

idt_no_err_stub 0
idt_no_err_stub 1
idt_no_err_stub 2
idt_no_err_stub 3
idt_no_err_stub 4
idt_no_err_stub 5
idt_no_err_stub 6
idt_no_err_stub 7
idt_err_stub    8
idt_no_err_stub 9
idt_err_stub    10
idt_err_stub    11
idt_err_stub    12
idt_err_stub    13
idt_err_stub    14
idt_no_err_stub 15
idt_no_err_stub 16
idt_err_stub    17
idt_no_err_stub 18
idt_no_err_stub 19
idt_no_err_stub 20
idt_no_err_stub 21
idt_no_err_stub 22
idt_no_err_stub 23
idt_no_err_stub 24
idt_no_err_stub 25
idt_no_err_stub 26
idt_no_err_stub 27
idt_no_err_stub 28
idt_no_err_stub 29
idt_err_stub    30
idt_no_err_stub 31

; all the IDT entries after 31 don't have error codes
%assign stub_num 0
%rep idt_size
%if stub_num >= 32
idt_no_err_stub stub_num
%endif
%assign stub_num stub_num+1
%endrep

global idt_stub_table
idt_stub_table:
%assign i 0
%rep    idt_size
    dd idt_stub_%+i
%assign i i+1
%endrep
