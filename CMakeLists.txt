cmake_minimum_required(VERSION 3.25.1)

include(ExternalProject)

if(NOT DEFINED TALOS_TARGET)
    set(TALOS_TARGET i686-talos)
endif()

if(NOT DEFINED TALOS_PLATFORM)
    set(TALOS_PLATFORM ibmpc)
endif()

if(NOT DEFINED CMAKE_VERBOSE_MAKEFILE)
    set(CMAKE_VERBOSE_MAKEFILE yes)
endif()

if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the build type")
endif()

if(DEFINED TALOS_TARGET)
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/target-${TALOS_TARGET}.cmake)
endif()

set(TALOS_WARNING_FLAGS "-Werror -Wfatal-errors -Wall -Wextra -Wpedantic -Wshadow -Wformat=2")
set(TALOS_DEBUG_FLAGS "-g -Og -ggdb")

project(
    talos

    VERSION 0.0.0
    LANGUAGES ASM_NASM C CXX
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_ASM_NASM_FLAGS_DEBUG "${CMAKE_ASM_NASM_FLAGS} -g")
endif()

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED YES)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TALOS_WARNING_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "${TALOS_DEBUG_FLAGS}")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${TALOS_DEBUG_FLAGS}")

set(CMAKE_LINKER ${CMAKE_CXX_COMPILER})

if(NOT DEFINED TALOS_PAGE_SIZE)
    set(TALOS_PAGE_SIZE 4096)
endif()

if(NOT BUILD_TOOLCHAIN)
    add_subdirectory(src/)
elseif(NOT DEFINED TALOS_TARGET)
    message(FATAL_ERROR "BUILD_TOOLCHAIN requires TALOS_TARGET")
else()
    add_subdirectory(toolchain/)
endif()

