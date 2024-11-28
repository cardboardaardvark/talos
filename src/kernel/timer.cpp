#include <atomic>
#include <forward_list>
#include <list>

#include <hal/interrupt.hpp>
#include <libk/assert.hpp>
#include <libk/error.hpp>
#include <libk/exceptions.hpp>
#include <libk/logging.hpp>
#include <libk/mutex.hpp>
#include <libk/util.hpp>

#include "job.hpp"
#include "timer.hpp"

namespace kernel
{

using timer_entry_t = struct
{
    timer_id_t id = 0;
    tick_counter_t expires = 0;
    interval_timer_t config;
    timer_handler_t handler;
};

using timer_queue_t = std::list<timer_entry_t *>;

static std::atomic<tick_counter_t> tick_counter = ATOMIC_VAR_INIT(0);
static std::atomic<timer_id_t> next_timer_id = ATOMIC_VAR_INIT(1);
static std::atomic<bool> check_timers_pending = ATOMIC_VAR_INIT(false);

static libk::spin_mutex_t timer_queue_mutex;
static timer_queue_t timer_queue;

tick_counter_t get_tick_count() noexcept
{
    return tick_counter.load(std::memory_order_acquire);
}

static timer_id_t get_timer_id() noexcept
{
    return next_timer_id.fetch_add(1, std::memory_order_acq_rel);
}

static void schedule_entry(timer_entry_t *needs_scheduling)
{
    assert(needs_scheduling != nullptr);
    assert_locked_mutex(timer_queue_mutex);

    if (timer_queue.empty()) {
        timer_queue.push_front(needs_scheduling);
    } else {
        timer_queue_t::iterator i;

        for (i = timer_queue.begin(); i != timer_queue.end();) {
            auto entry = *i;

            if (entry->expires >= needs_scheduling->expires) break;

            i++;
        }

        timer_queue.insert(i, needs_scheduling);
    }
}

timer_id_t start_timer(const interval_timer_t& config, timer_handler_t handler)
{
    const auto ticked_at = get_tick_count();

    if (config.initial == 0 && config.repeat == 0) throw libk::RuntimeError("Interval timer initial and repeat value cannot both be 0");
    if (handler == nullptr) throw libk::RuntimeError("Interval timer handler cannot be nullptr");

    const auto id = get_timer_id();
    auto interval = config.initial;

    if (interval == 0) interval = config.repeat;

    const auto expires = ticked_at + interval;
    auto entry = new timer_entry_t(id, expires, config, handler);

    libk::SpinLock lock(timer_queue_mutex);

    schedule_entry(entry);

    return id;
}

static void check_timers_job() noexcept
{
    assert(check_timers_pending.load(std::memory_order_consume) == true);

    auto now = get_tick_count();

    std::forward_list<timer_entry_t *> reschedule_list;

    libk::SpinLock lock(timer_queue_mutex);

    for (auto i = timer_queue.begin(); i != timer_queue.end();) {
        auto entry = *i;

        if (entry->expires > now) break;

        i = timer_queue.erase(i);

        add_job(entry->handler);

        if (entry->config.repeat) {
            entry->expires += entry->config.repeat;

            reschedule_list.push_front(entry);
        } else {
            delete entry;
        }
    }

    for (auto entry : reschedule_list) schedule_entry(entry);

    check_timers_pending.store(false, std::memory_order_release);
}

// Called as an ISR
void isr_tick_handler() noexcept
{
    tick_counter.fetch_add(1, std::memory_order_release);

    // No need to add a job to check the timers if a job is already in the queue
    // or running.
    if (check_timers_pending.load(std::memory_order_acquire)) return;

    check_timers_pending.store(true, std::memory_order_release);

    // To get out of the ISR as soon as possible schedule a job to check the contents
    // of the timer queue.
    add_job(check_timers_job);
}

} // namespace kernel
