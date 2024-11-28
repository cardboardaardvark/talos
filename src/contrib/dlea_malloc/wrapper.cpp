#include <cstdint>
#include <cstdlib>

#include <libk/util.hpp>

extern "C"
{

void * dlcalloc(size_t nmemb, size_t size);
void dlfree(void *ptr);
void * dlmalloc(size_t size);
void * dlrealloc(void *ptr, size_t size);

// The memory allocation functions can be called from inside and outside of
// ISRs. If control is inside of the memory allocation functions outside
// of an ISR, then an interrupt happens, and an ISR calls one of the functions,
// the call winds up reentrant. Dan Lea's malloc is setup with spinlocks which
// will nicely hang instead of getting all weird but the reentrancy still needs
// to be prevented. This is handled by disabling interrupts while these
// functions are called.

void * calloc(size_t nmemb, size_t size)
{
    libk::DisableInterrupts guard;
    return dlcalloc(nmemb, size);
}

void free(void *ptr)
{
    libk::DisableInterrupts guard;
    return dlfree(ptr);
}

void * malloc(size_t bytes)
{
    libk::DisableInterrupts guard;
    return dlmalloc(bytes);
}

void * realloc(void *ptr, size_t size)
{
    libk::DisableInterrupts guard;
    return dlrealloc(ptr, size);
}

} // extern "C"
