#include <string>
#include <format>

namespace libk
{

template <typename... Args>
std::string formatted_string(const std::format_string<Args...> fmt, Args&&... args)
{
    return std::vformat(fmt.get(), std::make_format_args(args...));
}

} // namespace libk
