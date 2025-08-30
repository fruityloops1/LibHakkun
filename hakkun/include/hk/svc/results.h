#pragma once

#include "hk/Result.h"

namespace hk::svc {

    HK_RESULT_MODULE(1)
    HK_DEFINE_RESULT_RANGE(Kernel, 0, 520)

    HK_DEFINE_RESULT(OutOfSessions, 7);

    HK_DEFINE_RESULT(InvalidArgument, 14);

    HK_DEFINE_RESULT(NotImplemented, 33);

    HK_DEFINE_RESULT(StopProcessingException, 54);

    HK_DEFINE_RESULT(NoSynchronizationObject, 57);

    HK_DEFINE_RESULT(TerminationRequested, 59);

    HK_DEFINE_RESULT(NoEvent, 70);

    HK_DEFINE_RESULT(InvalidSize, 101);
    HK_DEFINE_RESULT(InvalidAddress, 102);
    HK_DEFINE_RESULT(OutOfResource, 103);
    HK_DEFINE_RESULT(OutOfMemory, 104);
    HK_DEFINE_RESULT(OutOfHandles, 105);
    HK_DEFINE_RESULT(InvalidCurrentMemory, 106);

    HK_DEFINE_RESULT(InvalidNewMemoryPermission, 108);

    HK_DEFINE_RESULT(InvalidMemoryRegion, 110);

    HK_DEFINE_RESULT(InvalidPriority, 112);
    HK_DEFINE_RESULT(InvalidCoreId, 113);
    HK_DEFINE_RESULT(InvalidHandle, 114);
    HK_DEFINE_RESULT(InvalidPointer, 115);
    HK_DEFINE_RESULT(InvalidCombination, 116);
    HK_DEFINE_RESULT(TimedOut, 117);
    HK_DEFINE_RESULT(Cancelled, 118);
    HK_DEFINE_RESULT(OutOfRange, 119);
    HK_DEFINE_RESULT(InvalidEnumValue, 120);
    HK_DEFINE_RESULT(NotFound, 121);
    HK_DEFINE_RESULT(Busy, 122);
    HK_DEFINE_RESULT(SessionClosed, 123);
    HK_DEFINE_RESULT(NotHandled, 124);
    HK_DEFINE_RESULT(InvalidState, 125);
    HK_DEFINE_RESULT(ReservedUsed, 126);
    HK_DEFINE_RESULT(NotSupported, 127);
    HK_DEFINE_RESULT(Debug, 128);
    HK_DEFINE_RESULT(NoThread, 129);
    HK_DEFINE_RESULT(UnknownThread, 130);
    HK_DEFINE_RESULT(PortClosed, 131);
    HK_DEFINE_RESULT(LimitReached, 132);
    HK_DEFINE_RESULT(InvalidMemoryPool, 133);

    HK_DEFINE_RESULT(ReceiveListBroken, 258);
    HK_DEFINE_RESULT(OutOfAddressSpace, 259);
    HK_DEFINE_RESULT(MessageTooLarge, 260);

    HK_DEFINE_RESULT(InvalidProcessId, 517);
    HK_DEFINE_RESULT(InvalidThreadId, 518);
    HK_DEFINE_RESULT(InvalidId, 519);
    HK_DEFINE_RESULT(ProcessTerminated, 520);

} // namespace hk::svc
