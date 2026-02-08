#pragma once

#include "hk/Result.h"

namespace hk::vi {

    HK_RESULT_MODULE(412)
    HK_DEFINE_RESULT_RANGE(Vi, 70, 90)

    HK_DEFINE_RESULT(NotInitialized, 70)
    HK_DEFINE_RESULT(InvalidParcelSize, 71)
    HK_DEFINE_RESULT(InvalidPayloadSize, 72)

    HK_DEFINE_RESULT(BinderPermissionDenied, 74)
    HK_DEFINE_RESULT(BinderNameNotFound, 77)
    HK_DEFINE_RESULT(BinderWouldBlock, 76)
    HK_DEFINE_RESULT(BinderNoMemory, 77)
    HK_DEFINE_RESULT(BinderAlreadyExists,78)
    HK_DEFINE_RESULT(BinderNoInit, 79)
    HK_DEFINE_RESULT(BinderBadValue, 80)
    HK_DEFINE_RESULT(BinderDeadObject, 81)
    HK_DEFINE_RESULT(BinderInvalidOperation, 82)
    HK_DEFINE_RESULT(BinderNotEnoughData, 83)
    HK_DEFINE_RESULT(BinderUnknownTransaction, 84)
    HK_DEFINE_RESULT(BinderBadIndex, 85)
    HK_DEFINE_RESULT(BinderTimedOut, 86)
    HK_DEFINE_RESULT(BinderFdsNotAllowed, 87)
    HK_DEFINE_RESULT(BinderFailedTransaction, 88)
    HK_DEFINE_RESULT(BinderBadType, 89)
    HK_DEFINE_RESULT(BinderUnknownError, 90)

} // namespace hk::vi
