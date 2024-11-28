#include <cstdarg>

#include <hal/io.hpp>

#include "logging.hpp"

extern "C"
{

// Function definitions from mpaland printf
// because including it's header does nasty things
// like #define printf printf_
int vprintf_(const char* format, va_list va);

// Called by mpaland printf family of functions
// to receive their output.
void _putchar(char c)
{
    hal::log_put_char(c);
}

} // extern "C"

namespace libk
{

void print(const char *message)
{
    while (*message != '\0') {
        hal::log_put_char(*message);
        message++;
    }
}

void print(const char *message, std::size_t length)
{
    while(*message != '\0' && length-- > 0) {
        hal::log_put_char(*message);
        message++;
    }
}

void printf(const char *format, ...)
{
    std::va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void vprintf(const char *format, std::va_list args)
{
    // Use mpaland's vprintf because it works only
    // using the stack and logging needs to work
    // before dynamic memory allocation is available.
    vprintf_(format, args);
}

} // namespace libk
