extern "C"
{

// The kernel doesn't really ever exit by some traditional definition so the stuff registered
// with atexit() don't matter. For now return success but ignore what ever was supposed to be
// registered.
int __cxa_atexit([[maybe_unused]] void (*func) (void *), [[maybe_unused]] void *arg, [[maybe_unused]] void *dso_handle)
{
    return 0;
}

} // extern "C"
