#pragma once

#include "hk/diag/results.h"
#include "hk/svc/types.h"

namespace hk::diag {

    Result setCurrentThreadName(const char* name);

    hk_noreturn void abortImpl(svc::BreakReason reason, Result result, const char* file, int line, const char* msgFmt, ...);

    constexpr char sAssertionFailFormat[] =
        R"(
AssertionFailed: %s
)";
    constexpr char sAbortUnlessResultFormat[] =
        R"(
ResultAbort (%04d-%04d) [from %s]
)";

#ifdef HK_RELEASE

#define HK_ASSERT(CONDITION)                      \
    {                                             \
        const bool _condition_temp = (CONDITION); \
        if (_condition_temp == false) {           \
        }                                         \
    }

#define HK_ABORT(FMT, ...)               \
    {                                    \
        __builtin_trap();                \
        ::hk::diag::abortImpl(           \
            ::hk::svc::BreakReason_User, \
            ::hk::diag::ResultAbort(),   \
            nullptr,                     \
            0,                           \
            nullptr,                     \
            0);                          \
    }

#define HK_ABORT_UNLESS(CONDITION, FMT, ...)      \
    {                                             \
        const bool _condition_temp = (CONDITION); \
        const char* _fmt = FMT;                   \
        if (_condition_temp == false) {           \
            __builtin_trap();                     \
            ::hk::diag::abortImpl(                \
                ::hk::svc::BreakReason_User,      \
                ::hk::diag::ResultAbort(),        \
                nullptr,                          \
                0,                                \
                nullptr,                          \
                0);                               \
        }                                         \
    }

#define HK_ABORT_UNLESS_R(RESULT)                 \
    {                                             \
        const ::hk::Result _result_temp = RESULT; \
        if (_result_temp.failed()) {              \
            __builtin_trap();                     \
            ::hk::diag::abortImpl(                \
                ::hk::svc::BreakReason_User,      \
                _result_temp,                     \
                nullptr,                          \
                0,                                \
                nullptr,                          \
                0);                               \
        }                                         \
    }

#else

#define HK_ASSERT(CONDITION)                          \
    {                                                 \
        const bool _condition_temp = (CONDITION);     \
        if (_condition_temp == false) {               \
            ::hk::diag::abortImpl(                    \
                ::hk::svc::BreakReason_Assert,        \
                ::hk::diag::ResultAssertionFailure(), \
                __FILE__,                             \
                __LINE__,                             \
                ::hk::diag::sAssertionFailFormat,     \
                #CONDITION);                          \
        }                                             \
    }

#define HK_ABORT(FMT, ...)               \
    {                                    \
        ::hk::diag::abortImpl(           \
            ::hk::svc::BreakReason_User, \
            ::hk::diag::ResultAbort(),   \
            __FILE__,                    \
            __LINE__,                    \
            "\n" FMT "\n",               \
            __VA_ARGS__);                \
    }

#define HK_ABORT_UNLESS(CONDITION, FMT, ...)      \
    {                                             \
        const bool _condition_temp = (CONDITION); \
        const char* _fmt = FMT;                   \
        if (_condition_temp == false) {           \
            ::hk::diag::abortImpl(                \
                ::hk::svc::BreakReason_User,      \
                ::hk::diag::ResultAbort(),        \
                __FILE__,                         \
                __LINE__,                         \
                "\n" FMT "\n",                    \
                __VA_ARGS__);                     \
        }                                         \
    }

#define HK_ABORT_UNLESS_R(RESULT)                     \
    {                                                 \
        const ::hk::Result _result_temp = RESULT;     \
        if (_result_temp.failed()) {                  \
            ::hk::diag::abortImpl(                    \
                ::hk::svc::BreakReason_User,          \
                _result_temp,                         \
                __FILE__,                             \
                __LINE__,                             \
                ::hk::diag::sAbortUnlessResultFormat, \
                _result_temp.getModule() + 2000,      \
                _result_temp.getDescription(),        \
                #RESULT);                             \
        }                                             \
    }

#endif

} // namespace hk::diag
