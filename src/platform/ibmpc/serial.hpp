#include <cstdint>

#include <cpu/x86/io.hpp>
#include <driver/serial/uart.hpp>

#define COM1_IO_PORT 0x3F8

namespace platform
{

namespace ibmpc
{

class ComPort : public driver::serial::UART
{
    private:
    cpu::x86::io_port_t m_port;

    public:
    ComPort(cpu::x86::io_port_t port, speed_t speed);

    virtual void set_speed(speed_t speed) override;
    virtual speed_t get_speed() const override;
    bool transmit_ready() const;
    using driver::serial::UART::write;
    virtual void write(char c) override;
};

} // namespace ibmpc

} // namespace platform
