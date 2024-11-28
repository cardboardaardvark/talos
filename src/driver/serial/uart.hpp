#include <cstddef>
#include <cstdint>

namespace driver
{

namespace serial
{

class UART
{
    public:
    using speed_t = std::uint32_t;

    UART() = default;
    UART(const UART&) = delete;
    UART& operator =(const UART&) = delete;
    virtual ~UART() = default;

    virtual void set_speed(speed_t new_speed) = 0;
    virtual speed_t get_speed() const = 0;

    virtual void write(char c) = 0;
    virtual void write(const void *buffer, std::size_t bytes);
};

} // namespace serial

} // namespace driver
