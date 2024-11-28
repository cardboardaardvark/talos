#include <hal/interrupt.hpp>
#include <libk/assert.hpp>
#include <libk/logging.hpp>
#include <libk/util.hpp>

#include "mutex.hpp"

namespace libk
{

// Using TTAS (test and test-and-set) algorithm as described at https://rigtorp.se/spinlock/
// RESTRICT No assert()
// RESTRICT No dynamic memory
void lock_mutex(spin_mutex_t& mutex) noexcept
{
#ifndef NDEBUG
    // Make sure the normal mutex is not used from inside an ISR. There is another
    // type of mutex that should be used when execution can happen inside an ISR.
    // Can't use assert. The infinite loop will be easy to find with GDB.
    if (hal::inside_isr()) {
        while (true) hal::wait();
    }
#endif

    while (true) {
        // If the returned value is locked then something else set it.
        // We get unlocked back if this call set the value to locked
        if (__sync_lock_test_and_set(reinterpret_cast<mutex_state_t *>(&mutex.state), static_cast<mutex_state_t>(MutexState::locked)) == static_cast<mutex_state_t>(MutexState::unlocked)) {
            break;
        }

        while (mutex_state(mutex) == MutexState::locked) {
            __builtin_ia32_pause();
        }
    }
}

// RESTRICT No assert()
// RESTRICT No dynamic memory
void unlock_mutex(spin_mutex_t& mutex) noexcept
{
    __atomic_store_n(reinterpret_cast<mutex_state_t *>(&mutex.state), static_cast<mutex_state_t>(MutexState::unlocked), __ATOMIC_RELEASE);
}

// RESTRICT No assert()
// RESTRICT No dynamic memory
MutexState mutex_state(spin_mutex_t& mutex) noexcept
{
    return static_cast<MutexState>(__atomic_load_n(reinterpret_cast<mutex_state_t *>(&mutex.state), __ATOMIC_RELAXED));
}

void lock_mutex(isr_spin_mutex_t& mutex) noexcept
{
#ifndef NDEBUG
    // Make sure interrupts are disabled when trying to lock the mutex.
    // This avoids reentrancy problems that exist when a mutex can be used
    // inside and outside of an ISR.
    // Can't use assert. The infinite loop will be easy to find with GDB.
    if (hal::interrupts_enabled()) {
        while (true) hal::wait();
    }
#endif

    while (true) {
        // If the returned value is locked then something else set it.
        // We get unlocked back if this call set the value to locked
        if (__sync_lock_test_and_set(reinterpret_cast<mutex_state_t *>(&mutex.state), static_cast<mutex_state_t>(MutexState::locked)) == static_cast<mutex_state_t>(MutexState::unlocked)) {
            break;
        }

        while (mutex_state(mutex) == MutexState::locked) {
            __builtin_ia32_pause();
        }
    }
}

// RESTRICT No assert()
// RESTRICT No dynamic memory
void unlock_mutex(isr_spin_mutex_t& mutex) noexcept
{
    __atomic_store_n(reinterpret_cast<mutex_state_t *>(&mutex.state), static_cast<mutex_state_t>(MutexState::unlocked), __ATOMIC_RELEASE);
}

// RESTRICT No assert()
// RESTRICT No dynamic memory
MutexState mutex_state(isr_spin_mutex_t& mutex) noexcept
{
    return static_cast<MutexState>(__atomic_load_n(reinterpret_cast<mutex_state_t *>(&mutex.state), __ATOMIC_RELAXED));
}

} // namespace libk
