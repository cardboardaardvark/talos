#include <cstddef>
#include <cstdint>

#include <hal/memory.hpp>

namespace kernel
{

enum class ShutdownAction : unsigned int
{
    halt = 1,
    poweroff = 2,
};

extern "C" [[noreturn]] void start();

} // namespace kernel
