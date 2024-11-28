add_library(
    talos-abi STATIC
    EXCLUDE_FROM_ALL

    heap.cpp
    newlib-stubs.cpp
    sysconf.cpp
    terminate.cpp
)

target_link_libraries(talos-abi contrib-malloc talos-libk)
