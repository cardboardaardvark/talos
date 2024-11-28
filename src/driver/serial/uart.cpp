#include <cstring>

#include "uart.hpp"

namespace driver
{

namespace serial
{

void UART::write(const void *buffer, std::size_t bytes)
{
    auto p = reinterpret_cast<const char *>(buffer);

    for (std::size_t i = 0; i < bytes; i++) {
        write(p[i]);
    }
}

} // namespace serial

} // namespace driver
