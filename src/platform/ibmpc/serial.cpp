#include <cstdio>
#include <cstdlib>

#include <cpu/x86/io.hpp>
// #include <kernel/error.hpp>
#include <libk/logging.hpp>

#include "serial.hpp"

#define IO_REGISTER 0 // DLAB off
#define SPEED_LOW_REGISTER 0 // DLAB on
#define INT_ENABLE_REGISTER 1 // DLAB off
#define SPEED_HIGH_REGISTER 1 // DLAB on
#define INT_IDENTIFICATION_REGISTER 2 // when reading
#define FIFO_CONTROL_REGISTER 2 // when writing
#define LINE_CONTROL_REGISTER 3
#define MODEM_CONTROL_REGISTER 4
#define LINE_STATUS_REGISTER 5
#define MODEM_STATUS_REGISTER 6
#define SCRATCH_REGISTER 7

#define MODEM_CONTROL_DTR (1 << 0)
#define MODEM_CONTROL_DTS (1 << 1)
#define MODEM_CONTROL_IRQ (1 << 3)
#define MODEM_CONTROL_LOOPBACK (1 << 4)

#define LINE_STATUS_TX_BUFFER_EMPTY (1 << 5)

// Divisor Latch Access Bit
#define LINE_CONTROL_DLAB (1 << 7)

#define UART_CLOCK_HZ 115200

namespace platform
{

namespace ibmpc
{

namespace x86 = cpu::x86;

// It's a National Semiconductor 16550 compatible UART
ComPort::ComPort(x86::io_port_t port, speed_t speed)
:m_port(port)
{
    // Disable all interrupts
    x86::outb(m_port + INT_ENABLE_REGISTER, 0);
    // Set the usual 8 bits, no parity, one stop bit
    x86::outb(m_port + LINE_CONTROL_REGISTER, 0x03);
    // Enable FIFO, clear them, with 14-byte threshold
    x86::outb(m_port+ FIFO_CONTROL_REGISTER, 0xC7);
    // Set Data Terminal Ready
    x86::outb(m_port + MODEM_CONTROL_REGISTER, MODEM_CONTROL_DTR);
    x86::outb(m_port + MODEM_CONTROL_REGISTER, 0x0B);

    set_speed(speed);
}

void ComPort::set_speed([[maybe_unused]] speed_t speed)
{
    if (speed == 0 || speed > UART_CLOCK_HZ || UART_CLOCK_HZ % speed != 0) {
        std::abort();
        // throw kernel::InvalidArgumentError("Invalid speed requested");
    }

    auto line_control = x86::inb(m_port + LINE_CONTROL_REGISTER);
    std::uint16_t divisor = UART_CLOCK_HZ / speed;

    // Set the speed of the port
    x86::outb(m_port + LINE_CONTROL_REGISTER, line_control | LINE_CONTROL_DLAB);
    x86::outb(m_port + SPEED_LOW_REGISTER, divisor & 0xFF);
    x86::outb(m_port + SPEED_HIGH_REGISTER, (divisor & 0xFF00) >> 8);
    x86::outb(m_port + LINE_CONTROL_REGISTER, line_control & ~LINE_CONTROL_DLAB);
}

ComPort::speed_t ComPort::get_speed() const
{
    auto line_control = x86::inb(m_port + LINE_CONTROL_REGISTER);
    auto with_dlab = line_control |= LINE_CONTROL_DLAB;
    std::uint16_t divisor = 0;

    x86::outb(m_port + LINE_CONTROL_DLAB, with_dlab);
    divisor |= x86::inb(m_port + SPEED_HIGH_REGISTER) << 8;
    divisor |= x86::inb(m_port + SPEED_LOW_REGISTER);

    x86::outb(m_port + LINE_CONTROL_REGISTER, line_control);

    return UART_CLOCK_HZ / divisor;
}

// Returns true if the transmit buffer is empty / writing
// will not block.
bool ComPort::transmit_ready() const
{
    if (x86::inb(m_port + LINE_STATUS_REGISTER) & LINE_STATUS_TX_BUFFER_EMPTY) {
        return true;
    }

    return false;
}

void ComPort::write(char c)
{
    while (! transmit_ready()) {
        // Block by spinning until there is room in the transmit buffer
    }

    x86::outb(m_port + IO_REGISTER, c);
}

} // namespace ibmpc

} // namespace platform
