#include "hk/diag/ipclogger.h"
namespace hk::diag::ipclogger {
    IpcLogger* IpcLogger::instance() {
        if (sInstance.mSession == 0) {
            auto result = svc::ConnectToNamedPort("hklog");
            if (result.hasValue())
                sInstance.mSession = result.value();
            else
                sInstance.mSession = cInvalidSession;
        }

        return &sInstance;
    }
}
