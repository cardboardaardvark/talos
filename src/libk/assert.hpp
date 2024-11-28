#pragma once

#include "error.hpp"

#ifndef NDEBUG
#define assert(expression) { if (! (expression)) { ::libk::panic("Assertion failed at %s:%u: %s\n", __FILE__, __LINE__, #expression); } }
#define assert_locked_mutex(mutex) { if (::libk::mutex_state(mutex) != ::libk::MutexState::locked) { ::libk::panic("Mutex %s is not locked at %s:%u\n", #mutex, __FILE__, __LINE__); } }
#else
#define assert(expression)
#define assert_locked_mutex(mutex)
#endif
