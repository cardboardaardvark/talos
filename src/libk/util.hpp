#include <functional>

namespace libk
{

class Finally
{
    public:
    using handler_t = std::function<void ()>;

    private:
    bool m_armed = false;
    handler_t m_handler = nullptr;

    public:
    Finally(handler_t handler) noexcept;
    Finally(const Finally&) = delete;
    Finally& operator =(const Finally&) = delete;
    ~Finally() noexcept;

    bool armed() const noexcept;
    void disarm() noexcept;
    void fire() noexcept;
};

class DisableInterrupts
{
    private:
    bool m_need_enable;

    public:
    DisableInterrupts() noexcept;
    DisableInterrupts(const DisableInterrupts&) = delete;
    DisableInterrupts& operator =(const DisableInterrupts&) = delete;
    ~DisableInterrupts() noexcept;
};

} // namespace libk
