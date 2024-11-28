#pragma once

#include <cstdint>
#include <functional>

namespace kernel
{

using tick_counter_t = uint64_t;
using timer_handler_t = std::function<void ()>;
using timer_id_t = uint32_t;
using timer_interval_t = uint32_t;

typedef struct
{
    timer_interval_t initial = 0;
    timer_interval_t repeat = 0;
} interval_timer_t;

void isr_tick_handler() noexcept;
tick_counter_t get_tick_count() noexcept;

timer_id_t start_timer(const interval_timer_t& config, timer_handler_t handler);

// When this is implemented it needs to have the following behavior:
// * Returns false if the timer id was not in the timer queue (no such timer)
// * Removes the timer from the timer queue
// * If the timer has a job in the kernel job queue that is removed too
// * If the timer has a job in the kernel job queue and the timer is executing
//   then this call blocks until the job is done executing.
//
// When this call returns the caller must be able to depend on nothing
// that the timer does/did will happen in the future.
bool cancel_timer(timer_id_t timer) noexcept;

} // namespace kernel
