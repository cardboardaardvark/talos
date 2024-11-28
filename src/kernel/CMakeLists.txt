add_library(
    talos-kernel STATIC
    EXCLUDE_FROM_ALL

    job.cpp
    main.cpp
    timer.cpp
)

target_link_libraries(talos-kernel talos-libk)
