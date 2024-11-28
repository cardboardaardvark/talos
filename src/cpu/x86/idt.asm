%define idt_size 256

extern handle_interrupt
extern kernel_page_directory

idt_stub_common:
    ; get a backup of the processor state before calling into the kernel
    pusha ; push general purpose registers
    mov eax, cr3 ; store the page directory in use when the interrupt happened
    push eax
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10   ; Load the Kernel Data Segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp   ; Push us onto the stack - is this needed?
    push eax

    ; make sure the kernel page directory will be used when executing
    ; the ISRs
    mov eax, [kernel_page_directory]
    mov ebx, cr3
    cmp eax, ebx
    je .execute_handler

    mov cr3, eax

    .execute_handler:
    mov eax, handle_interrupt
    call eax       ; A special call, preserves the 'eip' register

    pop eax
    pop gs
    pop fs
    pop es
    pop ds

    ; use the original page directory if it is different from the kernel page directory
    mov eax, [kernel_page_directory]
    pop ebx
    cmp eax, ebx
    je .continue_return

    mov cr3, ebx

    .continue_return:
    popa ; pop general purpose registers
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
