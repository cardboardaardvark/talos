#include <unistd.h>

#include <config-memory.hpp>
#include <libk/error.hpp>

namespace abi
{

extern "C" long sysconf(int name)
{
    switch (name) {
        case _SC_PAGE_SIZE: return hal::page_size;
    }

    libk::panic("sysconf() called with unsupported name: %i\n", name);
}

} // namespace abi
