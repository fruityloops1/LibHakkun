#include "hk/os/Thread.h"

namespace hk::os {

    void Thread::threadEntry() {
        auto* tls = svc::getTLS();
        new (tls) svc::ThreadLocalRegion;

        tls->nnsdkThread = this;

        mThreadFunc(mThreadFuncArg);

        svc::ExitThread();
    }

} // namespace hk::os
