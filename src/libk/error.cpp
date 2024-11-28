#include <cstdarg>

#include <hal/interrupt.hpp>
#include <libk/logging.hpp>

#include "error.hpp"
#include "terminate.hpp"

namespace libk
{

[[noreturn]] void panic(const char *format, ...)
{
    hal::enable_interrupts(false);

    std::va_list args;

    va_start(args, format);
    libk::print("PANIC: ");
    libk::vprintf(format, args);
    va_end(args);

    halt();
}

} // namespace libk
