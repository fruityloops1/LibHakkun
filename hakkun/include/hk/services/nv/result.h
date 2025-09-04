
#pragma once

#include "hk/Result.h"

namespace hk::nvdrv {

    HK_RESULT_MODULE(412)
    HK_DEFINE_RESULT_RANGE(NvidiaDriver, 50, 70)

    HK_DEFINE_RESULT(Unimplemented, 50)
    HK_DEFINE_RESULT(Unsupported, 51)
    HK_DEFINE_RESULT(Uninitialized, 52)
    HK_DEFINE_RESULT(BadArgument, 53)
    HK_DEFINE_RESULT(Timeout, 54)
    HK_DEFINE_RESULT(OutOfMemory, 55)
    HK_DEFINE_RESULT(ReadOnly, 56)
    HK_DEFINE_RESULT(InvalidState, 57)
    HK_DEFINE_RESULT(InvalidAddress, 58)
    HK_DEFINE_RESULT(InvalidSize, 59)
    HK_DEFINE_RESULT(InvalidValue, 60)
    HK_DEFINE_RESULT(AlreadyAllocated, 61)
    HK_DEFINE_RESULT(Busy, 62)
    HK_DEFINE_RESULT(ResourceError, 63)
    HK_DEFINE_RESULT(CountMismatch, 64)
    HK_DEFINE_RESULT(SharedMemoryTooSmall, 65)
    HK_DEFINE_RESULT(OperationFailed, 66)
    HK_DEFINE_RESULT(IoctlFailed, 67)
    HK_DEFINE_RESULT(UnhandledError, 68)

    Result convertErrorToResult(u32 error); 
} // namespace hk::nvdrv
