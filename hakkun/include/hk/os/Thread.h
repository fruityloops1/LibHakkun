#pragma once

#include "hk/hook/MapUtil.h"
#include "hk/svc/api.h"
#include "hk/svc/types.h"
#include "hk/types.h"
#include <cstdlib>

namespace hk::os {

    class Thread : svc::ThreadType {
        ptr mOwnedStack = 0;
        ptr mStack = 0;
        ptr mStackMap = 0;
        size mStackSize = 0;
        svc::ThreadFunc mThreadFunc = nullptr;
        ptr mThreadFuncArg = 0;

        template <typename Arg>
        using ThreadFunc = void (*)(Arg*);

        static void threadEntry(ptr thread) { cast<Thread*>(thread)->threadEntry(); }
        void threadEntry();
        static constexpr size cDefaultStackSize = cPageSize;

        NON_COPYABLE(Thread);
        NON_MOVABLE(Thread);

    public:
        template <typename Arg>
        Thread(ThreadFunc<Arg> func, Arg* arg, ptr stack = 0, size stackSize = cDefaultStackSize, s32 priority = 44, s32 coreId = -2)
            : mStack(stack)
            , mStackSize(stackSize)
            , mThreadFunc(cast<svc::ThreadFunc>(func))
            , mThreadFuncArg(ptr(arg)) {
            HK_ABORT_UNLESS(isAlignedPage(stack), "Stack must be aligned to page size (0x%zx)", cPageSize);
            HK_ABORT_UNLESS(isAlignedPage(stackSize), "Stack size must be aligned to page size (0x%zx)", cPageSize);

            if (stack == 0)
                mOwnedStack = mStack = stack = ptr(aligned_alloc(cPageSize, stackSize));

            mStackMap = hook::findStack(mStackSize);
            HK_ABORT_UNLESS_R(svc::MapMemory(mStackMap, stack, mStackSize));

            this->handle = HK_UNWRAP(svc::CreateThread(threadEntry, ptr(this), mStackMap + mStackSize, priority, coreId));
        }

        Result start() {
            return svc::StartThread(this->handle);
        }

        void setName(const char* name) {
            strncpy(this->threadName, name, sizeof(this->threadName));
        }

        ~Thread() {
            if (mOwnedStack != 0)
                free(cast<void*>(mOwnedStack));
            if (this->handle != 0)
                svc::CloseHandle(this->handle);
            if (mStackMap != 0)
                svc::UnmapMemory(mStackMap, mStack, mStackSize);
        }
    };

} // namespace hk::os
