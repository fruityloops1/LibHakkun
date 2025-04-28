#pragma once

#include "hk/diag/results.h"
#include "hk/svc/types.h"
#include <cstdarg>

namespace hk::diag {

    Result setCurrentThreadName(const char* name);

    hk_noreturn void abortImpl(svc::BreakReason reason, Result result, const char* file, int line, const char* msgFmt, ...);
    hk_noreturn void abortImpl(svc::BreakReason reason, Result result, const char* file, int line, const char* msgFmt, std::va_list arg);

    constexpr char cAssertionFailFormat[] =
        R"(
AssertionFailed: %s
)";
    constexpr char cAbortUnlessResultFormat[] =
        R"(
ResultAbort (%04d-%04d) [from %s]
)";

#if defined(HK_RELEASE) and not defined(HK_RELEASE_DEBINFO)

#define HK_ASSERT(CONDITION)                      \
    do {                                          \
        const bool _condition_temp = (CONDITION); \
        if (_condition_temp == false) {           \
            __builtin_trap();                     \
        }                                         \
    } while (0)

#define HK_ABORT(FMT, ...) \
    do {                   \
        __builtin_trap();  \
    } while (0)

#define HK_ABORT_UNLESS(CONDITION, FMT, ...)      \
    do {                                          \
        const bool _condition_temp = (CONDITION); \
        if (_condition_temp == false) {           \
            __builtin_trap();                     \
        }                                         \
    } while (0)

#define HK_ABORT_UNLESS_R(RESULT)                 \
    do {                                          \
        const ::hk::Result _result_temp = RESULT; \
        if (_result_temp.failed()) {              \
            __builtin_trap();                     \
        }                                         \
    } while (0)

#else

#define HK_ASSERT(CONDITION)                          \
    do {                                              \
        const bool _condition_temp = (CONDITION);     \
        if (_condition_temp == false) {               \
            ::hk::diag::abortImpl(                    \
                ::hk::svc::BreakReason_Assert,        \
                ::hk::diag::ResultAssertionFailure(), \
                __FILE__,                             \
                __LINE__,                             \
                ::hk::diag::cAssertionFailFormat,     \
                #CONDITION);                          \
        }                                             \
    } while (0)

#define HK_ABORT(FMT, ...)               \
    do {                                 \
        ::hk::diag::abortImpl(           \
            ::hk::svc::BreakReason_User, \
            ::hk::diag::ResultAbort(),   \
            __FILE__,                    \
            __LINE__,                    \
            "\n" FMT "\n",               \
            __VA_ARGS__);                \
    } while (0)

#define HK_ABORT_UNLESS(CONDITION, FMT, ...)      \
    do {                                          \
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
    } while (0)

#define HK_ABORT_UNLESS_R(RESULT)                     \
    do {                                              \
        const ::hk::Result _result_temp = RESULT;     \
        if (_result_temp.failed()) {                  \
            ::hk::diag::abortImpl(                    \
                ::hk::svc::BreakReason_User,          \
                _result_temp,                         \
                __FILE__,                             \
                __LINE__,                             \
                ::hk::diag::cAbortUnlessResultFormat, \
                _result_temp.getModule() + 2000,      \
                _result_temp.getDescription(),        \
                #RESULT);                             \
        }                                             \
    } while (0)

#endif

#if defined(HK_RELEASE) and not defined(HK_RELEASE_DEBINFO)
    inline void debugLog(const char* fmt, ...) { }
#else
    void debugLog(const char* fmt, ...);
#endif

} // namespace hk::diag
