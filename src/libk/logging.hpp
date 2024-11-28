#include <cstdarg>
#include <cstddef>

namespace libk
{

void print(const char *message);
void print(const char *message, std::size_t length);
void printf(const char *format, ...);
void vprintf(const char *format, std::va_list args);

} // namespace kernel
