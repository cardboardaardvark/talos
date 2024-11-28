#pragma once

#include <functional>

namespace kernel
{

using kernel_job_t = std::function<void ()>;

class Job
{
    protected:
    Job() noexcept = default;

    public:
    virtual ~Job() noexcept = default;
    void operator ()() noexcept;

    virtual void run() noexcept = 0;
};

// This needs to be extended:
//   * Return a job id when the job is added to the queue.
//   * Allow jobs to be removed from the queue if they are not running.
//   * If there is an attempt to remove a job from the queue and the job
//     is running then block until the job is done running.
void add_job(const kernel_job_t& job);
kernel_job_t take_job();

} // namespace kernel
