add_library(
    talos-libk STATIC
    EXCLUDE_FROM_ALL

    exceptions.cpp
    error.cpp
    logging.cpp
    memory.cpp
    mutex.cpp
    terminate.cpp
    util.cpp
)

target_link_libraries(talos-libk contrib-printf talos-abi)
