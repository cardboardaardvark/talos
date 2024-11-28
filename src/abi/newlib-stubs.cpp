#include <cerrno>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <libk/logging.hpp>
#include <libk/error.hpp>

extern "C"
{

void * _calloc_r(struct _reent *, size_t nmemb, size_t size)
{
    return calloc(nmemb, size);
}

void _free_r(struct _reent *, void *ptr)
{
    free(ptr);
}

void * _malloc_r(struct _reent *, size_t bytes)
{
    return malloc(bytes);
}

void * _realloc_r(struct _reent *, void *ptr, size_t size)
{
    return realloc(ptr, size);
}

[[noreturn]] void abort()
{
    libk::panic("abort() called\n");
}

// libstdc++ calls this - I think as a part of global initialization
int getentropy([[maybe_unused]] void *buffer, [[maybe_unused]] size_t length)
{
    return ENOSYS;
}

_READ_WRITE_RETURN_TYPE write([[maybe_unused]] int file, const void *ptr, size_t len)
{
    libk::write_log(reinterpret_cast<const char *>(ptr), len);

    return len;
}

// Things that will never be implemented. Would be nice
// to get the libc to not even know about the ones that aren't
// a part of normal C such as the Unix and POSIX functions.
[[noreturn]] void _exit(int status)
{
    libk::panic("exit() called with status of %i\n", status);
}

int close(int file)
{
    libk::panic("close() called; file=%i\n", file);
}

int fstat(int file, [[maybe_unused]] struct stat *st)
{
    libk::panic("fstat() called: file=%i\n", file);
}

pid_t getpid(void)
{
    return 0;
}

int gettimeofday([[maybe_unused]] struct timeval *tv, [[maybe_unused]] struct timezone *tz)
{
    libk::panic("gettimeofday() called\n");
}

int isatty(int file)
{
    libk::panic("isatty() called; file=%i\n", file);
}

int kill(pid_t pid, int signal)
{
    libk::panic("kill() called; pid=%u signal=%i\n", pid, signal);
}

off_t lseek(int file, [[maybe_unused]] off_t ptr, [[maybe_unused]] int dir)
{
    libk::panic("lseek() called; file=%u\n", file);
}

_READ_WRITE_RETURN_TYPE read(int file, [[maybe_unused]] void *ptr, [[maybe_unused]] size_t len)
{
    libk::panic("read() called; file=%u\n", file);
}

// TODO: Implement these even if they just panic()
// char **environ; /* pointer to array of char * strings that define the current environment variables */
// int execve(char *name, char **argv, char **env);
// int fork();
// int fstat(int file, struct stat *st);
// int link(char *old, char *new);
// int open(const char *name, int flags, ...);
// int stat(const char *file, struct stat *st);
// clock_t times(struct tms *buf);
// int unlink(char *name);
// int wait(int *status);
// int gettimeofday(struct timeval * restrict,  void * restrict);

} // extern "C"
