#include "hk/util/Context.h"

namespace hk::util {

    struct StackFrame {
        StackFrame* fp;
        ptr lr;
    };

    ptr getReturnAddress(int n) {
        StackFrame* fp;
        asm volatile("mov %0, fp" : "=r"(fp));

        for (int i = 0; i < n; i++) {
            if (fp == nullptr || ptr(fp) % sizeof(ptr) != 0)
                return 0;

            fp = fp->fp;
        }

        return fp->lr;
    }

} // namespace hk::util
