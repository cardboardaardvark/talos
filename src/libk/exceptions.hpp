#pragma once

#include <stdexcept>

namespace libk
{

class RuntimeError : public std::runtime_error
{

    public:
    RuntimeError(const char *message) noexcept;
};

} // namespace libk
