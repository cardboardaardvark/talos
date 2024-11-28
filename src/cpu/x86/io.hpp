#include <cstdint>

namespace cpu
{

namespace x86
{

using io_port_t = std::uint16_t;

unsigned char inb(io_port_t port) noexcept;
void outb(io_port_t port, unsigned char byte) noexcept;

} // namespace x86

} // namespace cpu
