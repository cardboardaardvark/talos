# TalOS Experimental Operating System

There is not much here yet. One notable thing about the kernel I'm creating is that
it has most of a full libc implementation available as well as a hosted (not standalone)
C++ environment available. The C and C++ compiler are both using the 23 standard. You
can use most things you'd expect from modern C++ like std::vector and std::shared_ptr.
Some stuff may need some runtime features implemented to work and some stuff will probably
never work like std::mutex and std::thread. There are TalOS specific versions of
those C++ features.

## FAQ

* Q: How many bugs are there? A: Yes.

* Q: Why? A: It's fun. Also I want to experiment with creating a new user space API
  that is not Windows and is not Unix.

## Getting Started

Note: this guide assumes you somewhat know your way around a Unix/Linux system and
compiling software.

Development is happening on a Debian Bookworm system. Other systems will likely
work but YMMV. If you have to make a change to get TalOS to compile on your system
please send a patch to be incorporated into the project.

This software must presently be cross compiled and then booted under an emulator,
such as QEMU, or if you are brave, on real hardware. Only the 32 bit Intel/AMD CPU
is supported at the moment. Booting can be done with a multiboot 1 compatible bootloader
but only QEMU direct kernel boot and GRUB have been tested.

You must cross compile using GNU binutils and GCC modified to understand the i686-talos
target. The build system can automatically download and build these tools as well
as the required Newlib libc implementation.

Your system must already have all the requirements met to compile binutils version
2.43 and GCC 14.2.0. Debian Bookworm does not have new enough versions of all
GCC dependencies: you'll have to install at least a newer libgmp. The GCC configure
will fail and tell you what libraries are missing or are not new enough. Otherwise
you can look at https://wiki.osdev.org/GCC_Cross-Compiler#Installing_Dependencies for
information on which Debian and other OS packages you need to install so you can
compile GCC and binutils.

You will also need nasm and CMake to build this project. The versions from
Debian Bookworm work fine. The project is also setup to use QEMU and also the
version in Debian Bookworm works fine.

### Building the Toolchain

Tip: If CMake is setup with defaults you can use make to execute the build targets. You
don't have to use cmake --build

* Pick a root directory for the toolchain to be installed into. If you do not specify
  one then /usr/local will be used. If /usr/local is to be used then not all of the
  toolchain can be compiled as a non-root user before it is installed. You will have
  to compile binutils, gcc and libgcc first, then install them, then compile newlib,
  then install that, then compile libstdc++ and finally install that. There are build
  targets for each of those steps. Look in the toolchain/CMakeLists.txt file to
  find out what they are. This guide will assume the toolchain will be installed as
  your user.

* The bin directory inside the toolchain root must be in your path before you start
  building the toolchain. As well, the toolchain root must be specified using a fully
  qualified directory. Using ~ to specify your home directory will not work. This
  guide will assume your toolchain root directory will be /home/user/local/talos

* First, update your path by running export PATH="/home/user/local/talos/bin:$PATH"

* Create a directory to build the toolchain and TalOS. Running mkdir build inside
  the root directory of this project is a good choice.

* Move into your build directory and run cmake with
  -DBUILD_TOOLCHAIN=yes -DTOOLCHAIN_PREFIX=/home/user/local/talos/
  and the path to the root of this project. If you have a build directory inside
  this project then that looks like:
  cmake -DBUILD_TOOLCHAIN=yes -DTOOLCHAIN_PREFIX=/home/user/local/talos/ ..

* The install-toolchain build target will download, build, and install the entire
  toolchain. There are many ways to run the target. One example is
  nice cmake --build . -j4 -t install-toolchain
  from inside the build directory.

* Be patient. The toolchain build takes anywhere from 15 minutes on a fast machine
  to over an hour on a slower one.

* If you see an error related to being unable to find i686-talos-cc then the toolchain
  bin directory was not in your path correctly. Fix the error,
  remove the toolchain/toolchain-newlib-prefix/ directory inside the build directory,
  and start the install-toolchain target running again.

* Once the toolchain is built and installed you should be able to run
  i686-talos-gcc --version and see i686-talos-gcc (GCC) 14.2.0

### Building The Kernel

* If you just finished building the toolchain you have to move the build system
  into a mode to compile the kernel. It's a weird requirement but because of CMake
  caching it is a present necessity. Run cmake -DBUILD_TOOLCHAIN=no .. (assuming your
  build directory is inside this project root) to do this.

* Build the talos-bin target to compile the kernel. There are again many ways
  to do this but as an example try: nice cmake --build . -j4 -t talos-bin

### Running The Kernel

* The kernel outputs log messages to the VGA screen and to PC serial port #1.

* Once the kernel is built you can execute it inside QEMU with something like:
  qemu-system-i386 --kernel talos.bin

* It is possible to run the kernel with qemu-system-x86_64 and it will work aside
  from GDB will not work correctly.

* Once QEMU starts you should see some diagnostic information supplied while it
  boots then have a clock in the upper right hand corner counting uptime. That's it.
  That's all there is right now.

* You can run the run-qemu build target to compile the kernel and launch it inside QEMU
  all at once. There is also a run-qemu-debug target which will build and launch the
  kernel inside QEMU but have QEMU wait for GDB to attach to it before the kernel is
  booted. The run-gdb target will execute GDB and attach it to QEMU for you.

* The grub-iso target will compile the kernel and assemble it into a GRUB based
  bootable CD image which can be used with QEMU or booted on a non-UEFI PC after
  burning it to optical media or writing it to a USB thumb drive. An example
  of booting the GRUB iso with QEMU: qemu-system-i386 --cdrom grub.iso

* You can control the VGA and serial output from QEMU with CMAKE variables. Look
  at the CMakeLists.txt in platform/ibmpc to see the variables.

## Hacking

Note: This information could easily be out of date or stale.

Warning: The build system is blind to some dependencies. For instance if link.ld
is changed it will not automatically cause talos.bin to be relinked. For the places
this happens you'll find comments at the top of the files it applies to. File a bug
report if such a comment is missing.

Look in the [src/platform/ibmpc](src/platform/ibmpc) directory. It all starts with
[link.ld](src/platform/ibmpc/link.ld) which is the linker script used as the last
step of building talos.bin. The entry point is defined in there and it is the
boot() function.

The boot() function is implemented in assembly and exists in [crt0.asm](src/platform/ibmpc/crt0.asm).
crt0.asm calls the init() function, which is implemented in [init.cpp](src/platform/ibmpc/init.cpp),
then finally the start() function which is implemented in
[src/kernel/start.cpp](src/kernel/start.cpp).

The directory organization right now isn't great.

* [cmake/](cmake/) Holds the cross compile configuration

* [src/abi/](src/abi/) Mostly stuff related to keeping the libc and libstdc++ happy. Also where
  the heap/program break is implemented.

* [src/config/](src/config/) Header files that define platform and CPU specific configuration values.

* [src/contrib/](src/contrib/) Software from third parties that is incorporated with little or no modification.
  See below for more information about the contributed software.

* [src/cpu/](src/cpu/) CPU specific things like setting up the interupt vector, paging/TLB configuration, etc.

* [src/driver/](src/driver/) Hardware things that are not tied to any specific CPU (ie, i686) or platform (ie, IBM PC).
  The VGA implementation is in there as well as a generic UART interface and some others.

* [src/hal/](src/hal/) Hardware Abstraction Layer - currently header files that define an interface the
  CPU and platform implement.

* [src/kernel/](src/kernel/) The part that does kernel type things: schedule and execute timers, maintain a job list, etc

* [src/libk/](src/libk/) A library of stuff that's useful for all the different parts of the project. Wraps around the HAL,
  libc, libstdc++, etc.

* [src/platform/](src/platform/) Stuff related to making a bootable binary for the PC, doing hardware initialization, sending the
  log messages to COM1 and the VGA screen, etc.

* [toolchain/](toolchain/) The compiler, binutils, libc, etc

## Contributed Software

Right now two different projects are incorporated into the source tree: Dan Lea's
[malloc()](https://gee.cs.oswego.edu/dl/html/malloc.html) implementation and
Marco Paland's [embeded printf()](https://github.com/mpaland/printf) implementation.

### malloc

Newlib comes with it's own, older, version of Dan Lea's malloc incorporated in it but this is disabled. The newer
version of Dan's malloc is used with a different configuration and some custom wrapping. That software is licensed
under the MIT license.

### printf

Marco Paland's printf() is used to implement the libk::write* functions because it does not require malloc()
to be functional. It is not exposed as the libc printf() which is the one that comes from Newlib. There is a good amount
of logging that needs to happen before malloc() works which is why a printf() implementation that only uses the stack
was chosen to implement it. It has a limited feature set though which is why it is not exposed as the libc printf().

That software is also made available under the MIT license.

## License

All the original work from this author for the TalOS project is licensed under the terms of the GPL version 3. See the
LICENSE file for details. The software under src/contrib continues to be available, even if modified for this project,
under the terms of it's original license, including any modifications.
