#include "io.hpp"

namespace cpu
{

namespace x86
{

// RESTRICT No assert()
// RESTRICT No dynamic memory
unsigned char inb(io_port_t port) noexcept
{
    unsigned char value;

    __asm__ volatile
    (
        "inb %w1, %b0"
        : "=a"(value)
        : "Nd"(port)
        : "memory"
    );

    return value;
}

// RESTRICT No assert()
// RESTRICT No dynamic memory
void outb(io_port_t port, unsigned char byte) noexcept
{
    __asm__ volatile ("outb %1, %0" : : "dN" (port), "a" (byte));
}

} // namespace x86

} // namespace cpu
