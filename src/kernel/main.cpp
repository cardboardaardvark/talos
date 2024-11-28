#include <hal/interrupt.hpp>
#include <libk/error.hpp>
#include <libk/logging.hpp>
#include <libk/terminate.hpp>

#include "job.hpp"
#include "main.hpp"
#include "timer.hpp"

#define KERNEL_FREQ 5000

namespace kernel
{

static ShutdownAction main_loop()
{
    while(true) {
        while (true) {
            auto job = take_job();

            if (job == nullptr) break;

            job();
        }

        hal::wait();
    }

    return ShutdownAction::halt;
}

static void init()
{
    hal::set_tick_handler(isr_tick_handler);
    hal::set_tick_frequency(KERNEL_FREQ);
}

// By the time control reaches here the following must be true:
// * Paging is setup and dynamic memory can be allocated
// * Interrupts are disabled
extern "C" [[noreturn]] void start()
{

    init();

    hal::enable_interrupts(true);

    auto action = main_loop();

    switch (action) {
        case ShutdownAction::halt: libk::halt();
    }

    libk::panic("Unknown ShutdownAction: %u\n", static_cast<unsigned int>(action));
}

kernel_uptime_t uptime() noexcept
{
    return get_tick_count() / KERNEL_FREQ;
}

} // namespace kernel
