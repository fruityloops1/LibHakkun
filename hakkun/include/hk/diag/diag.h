#pragma once

#include "hk/diag/results.h" // IWYU pragma: keep
#include "hk/svc/types.h"
#include <cstdarg>

namespace hk::diag {

    Result setCurrentThreadName(const char* name);
    ValueOrResult<const char*> getCurrentThreadName();

    void dumpStackTrace();

#if !defined(HK_RELEASE) or defined(HK_RELEASE_DEBINFO)
    const char* getResultName(hk::Result result);
#else
    hk_alwaysinline inline const char* getResultName(hk::Result result) { return nullptr; }
#endif

    hk_noreturn void abortImpl(svc::BreakReason reason, Result result, const char* file, int line, const char* msgFmt, ...);
    hk_noreturn void abortImpl(svc::BreakReason reason, Result result, const char* file, int line, const char* msgFmt, std::va_list arg);

    constexpr char cAssertionFailFormat[] =
        R"(
AssertionFailed: %s
)";
    constexpr char cAbortUnlessResultFormat[] =
        R"(
ResultAbort (%04d-%04d/0x%x) [from %s]
)";
    constexpr char cAbortUnlessResultFormatWithName[] =
        R"(
ResultAbort (%04d-%04d/%s) [from %s]
)";
    constexpr char cUnwrapResultFormat[] =
        R"(
Unwrap (%04d-%04d/0x%x) [from %s]
)";
    constexpr char cUnwrapResultFormatWithName[] =
        R"(
Unwrap (%04d-%04d/%s) [from %s]
)";
    constexpr char cNullptrUnwrapFormat[] =
        R"(
Unwrap (nullptr) [from %s]
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

#define HK_TODO(...) __builtin_trap();

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

#define HK_ABORT(FMT, ...)                             \
    do {                                               \
        ::hk::diag::abortImpl(                         \
            ::hk::svc::BreakReason_User,               \
            ::hk::diag::ResultAbort(),                 \
            __FILE__,                                  \
            __LINE__,                                  \
            "\n" FMT "\n" __VA_OPT__(, ) __VA_ARGS__); \
    } while (0)

#define HK_ABORT_UNLESS(CONDITION, FMT, ...)               \
    do {                                                   \
        const bool _condition_temp = (CONDITION);          \
        const char* _fmt = FMT;                            \
        if (_condition_temp == false) {                    \
            ::hk::diag::abortImpl(                         \
                ::hk::svc::BreakReason_User,               \
                ::hk::diag::ResultAbort(),                 \
                __FILE__,                                  \
                __LINE__,                                  \
                "\n" FMT "\n" __VA_OPT__(, ) __VA_ARGS__); \
        }                                                  \
    } while (0)

#define HK_ABORT_UNLESS_R(RESULT)                                                    \
    do {                                                                             \
        const ::hk::Result _result_temp = RESULT;                                    \
        if (_result_temp.failed()) {                                                 \
            const char* _result_temp_name = ::hk::diag::getResultName(_result_temp); \
            if (_result_temp_name != nullptr) {                                      \
                ::hk::diag::abortImpl(                                               \
                    ::hk::svc::BreakReason_User,                                     \
                    _result_temp,                                                    \
                    __FILE__,                                                        \
                    __LINE__,                                                        \
                    ::hk::diag::cAbortUnlessResultFormatWithName,                    \
                    _result_temp.getModule() + 2000,                                 \
                    _result_temp.getDescription(),                                   \
                    _result_temp_name,                                               \
                    #RESULT);                                                        \
            } else {                                                                 \
                ::hk::diag::abortImpl(                                               \
                    ::hk::svc::BreakReason_User,                                     \
                    _result_temp,                                                    \
                    __FILE__,                                                        \
                    __LINE__,                                                        \
                    ::hk::diag::cAbortUnlessResultFormat,                            \
                    _result_temp.getModule() + 2000,                                 \
                    _result_temp.getDescription(),                                   \
                    _result_temp.getValue(),                                         \
                    #RESULT);                                                        \
            }                                                                        \
        }                                                                            \
    } while (0)

#define HK_TODO(...) \
    HK_ABORT("todo" __VA_OPT__(": ", ) __VA_ARGS__)

#endif

    /**
     * @brief Weak sink function that will be logged to from hk::diag::log
     *
     */
    extern "C" void hkLogSink(const char* msg, size len);

#if defined(HK_RELEASE) and not defined(HK_RELEASE_DEBINFO)
    inline void logBuffer(const char* buf, size len) { }
    inline void logImpl(const char* fmt, std::va_list list) { }
    inline void log(const char* fmt, ...) { }
    inline void logLineImpl(const char* fmt, std::va_list list) { }
    inline void logLine(const char* fmt, ...) { }
    [[deprecated("use hk::debug::logLine instead")]] inline void debugLog(const char* fmt, ...) { }
#else
    void logBuffer(const char* buf, size len);
    void logImpl(const char* fmt, std::va_list list);
    void log(const char* fmt, ...);
    void logLineImpl(const char* fmt, std::va_list list);
    void logLine(const char* fmt, ...);
    [[deprecated("use hk::debug::logLine instead")]] void debugLog(const char* fmt, ...);
#endif

} // namespace hk::diag
