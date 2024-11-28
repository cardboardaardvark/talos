#include <hal/io.hpp>

#include "console.hpp"
#include "serial.hpp"

#define SERIAL_PORT COM1_IO_PORT
#define SERIAL_SPEED 115200

namespace platform
{

namespace ibmpc
{

static ComPort com1(SERIAL_PORT, SERIAL_SPEED);

} // namespace ibmpc

} // namespace platform

namespace hal
{

void log_put_char(unsigned char c)
{
    platform::ibmpc::console_put_char(c);
    platform::ibmpc::com1.write(c);
}

} // namespace hal
