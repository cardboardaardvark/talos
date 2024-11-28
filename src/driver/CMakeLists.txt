add_library(
    talos-driver STATIC
    EXCLUDE_FROM_ALL

    boot/multiboot/multiboot1.cpp

    serial/uart.cpp

    video/vga/vga.cpp
)
