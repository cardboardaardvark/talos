; FIXME When changes are made to this file dependent things are not relinked

; This will set up our new segment registers.

%define CODE_SEGMENT_OFFSET 0x08
%define DATA_SEGMENT_OFFSET 0x10

extern gdt_table

global install_gdt:function
install_gdt:
    lgdt [gdt_table]
    mov ax, DATA_SEGMENT_OFFSET
    mov ds, ax ; data segment
    mov es, ax ;
    mov fs, ax ;
    mov gs, ax ;
    mov ss, ax ; stack segment

    ; We need to do something special in order to set
    ; the code segment (CS): We do what is called a
    ; far jump which is a jump that includes a segment
    ; as well as an offset.
    jmp CODE_SEGMENT_OFFSET:post_gdt_install

post_gdt_install:
    ret
