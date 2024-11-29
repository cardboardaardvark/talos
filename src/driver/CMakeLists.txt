add_subdirectory(acpi)

add_library(
    talos-driver STATIC
    EXCLUDE_FROM_ALL

    boot/multiboot/multiboot1.cpp

    serial/uart.cpp

    video/vga/vga.cpp
)

target_link_libraries(talos-driver contrib-acpica talos-driver-acpi)
