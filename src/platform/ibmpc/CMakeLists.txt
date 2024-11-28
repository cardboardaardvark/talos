if (NOT DEFINED QEMU_KVM)
    set(QEMU_KVM on)
endif()

if (NOT DEFINED QEMU_VGA)
    set(QEMU_VGA on)
endif()

if (NOT DEFINED QEMU_VGA_BACKEND)
    set(QEMU_VGA_BACKEND std)
endif()

if (NOT DEFINED QEMU_SERIAL)
    set(QEMU_SERIAL on)
endif()

if (QEMU_KVM)
    set(QEMU_KVM_ARG --enable-kvm)
endif()

if (QEMU_VGA)
    list(APPEND QEMU_OUTPUT_ARGS --vga)
    list(APPEND QEMU_OUTPUT_ARGS ${QEMU_VGA_BACKEND})
else()
    list(APPEND QEMU_OUTPUT_ARGS --display)
    list(APPEND QEMU_OUTPUT_ARGS none)
endif()

if (QEMU_SERIAL)
    list(APPEND QEMU_OUTPUT_ARGS --serial)
    list(APPEND QEMU_OUTPUT_ARGS stdio)
endif()

add_library(
    talos-crt OBJECT
    EXCLUDE_FROM_ALL

    crt0.asm
)

target_compile_definitions(talos-crt PRIVATE PAGE_SIZE=${TALOS_PAGE_SIZE})

set(CRT0_O ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/talos-crt.dir/crt0.asm.o)

set(
    CMAKE_CXX_LINK_EXECUTABLE
    # The CRT objects have to be linked in the specifc order specified here
    "${CMAKE_CXX_COMPILER} <CMAKE_CXX_LINK_FLAGS> <FLAGS> <LINK_FLAGS> -o <TARGET> -T ${CMAKE_CURRENT_SOURCE_DIR}/link.ld ${CRT0_O} ${TALOS_CRTI} ${TALOS_CRTBEGIN} -Wl,--start-group <OBJECTS> <LINK_LIBRARIES> -Wl,--end-group ${TALOS_CRTEND} ${TALOS_CRTN}"
)

add_executable(
    talos-bin

    console.cpp
    init.cpp
    logging.cpp
    serial.cpp
)

add_dependencies(talos-bin talos-crt)
target_link_libraries(talos-bin talos-cpu talos-driver talos-kernel talos-libk stdc++ c m gcc)

set_target_properties(talos-bin PROPERTIES OUTPUT_NAME "talos")
set_target_properties(talos-bin PROPERTIES SUFFIX ".bin")
set_target_properties(talos-bin PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_custom_target(
    grub-iso

    DEPENDS talos-bin
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/make-grub-iso ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}
    BYPRODUCTS ${CMAKE_BINARY_DIR}/grub-iso ${CMAKE_BINARY_DIR}/grub.iso
)

add_custom_target(
    run-qemu

    # qemu isn't supplying the optional ELF symbol info when using --kernel so boot the GRUB iso instead
    COMMAND echo "Executing qemu" && qemu-system-i386 ${QEMU_KVM_ARG} -s ${QEMU_OUTPUT_ARGS} --kernel ${CMAKE_BINARY_DIR}/talos.bin
)

add_dependencies(run-qemu talos-bin)

# Have qemu wait for GDB to connect before starting the kernel
add_custom_target(
    run-qemu-debug

    COMMAND echo "Executing qemu - connect with debugger" && qemu-system-i386 ${QEMU_KVM_ARG} -s -S ${QEMU_OUTPUT_ARGS} --kernel ${CMAKE_BINARY_DIR}/talos.bin
)

add_dependencies(run-qemu-debug talos-bin)

add_custom_target(
    run-gdb

    COMMAND gdb -s ${CMAKE_BINARY_DIR}/talos.bin -d ${CMAKE_SOURCE_DIR}/src -ex "target remote localhost:1234"
)
