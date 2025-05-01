#pragma once

#include "hk/svc/api.h"
#include "hk/types.h"

namespace hk::os {

    class Mutex {
        Handle mValue = cInvalidHandle;

        static constexpr Handle cInvalidHandle = 0;
        static constexpr Handle cWaitMask = 1 << 30;

        static u32 getCurrentThreadHandle() { return hk::svc::getTLS()->nnsdk_thread_ptr->handle; }

        class Lock {
            hk::os::Mutex& mMutex;

        public:
            Lock(Mutex& mutex)
                : mMutex(mutex) {
                mMutex.lock();
            }

            ~Lock() {
                mMutex.unlock();
            }

            Lock(const Lock&) = delete;
            Lock& operator=(const Lock&) = delete;

            Lock(Lock&& other) noexcept
                : mMutex(other.mMutex) {
            }

            Lock& operator=(Lock&& other) noexcept {
                if (this != &other) {
                    mMutex.unlock();
                    mMutex = other.mMutex;
                }
                return *this;
            }
        };

    public:
        void lock() {
            const Handle currentThread = getCurrentThreadHandle();

            while (true) {
                u32 value = svc::loadExclusive(&mValue);

                if (value == cInvalidHandle) {
                    if (svc::storeExclusive(&mValue, currentThread))
                        return;
                    continue;
                }

                if ((value & ~cWaitMask) == currentThread) {
                    svc::clearExclusive();
                    return;
                }

                if ((value & cWaitMask) == 0) {
                    if (!svc::storeExclusive(&mValue, value | cWaitMask))
                        continue;
                } else
                    svc::clearExclusive();

                hk::svc::ArbitrateLock(value & ~cWaitMask, uintptr_t(&mValue), currentThread);
            }
        }

        bool tryLock() {
            const Handle currentThread = getCurrentThreadHandle();
            u32 value = svc::loadExclusive(&mValue);

            while (true) {
                u32 value = svc::loadExclusive(&mValue);

                if (value != cInvalidHandle) {
                    svc::clearExclusive();
                    return false;
                }

                if (svc::storeExclusive(&mValue, currentThread))
                    return true;
            }
        }

        void unlock() {
            const Handle currentThread = getCurrentThreadHandle();

            while (true) {
                u32 value = svc::loadExclusive(&mValue);

                if ((value & ~cWaitMask) != currentThread) {
                    svc::clearExclusive();
                    break;
                }

                if (svc::storeExclusive(&mValue, cInvalidHandle)) {
                    if (value & cWaitMask)
                        hk::svc::ArbitrateUnlock(uintptr_t(&mValue));
                    return;
                }
            }
        }

        bool isLockedByCurrentThread() const {
            return (mValue & ~cWaitMask) == getCurrentThreadHandle();
        }

        Lock lockScoped() { return { *this }; }
    };

} // namespace hk::os
