#pragma once

#include "hk/svc/api.h"
#include "hk/svc/types.h"
#include "hk/types.h"

namespace hk::os {

    // that Shit doesnt work.
    class SystemEvent {
        svc::Handle mWriteHandle = 0;
        svc::Handle mReadHandle = 0;

        NON_COPYABLE(SystemEvent);
        NON_MOVABLE(SystemEvent);

    public:
        SystemEvent() {
            tie(mWriteHandle, mReadHandle) = HK_UNWRAP(svc::CreateEvent());
        }

        Result signal() {
            return svc::SignalEvent(mWriteHandle);
        }

        Result wait(s64 timeout = -1) const {
            return svc::WaitSynchronization(&mReadHandle, 1, timeout);
        }

        ~SystemEvent() {
            if (mWriteHandle != 0)
                svc::CloseHandle(mWriteHandle);
            if (mReadHandle != 0)
                svc::CloseHandle(mReadHandle);
        }

        Handle takeReadHandle() {
            Handle readHandle = mReadHandle;
            mReadHandle = 0;
            return mReadHandle;
        }
    };

} // namespace hk::os
