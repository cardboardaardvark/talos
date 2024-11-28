if(DEFINED TALOS_C_COMPILER)
    set(CMAKE_C_COMPILER ${TALOS_C_COMPILER})
endif()

if(DEFINED TALOS_CXX_COMPILER)
    set(CMAKE_CXX_COMPILER ${TALOS_C_COMPILER})
endif()

set(TALOS_EXTRA_FLAGS "-ffile-prefix-map=${CMAKE_SOURCE_DIR}/src/=")

if(TALOS_FREESTANDING)
    set(TALOS_FREESTANDING_C_FLAGS "-nostdlib")
    set(TALOS_FREESTANDING_CXX_FLAGS "${TALOS_FREESTANDING_C_FLAGS}")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TALOS_FREESTANDING_C_FLAGS} ${TALOS_EXTRA_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TALOS_FREESTANDING_CXX_FLAGS} ${TALOS_EXTRA_FLAGS}")

add_compile_definitions(TALOS_VERSION="${CMAKE_PROJECT_VERSION}")

include_directories(. config/i686-ibmpc)

add_subdirectory(abi)
add_subdirectory(contrib)
add_subdirectory(driver)
add_subdirectory(kernel)
add_subdirectory(libk)

add_subdirectory(cpu/x86)
add_subdirectory(platform/ibmpc)
