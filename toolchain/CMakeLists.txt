if(NOT DEFINED TOOLCHAIN_PREFIX)
    set(TOOLCHAIN_PREFIX /usr/local)
endif()

if(NOT DEFINED TOOLCHAIN_DOWNLOADS)
    set(TOOLCHAIN_DOWNLOADS ${CMAKE_BINARY_DIR}/downloads)
endif()

ExternalProject_Add(
    toolchain-binutils

    EXCLUDE_FROM_ALL yes
    STEP_TARGETS build configure download install
    DOWNLOAD_DIR ${TOOLCHAIN_DOWNLOADS}
    URL
        http://mirrors.ibiblio.org/gnu/binutils/binutils-2.43.tar.xz
        http://mirror.keystealth.org/gnu/binutils/binutils-2.43.tar.xz
        http://ftp.gnu.org/gnu/binutils/binutils-2.43.tar.xz
    URL_HASH SHA256=b53606f443ac8f01d1d5fc9c39497f2af322d99e14cea5c0b4b124d630379365
    PATCH_COMMAND patch -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/binutils-2.43.patch
    CONFIGURE_COMMAND CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} ../toolchain-binutils/configure --target=${TALOS_TARGET} --prefix=${TOOLCHAIN_PREFIX} --disable-nls
    INSTALL_COMMAND $(MAKE) install
)

ExternalProject_Add(
    toolchain-gcc

    EXCLUDE_FROM_ALL yes
    DEPENDS toolchain-binutils-install
    STEP_TARGETS build configure download install
    DOWNLOAD_DIR ${TOOLCHAIN_DOWNLOADS}
    URL
        http://mirrors.ibiblio.org/gnu/gcc/gcc-14.2.0/gcc-14.2.0.tar.xz
        http://mirror.keystealth.org/gnu/gcc/gcc-14.2.0/gcc-14.2.0.tar.xz
        https://ftp.gnu.org/gnu/gcc/gcc-14.2.0/gcc-14.2.0.tar.xz
    URL_HASH SHA256=a7b39bc69cbf9e25826c5a60ab26477001f7c08d85cec04bc0e29cabed6f3cc9
    PATCH_COMMAND patch -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/gcc-14.2.0.patch
    CONFIGURE_COMMAND CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} ../toolchain-gcc/configure --target=${TALOS_TARGET} --prefix=${TOOLCHAIN_PREFIX} --enable-languages=c,c++ --with-newlib --enable-libstdcxx-static-eh-pool --disable-nls
    BUILD_COMMAND $(MAKE) all-gcc
    INSTALL_COMMAND $(MAKE) install-gcc
)

ExternalProject_Add(
    toolchain-newlib

    EXCLUDE_FROM_ALL yes
    DEPENDS toolchain-gcc-install
    STEP_TARGETS build configure download install
    DOWNLOAD_DIR ${TOOLCHAIN_DOWNLOADS}
    URL
        http://mirrors.kernel.org/sourceware/newlib/newlib-3.0.0.tar.gz
        http://www.mirrorservice.org/sites/sourceware.org/pub/newlib/newlib-3.0.0.tar.gz
        ftp://sourceware.org/pub/newlib/newlib-3.0.0.tar.gz
    URL_HASH SHA256=c8566335ee74e5fcaeb8595b4ebd0400c4b043d6acb3263ecb1314f8f5501332
    PATCH_COMMAND patch -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/newlib-3.0.0.patch
    CONFIGURE_COMMAND ../toolchain-newlib/configure CFLAGS=${TALOS_DEBUG_FLAGS} CXXFLAGS=${TALOS_DEBUG_FLAGS} --target=i686-talos --prefix=${TOOLCHAIN_PREFIX}
    BUILD_COMMAND $(MAKE) all
    INSTALL_COMMAND $(MAKE) install
)

add_custom_target(
    toolchain-gcc-build-libgcc

    DEPENDS toolchain-gcc-build toolchain-newlib-install
    COMMAND cd toolchain-gcc-prefix/src/toolchain-gcc-build && $(MAKE) all-target-libgcc
)

add_custom_target(
    toolchain-gcc-install-libgcc

    DEPENDS toolchain-gcc-build-libgcc
    COMMAND cd toolchain-gcc-prefix/src/toolchain-gcc-build && $(MAKE) install-target-libgcc
)

add_custom_target(
    toolchain-gcc-build-libstdcxx

    DEPENDS toolchain-gcc-build toolchain-newlib-install
    COMMAND cd toolchain-gcc-prefix/src/toolchain-gcc-build && $(MAKE) all-target-libstdc++-v3
)

add_custom_target(
    toolchain-gcc-install-libstdcxx

    DEPENDS toolchain-gcc-build-libstdcxx
    COMMAND cd toolchain-gcc-prefix/src/toolchain-gcc-build && $(MAKE) install-target-libstdc++-v3
)

add_custom_target(download-toolchain DEPENDS toolchain-binutils-download toolchain-gcc-download toolchain-newlib-download)
add_custom_target(install-toolchain DEPENDS download-toolchain toolchain-gcc-install-libstdcxx)
