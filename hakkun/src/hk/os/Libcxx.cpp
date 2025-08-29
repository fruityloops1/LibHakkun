#include "hk/diag/diag.h"
#include "hk/os/Mutex.h"

struct Guard {
    bool initialized;
    hk::os::Mutex mutex;
};

extern "C" {

bool hk_guard_acquire(Guard* guard) {
    if (guard->initialized)
        return false;
    guard->mutex.lock();
    return true;
}

void hk_guard_release(Guard* guard) {
    guard->initialized = true;
    guard->mutex.unlock();
}

void hk_atexit(void (*)()) {
    // ...
}

u32 hk_stack_chk_guard = 0xFEFEFEFE;

/*void abort_message(const char* msg) {
    HK_ABORT("abort_message: %s", msg);
}*/

void abort() {
    HK_ABORT("abort() called", 0);
}
}
