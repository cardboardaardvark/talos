add_library(
    talos-driver-acpi
    EXCLUDE_FROM_ALL

    acpi.cpp
    acpica.cpp
)

target_link_libraries(talos-driver-acpi contrib-acpica)
