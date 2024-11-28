#include <unistd.h>

#include <libk/error.hpp>

namespace abi
{

extern "C" long sysconf(int name)
{
    switch (name) {
        case _SC_PAGE_SIZE: return PAGE_SIZE;
    }

    libk::panic("sysconf() called with unsupported name: %i\n", name);
}

} // namespace abi
