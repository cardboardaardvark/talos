# ISRs go into a seperate part of the addres space
# from the rest of the kernel. Put them into their
# own archive so the linker can place their addresses
# in that space.
add_library(
    talos-cpu-x86-trampoline

    idt.asm
)

add_library(
    talos-cpu STATIC
    EXCLUDE_FROM_ALL

    gdt.cpp gdt.asm
    idt.cpp
    interrupt.cpp
    io.cpp
    memory.cpp
    paging.cpp
    registers.asm
)

target_link_libraries(talos-cpu talos-cpu-x86-trampoline)

# talos-cpu has HAL implementations libk uses
target_link_libraries(talos-libk talos-cpu)
