add_library(
    talos-driver-acpi
    EXCLUDE_FROM_ALL

    acpica.c
)

target_link_libraries(talos-driver-acpi contrib-acpica)
