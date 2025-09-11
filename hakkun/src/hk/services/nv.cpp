#include "hk/services/nv/result.h"
#include "hk/diag/diag.h"
#include "hk/services/nv/service.h"
#include "hk/util/Singleton.h"

namespace hk::nvdrv {
    HK_SINGLETON_IMPL(NvidiaDriver);
    Result convertErrorToResult(u32 error) {
        switch (error) {
        case 0:
            return ResultSuccess();
        case 1:
            return ResultUnimplemented();
        case 2:
            return ResultUnsupported();
        case 3:
            return ResultUninitialized();
        case 4:
            return ResultBadArgument();
        case 5:
            return ResultTimeout();
        case 6:
            return ResultOutOfMemory();
        case 7:
            return ResultReadOnly();
        case 8:
            return ResultInvalidState();
        case 9:
            return ResultInvalidAddress();
        case 10:
            return ResultInvalidSize();
        case 11:
            return ResultInvalidValue();
        case 13:
            return ResultAlreadyAllocated();
        case 14:
            return ResultBusy();
        case 15:
            return ResultResourceError();
        case 16:
            return ResultCountMismatch();
        case 0x1000:
            return ResultSharedMemoryTooSmall();
        case 0x30003:
            return ResultOperationFailed();
        case 0x3000F:
            return ResultIoctlFailed();
        default:
            diag::logLine("received unhandled error %d/0x%x from Nvidia Driver", error, error);
            return ResultUnhandledError();
        }
    }
}
