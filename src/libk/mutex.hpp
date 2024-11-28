#pragma once

#include <cstdint>

namespace libk
{

using mutex_state_t = int;

enum class MutexState : mutex_state_t
{
    unlocked = 0,
    locked = 1,
};

// General purpose spin lock based mutex
typedef struct alignas(PAGE_SIZE)
{
    // Make sure the storage for the atomic operations is the
    // first member of the struct so it is the thing aligned
    // according to the page size specified in alignas()
    MutexState state = MutexState::unlocked;
} spin_mutex_t;

// This mutex needs to be used in places where execution can happen
// inside an ISR. It has debug checks to ensure interrupts are disabled
// to avoid reentrancy issues that exist when execution can happen
// inside and outside of an ISR.
typedef struct alignas(PAGE_SIZE)
{
    // Make sure the storage for the atomic operations is the
    // first member of the struct so it is the thing aligned
    // according to the page size specified in alignas()
    MutexState state = MutexState::unlocked;
} isr_spin_mutex_t;

template <typename T>
class SpinLock
{
    private:
    T& m_mutex;

    public:
    SpinLock(const T&) = delete;
    SpinLock& operator=(const T&) = delete;

    SpinLock(T& mutex) noexcept
    : m_mutex(mutex)
    {
        lock_mutex(m_mutex);
    }

    ~SpinLock()
    {
        unlock_mutex(m_mutex);
    }

    void lock() noexcept;
    void unlock() noexcept;
};

void lock_mutex(spin_mutex_t& mutex) noexcept;
void unlock_mutex(spin_mutex_t& mutex) noexcept;
MutexState mutex_state(spin_mutex_t& mutex) noexcept;

void lock_mutex(isr_spin_mutex_t& mutex) noexcept;
void unlock_mutex(isr_spin_mutex_t& mutex) noexcept;
MutexState mutex_state(isr_spin_mutex_t& mutex) noexcept;

}
