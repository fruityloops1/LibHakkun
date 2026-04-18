#pragma once

#include "hk/diag/results.h" // IWYU pragma: keep
#include "hk/util/TemplateString.h"
#include <cstdarg>

#if NNSDK
#include "hk/svc/types.h"
#endif

namespace hk::diag {

#if NNSDK
    Result setCurrentThreadName(const char* name);
    ValueOrResult<const char*> getCurrentThreadName();

    void dumpStackTrace();
#endif

#if !defined(HK_RELEASE) or defined(HK_RELEASE_DEBINFO)
    const char* getResultName(hk::Result result);

    [[gnu::format(printf, HAS_NNSDK_TERNARY(5, 4), HAS_NNSDK_TERNARY(6, 5))]] hk_noreturn void abortImpl(HAS_NNSDK(svc::BreakReason reason, ) Result result, const char* file, int line, const char* msgFmt, ...);
    [[gnu::format(printf, HAS_NNSDK_TERNARY(5, 4), 0)]] hk_noreturn void abortImpl(HAS_NNSDK(svc::BreakReason reason, ) Result result, const char* file, int line, const char* msgFmt, std::va_list arg);
#else
    const hk_alwaysinline inline char* getResultName(hk::Result result) { return nullptr; }

    template <util::TemplateString File, int Line>
    hk_noreturn hk_noinline void abortReleaseImpl(Result result, ...) { __builtin_trap(); }
#endif

    constexpr char cAssertionFailFormat[] = "AssertionFailed: %s";
    constexpr char cAbortUnlessResultFormat[] = "ResultAbort (%04d-%04d/0x%x) [from %s]";
    constexpr char cAbortUnlessResultFormatWithName[] = "ResultAbort (%04d-%04d/%s) [from %s]";
    constexpr char cUnwrapResultFormat[] = "Unwrap (%04d-%04d/0x%x) [from %s]";
    constexpr char cUnwrapResultFormatWithName[] = "Unwrap (%04d-%04d/%s) [from %s]";
    constexpr char cNullptrUnwrapFormat[] = "Unwrap (nullptr) [from %s]";

#if defined(HK_RELEASE) and not defined(HK_RELEASE_DEBINFO)

#define HK_ASSERT(CONDITION, ...)                                                                   \
    do {                                                                                            \
        const bool _condition_temp = (CONDITION __VA_OPT__(, ) __VA_ARGS__);                        \
        if (_condition_temp == false) {                                                             \
            if (__builtin_is_constant_evaluated())                                                  \
                ::hk::diag::detail::abortConstexpr("AssertionFailed: " #CONDITION);                 \
            ::hk::diag::abortReleaseImpl<__FILE__, __LINE__>(::hk::diag::ResultAssertionFailure()); \
        }                                                                                           \
    } while (0)

#define HK_ABORT(FMT, ...)                                                                                      \
    do {                                                                                                        \
        if (__builtin_is_constant_evaluated())                                                                  \
            ::hk::diag::detail::abortConstexpr("ResultAbort: " FMT __VA_OPT__(, ) __VA_ARGS__);                 \
        ::hk::diag::abortReleaseImpl<__FILE__, __LINE__>(::hk::diag::ResultAbort() __VA_OPT__(, ) __VA_ARGS__); \
    } while (0)

#define HK_ABORT_UNLESS(CONDITION, FMT, ...)                                                                        \
    do {                                                                                                            \
        const bool _condition_temp = (CONDITION);                                                                   \
        if (_condition_temp == false) {                                                                             \
            if (__builtin_is_constant_evaluated())                                                                  \
                ::hk::diag::detail::abortConstexpr("ResultAbort: " FMT __VA_OPT__(, ) __VA_ARGS__);                 \
            ::hk::diag::abortReleaseImpl<__FILE__, __LINE__>(::hk::diag::ResultAbort() __VA_OPT__(, ) __VA_ARGS__); \
        }                                                                                                           \
    } while (0)

#define HK_ABORT_UNLESS_R(RESULT, ...)                                       \
    do {                                                                     \
        const ::hk::Result _result_temp = RESULT __VA_OPT__(, ) __VA_ARGS__; \
        if (_result_temp.failed()) {                                         \
            if (__builtin_is_constant_evaluated())                           \
                ::hk::diag::detail::abortConstexpr(                          \
                    "ResultAbort",                                           \
                    _result_temp.getModule() + 2000,                         \
                    _result_temp.getDescription(),                           \
                    _result_temp.getValue());                                \
            ::hk::diag::abortReleaseImpl<__FILE__, __LINE__>(_result_temp);  \
        }                                                                    \
    } while (0)

#define HK_TODO(...) HK_ABORT("todo" __VA_OPT__(": ", ) __VA_ARGS__)

#else
    // clang-format off
#define HK_ASSERT(CONDITION, ...)                                                         \
    do {                                                                                  \
        const bool _condition_temp = (CONDITION __VA_OPT__(, ) __VA_ARGS__);              \
        if (_condition_temp == false) {                                                   \
            if (__builtin_is_constant_evaluated())                                        \
                ::hk::diag::detail::abortConstexpr("AssertionFailed: " #CONDITION);       \
            ::hk::diag::abortImpl(                                                        \
                HAS_NNSDK(::hk::svc::BreakReason_Assert,)                                 \
                ::hk::diag::ResultAssertionFailure(),                                     \
                __FILE__,                                                                 \
                __LINE__,                                                                 \
                ::hk::diag::cAssertionFailFormat,                                         \
                #CONDITION);                                                              \
        }                                                                                 \
    } while (0)

#define HK_ABORT(FMT, ...)                                                      \
    do {                                                                        \
        if (__builtin_is_constant_evaluated())                                  \
            ::hk::diag::detail::abortConstexpr(FMT __VA_OPT__(, ) __VA_ARGS__); \
        ::hk::diag::abortImpl(                                                  \
            HAS_NNSDK(::hk::svc::BreakReason_User,)                             \
            ::hk::diag::ResultAbort(),                                          \
            __FILE__,                                                           \
            __LINE__,                                                           \
            "\n" FMT "\n" __VA_OPT__(, ) __VA_ARGS__);                          \
    } while (0)

#define HK_ABORT_UNLESS(CONDITION, FMT, ...)                                        \
    do {                                                                            \
        const bool _condition_temp = (CONDITION);                                   \
        const char* _fmt = FMT;                                                     \
        if (_condition_temp == false) {                                             \
            if (__builtin_is_constant_evaluated())                                  \
                ::hk::diag::detail::abortConstexpr(FMT __VA_OPT__(, ) __VA_ARGS__); \
            ::hk::diag::abortImpl(                                                  \
                HAS_NNSDK(::hk::svc::BreakReason_User,)                             \
                ::hk::diag::ResultAbort(),                                          \
                __FILE__,                                                           \
                __LINE__,                                                           \
                "\n" FMT "\n" __VA_OPT__(, ) __VA_ARGS__);                          \
        }                                                                           \
    } while (0)

#define HK_ABORT_UNLESS_R(RESULT, ...)                                               \
    do {                                                                             \
        const ::hk::Result _result_temp = RESULT __VA_OPT__(, ) __VA_ARGS__;         \
        if (_result_temp.failed()) {                                                 \
            if (__builtin_is_constant_evaluated())                                   \
                ::hk::diag::detail::abortConstexpr(                                  \
                    "ResultAbort",                                                   \
                    _result_temp.getModule() + 2000,                                 \
                    _result_temp.getDescription(),                                   \
                    _result_temp.getValue());                                        \
            const char* _result_temp_name = ::hk::diag::getResultName(_result_temp); \
            if (_result_temp_name != nullptr) {                                      \
                ::hk::diag::abortImpl(                                               \
                    HAS_NNSDK(::hk::svc::BreakReason_User,)                          \
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
                    HAS_NNSDK(::hk::svc::BreakReason_User,)                          \
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
    HK_ABORT("TODO: " __VA_ARGS__)
    // clang-format on
#endif

    /**
     * @brief Weak sink function that will be logged to from hk::diag::log
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
    [[gnu::format(printf, 1, 0)]] void logImpl(const char* fmt, std::va_list list);
    [[gnu::format(printf, 1, 2)]] void log(const char* fmt, ...);
    [[gnu::format(printf, 1, 0)]] void logLineImpl(const char* fmt, std::va_list list);
    [[gnu::format(printf, 1, 2)]] void logLine(const char* fmt, ...);
    [[deprecated("use hk::debug::logLine instead")]] [[gnu::format(printf, 1, 2)]] void debugLog(const char* fmt, ...);
#endif

    namespace detail {

        template <size N, typename... Args>
        constexpr hk_noreturn void abortConstexpr(const char (&str)[N], Args...) {
            __builtin_unreachable();
        }

    } // namespace detail

} // namespace hk::diag
