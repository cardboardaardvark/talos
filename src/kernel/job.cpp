#include <list>

#include <hal/interrupt.hpp>
#include <libk/assert.hpp>
#include <libk/mutex.hpp>
#include <libk/util.hpp>

#include "job.hpp"

namespace kernel
{

static libk::isr_spin_mutex_t job_queue_mutex;
static std::list<kernel_job_t> job_queue;

void Job::operator ()() noexcept
{
    run();
}

// This can be called from inside or outside an ISR.
void add_job(const kernel_job_t& job)
{
    assert(job != nullptr);

    {
        libk::DisableInterrupts interrupts_guard;
        libk::SpinLock lock(job_queue_mutex);

        job_queue.emplace_back(job);
    }

    hal::unwait();
}

// This can be called from inside or outside an ISR.
kernel_job_t take_job()
{
    libk::DisableInterrupts interrupts_guard;
    libk::SpinLock lock(job_queue_mutex);

    if (job_queue.empty()) return nullptr;

    auto job = job_queue.front();
    job_queue.pop_front();

    return job;
}

} // namespace kernel
