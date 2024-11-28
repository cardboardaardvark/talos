#include <cstdarg>
#include <cstddef>

namespace libk
{

void write_log(const char *message);
void write_log(const char *message, std::size_t length);
void writef_log(const char *format, ...);
void vwritef_log(const char *format, std::va_list args);

} // namespace kernel
