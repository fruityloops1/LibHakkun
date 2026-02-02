
#pragma once

#include "hk/Result.h"
#ifndef HK_COLLECTING_RESULTNAMES
#include "hk/ResultMap.h"
#endif

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

#ifndef HK_COLLECTING_RESULTNAMES
    using ResultMapNvidia = ResultMap<int,
        { 0, ResultSuccess() },
        { 1, ResultUnimplemented() },
        { 2, ResultUnsupported() },
        { 3, ResultUninitialized() },
        { 4, ResultBadArgument() },
        { 5, ResultTimeout() },
        { 6, ResultOutOfMemory() },
        { 7, ResultReadOnly() },
        { 8, ResultInvalidState() },
        { 9, ResultInvalidAddress() },
        { 10, ResultInvalidSize() },
        { 11, ResultInvalidValue() },
        { 13, ResultAlreadyAllocated() },
        { 14, ResultBusy() },
        { 15, ResultResourceError() },
        { 16, ResultCountMismatch() },
        { 0x1000, ResultSharedMemoryTooSmall() },
        { 0x30003, ResultOperationFailed() },
        { 0x3000F, ResultIoctlFailed() }>;
#endif

} // namespace hk::nvdrv
