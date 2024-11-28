#include "assert.hpp"
#include "exceptions.hpp"

namespace libk
{

RuntimeError::RuntimeError(const char *message) noexcept
: std::runtime_error(message)
{
    assert(message != nullptr);
}

} // namespace libk
