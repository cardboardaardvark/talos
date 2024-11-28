#pragma once

namespace libk
{

// Needs to be callable by the libc implementation
extern "C" [[noreturn]] void panic(const char *format, ...);

} // namespace libk
