#include <cstddef>
#include <cstdint>

#include <hal/memory.hpp>

namespace kernel
{

using kernel_uptime_t = uint32_t;

enum class ShutdownAction : unsigned int
{
    halt = 0,
};

extern "C" [[noreturn]] void start();
kernel_uptime_t uptime() noexcept;

} // namespace kernel
