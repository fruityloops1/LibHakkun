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

    [[gnu::format(printf, HAS_NNSDK_TERNARY(6, 5), HAS_NNSDK_TERNARY(7, 6))]] hk_noreturn void abortImpl(HAS_NNSDK(svc::BreakReason reason, ) Result result, const char* file, u32 line, u16 column, const char* msgFmt, ...);
    [[gnu::format(printf, HAS_NNSDK_TERNARY(6, 5), 0)]] hk_noreturn void abortImpl(HAS_NNSDK(svc::BreakReason reason, ) Result result, const char* file, u32 line, u16 column, const char* msgFmt, std::va_list arg);

#if !defined(HK_RELEASE) or defined(HK_RELEASE_DEBINFO)
    const char* getResultName(ResultBase result);
#else
    const hk_alwaysinline inline char* getResultName(ResultBase result) { return nullptr; }

    template <util::TemplateString File, u32 Line, u16 Col>
    hk_noreturn hk_noinline void abortReleaseImpl(Result result, ...) { __builtin_trap(); }
#endif

#if HK_RESULT_ADVANCED and (!defined(HK_RELEASE) or defined(HK_RELEASE_DEBINFO))
    void dumpResultTrace(Result result);
#else
    inline hk_alwaysinline void dumpResultTrace(Result result) { }
#endif

#if !defined(HK_RELEASE) or defined(HK_RELEASE_DEBINFO)
    void dumpImpl(Result result, const char* expr, const char* file, u32 line, u16 column);
#else
    inline void dumpImpl(Result result, const char* expr, const char* file, u32 line, u16 column) { }
#endif

    extern const char cAssertionFailFormat[];
    extern const char cAbortUnlessResultFormat[];
    extern const char cAbortUnlessResultFormatWithName[];
    extern const char cUnwrapResultFormat[];
    extern const char cUnwrapResultFormatWithName[];
    extern const char cNullptrUnwrapFormat[];

#if defined(HK_RELEASE) and not defined(HK_RELEASE_DEBINFO)

#define _HK_ASSERT_IMPL(COL_SUB, CONDITION, ...)                                                                  \
    do {                                                                                                          \
        const bool _condition_temp = (CONDITION __VA_OPT__(, ) __VA_ARGS__);                                      \
        if (_condition_temp == false) {                                                                           \
            if (__builtin_is_constant_evaluated())                                                                \
                ::hk::diag::detail::abortConstexpr("AssertionFailed: " #CONDITION __VA_OPT__(", ") #__VA_ARGS__); \
            constexpr static u16 _column = ::hk::diag::SourceLocation::current().column() - COL_SUB;              \
            ::hk::diag::abortReleaseImpl<__FILE__, __LINE__, _column>(::hk::diag::ResultAssertionFailure());      \
        }                                                                                                         \
    } while (0)

#define HK_ASSERT_WITH_LOCATION(LOC, CONDITION, ...) _HK_ASSERT_IMPL(__builtin_strlen(#LOC ", " #CONDITION __VA_OPT__(",") #__VA_ARGS__), CONDITION, __VA_ARGS__)
#define HK_ASSERT(CONDITION, ...) _HK_ASSERT_IMPL(__builtin_strlen(#CONDITION __VA_OPT__(",") #__VA_ARGS__), CONDITION __VA_OPT__(, ) __VA_ARGS__)

#define _HK_ABORT_IMPL(COL_SUB, FMT, ...)                                                                                \
    do {                                                                                                                 \
        if (__builtin_is_constant_evaluated())                                                                           \
            ::hk::diag::detail::abortConstexpr(FMT __VA_OPT__(, ) __VA_ARGS__);                                          \
        constexpr static u16 _column = ::hk::diag::SourceLocation::current().column() - COL_SUB;                         \
        ::hk::diag::abortReleaseImpl<__FILE__, __LINE__, _column>(::hk::diag::ResultAbort() __VA_OPT__(, ) __VA_ARGS__); \
    } while (0)

#define HK_ABORT_WITH_LOCATION(LOC, FMT, ...) _HK_ABORT_IMPL(__builtin_strlen(#LOC ", " #FMT __VA_OPT__(", ") #__VA_ARGS__), FMT __VA_OPT__(, ) __VA_ARGS__)
#define HK_ABORT(FMT, ...) _HK_ABORT_IMPL(__builtin_strlen(#FMT __VA_OPT__(", ") #__VA_ARGS__), FMT __VA_OPT__(, ) __VA_ARGS__)

#define _HK_ABORT_UNLESS_IMPL(COL_SUB, CONDITION, FMT, ...)                                                                  \
    do {                                                                                                                     \
        const bool _condition_temp = (CONDITION);                                                                            \
        if (_condition_temp == false) {                                                                                      \
            if (__builtin_is_constant_evaluated())                                                                           \
                ::hk::diag::detail::abortConstexpr(FMT __VA_OPT__(, ) __VA_ARGS__);                                          \
            constexpr static u16 _column = ::hk::diag::SourceLocation::current().column() - COL_SUB;                         \
            ::hk::diag::abortReleaseImpl<__FILE__, __LINE__, _column>(::hk::diag::ResultAbort() __VA_OPT__(, ) __VA_ARGS__); \
        }                                                                                                                    \
    } while (0)

#define HK_ABORT_UNLESS_WITH_LOCATION(LOC, CONDITION, FMT, ...) _HK_ABORT_UNLESS_IMPL(__builtin_strlen(#LOC ", " #CONDITION ", " #FMT __VA_OPT__(", ") #__VA_ARGS__), CONDITION, FMT __VA_OPT__(, ) __VA_ARGS__)
#define HK_ABORT_UNLESS(CONDITION, FMT, ...) _HK_ABORT_UNLESS_IMPL(__builtin_strlen(#CONDITION ", " #FMT __VA_OPT__(", ") #__VA_ARGS__), CONDITION, FMT __VA_OPT__(, ) __VA_ARGS__)

#define _HK_ABORT_UNLESS_R_IMPL(COL_SUB, RESULT, ...)                                                \
    do {                                                                                             \
        const ::hk::Result _result_temp = RESULT __VA_OPT__(, ) __VA_ARGS__;                         \
        if (_result_temp.failed()) {                                                                 \
            if (__builtin_is_constant_evaluated())                                                   \
                ::hk::diag::detail::abortConstexpr(                                                  \
                    "ResultAbort",                                                                   \
                    _result_temp.getModule() + 2000,                                                 \
                    _result_temp.getDescription(),                                                   \
                    _result_temp.getValue());                                                        \
            constexpr static u16 _column = ::hk::diag::SourceLocation::current().column() - COL_SUB; \
            ::hk::diag::abortReleaseImpl<__FILE__, __LINE__, _column>(_result_temp);                 \
        }                                                                                            \
    } while (0)

#define HK_ABORT_UNLESS_R_WITH_LOCATION(LOC, RESULT, ...) _HK_ABORT_UNLESS_R_IMPL(__builtin_strlen(#LOC ", " #RESULT __VA_OPT__(", ") #__VA_ARGS__), RESULT __VA_OPT__(, ) __VA_ARGS__)
#define HK_ABORT_UNLESS_R(RESULT, ...) _HK_ABORT_UNLESS_R_IMPL(__builtin_strlen(#RESULT __VA_OPT__(", ") #__VA_ARGS__), RESULT __VA_OPT__(, ) __VA_ARGS__)

#define HK_DUMP(RESULT, ...) (RESULT __VA_OPT__(, ) __VA_ARGS__)

#define HK_TODO(...) HK_ABORT("TODO" __VA_OPT__(": ") __VA_ARGS__)

#else
    // clang-format off
#define _HK_ASSERT_IMPL(LOC, COL_SUB, CONDITION, ...)                                                             \
    do {                                                                                                          \
        const bool _condition_temp = (CONDITION __VA_OPT__(, ) __VA_ARGS__);                                      \
        if (_condition_temp == false) {                                                                           \
            if (__builtin_is_constant_evaluated())                                                                \
                ::hk::diag::detail::abortConstexpr("AssertionFailed: " #CONDITION __VA_OPT__(", ") #__VA_ARGS__); \
            ::hk::diag::SourceLocation _hk_sourceLoc = LOC;                                                       \
            ::hk::diag::abortImpl(                                                                                \
                HAS_NNSDK(::hk::svc::BreakReason_Assert,)                                                         \
                MAKE_RESULT(::hk::diag::ResultAssertionFailure()),                                                \
                _hk_sourceLoc.file(),                                                                             \
                _hk_sourceLoc.line(),                                                                             \
                _hk_sourceLoc.column() - COL_SUB,                                                                 \
                ::hk::diag::cAssertionFailFormat,                                                                 \
                #CONDITION __VA_OPT__(", ") #__VA_ARGS__);                                                        \
        }                                                                                                         \
    } while (0)

#define HK_ASSERT_WITH_LOCATION(LOC, CONDITION, ...) _HK_ASSERT_IMPL(LOC, __builtin_strlen(#LOC ", " #CONDITION __VA_OPT__(", ") #__VA_ARGS__), CONDITION __VA_OPT__(,) __VA_ARGS__)
#define HK_ASSERT(CONDITION, ...) _HK_ASSERT_IMPL(::hk::diag::SourceLocation::current(), __builtin_strlen(#CONDITION __VA_OPT__(", ") #__VA_ARGS__), CONDITION __VA_OPT__(,) __VA_ARGS__)

#define _HK_ABORT_IMPL(LOC, COL_SUB, FMT, ...)                                 \
    do {                                                                       \
        if (__builtin_is_constant_evaluated())                                 \
            ::hk::diag::detail::abortConstexpr(FMT __VA_OPT__(,) __VA_ARGS__); \
        ::hk::diag::SourceLocation _hk_sourceLoc = LOC;                        \
        ::hk::diag::abortImpl(                                                 \
            HAS_NNSDK(::hk::svc::BreakReason_User,)                            \
            MAKE_RESULT(::hk::diag::ResultAbort()),                            \
            _hk_sourceLoc.file(),                                              \
            _hk_sourceLoc.line(),                                              \
            _hk_sourceLoc.column() - COL_SUB,                                  \
            FMT __VA_OPT__(,) __VA_ARGS__);                                    \
    } while (0)

#define HK_ABORT_WITH_LOCATION(LOC, FMT, ...) _HK_ABORT_IMPL(LOC, __builtin_strlen(#FMT __VA_OPT__(", ") #__VA_ARGS__), FMT __VA_OPT__(,) __VA_ARGS__)
#define HK_ABORT(FMT, ...) _HK_ABORT_IMPL(::hk::diag::SourceLocation::current(), __builtin_strlen(#FMT __VA_OPT__(", ") #__VA_ARGS__), FMT __VA_OPT__(,) __VA_ARGS__)

#define _HK_ABORT_UNLESS_IMPL(LOC, CONDITION, COL_SUB, FMT, ...)                   \
    do {                                                                           \
        const bool _condition_temp = (CONDITION);                                  \
        const char* _fmt = FMT;                                                    \
        if (_condition_temp == false) {                                            \
            if (__builtin_is_constant_evaluated())                                 \
                ::hk::diag::detail::abortConstexpr(FMT __VA_OPT__(,) __VA_ARGS__); \
            ::hk::diag::SourceLocation _hk_sourceLoc = LOC;                        \
            ::hk::diag::abortImpl(                                                 \
                HAS_NNSDK(::hk::svc::BreakReason_User,)                            \
                MAKE_RESULT(::hk::diag::ResultAbort()),                            \
                _hk_sourceLoc.file(),                                              \
                _hk_sourceLoc.line(),                                              \
                _hk_sourceLoc.column() - COL_SUB,                                  \
                FMT __VA_OPT__(,) __VA_ARGS__);                                    \
        }                                                                          \
    } while (0)

#define HK_ABORT_UNLESS_WITH_LOCATION(LOC, CONDITION, FMT, ...) _HK_ABORT_UNLESS_IMPL(LOC, CONDITION, __builtin_strlen(#CONDITION ", " #FMT __VA_OPT__(", ") #__VA_ARGS__), FMT __VA_OPT__(,) __VA_ARGS__)
#define HK_ABORT_UNLESS(CONDITION, FMT, ...) _HK_ABORT_UNLESS_IMPL(::hk::diag::SourceLocation::current(), CONDITION, __builtin_strlen(#CONDITION ", " #FMT __VA_OPT__(", ") #__VA_ARGS__), FMT __VA_OPT__(,) __VA_ARGS__)

#define _HK_ABORT_UNLESS_R_IMPL(LOC, EXPR, COL_SUB, RESULT, ...)                          \
    do {                                                                                  \
        const ::hk::Result _result_temp = MAKE_RESULT(RESULT __VA_OPT__(, ) __VA_ARGS__); \
        if (_result_temp.failed()) {                                                      \
            if (__builtin_is_constant_evaluated())                                        \
                ::hk::diag::detail::abortConstexpr(                                       \
                    "ResultAbort",                                                        \
                    _result_temp.getModule() + 2000,                                      \
                    _result_temp.getDescription(),                                        \
                    _result_temp.getValue());                                             \
            const char* _result_temp_name = ::hk::diag::getResultName(_result_temp);      \
            ::hk::diag::SourceLocation _hk_sourceLoc = LOC;                               \
            if (_result_temp_name != nullptr) {                                           \
                ::hk::diag::abortImpl(                                                    \
                    HAS_NNSDK(::hk::svc::BreakReason_User,)                               \
                    _result_temp,                                                         \
                    _hk_sourceLoc.file(),                                                 \
                    _hk_sourceLoc.line(),                                                 \
                    _hk_sourceLoc.column() - COL_SUB,                                     \
                    ::hk::diag::cAbortUnlessResultFormatWithName,                         \
                    _result_temp.getModule() + 2000,                                      \
                    _result_temp.getDescription(),                                        \
                    _result_temp_name,                                                    \
                    EXPR);                                                                \
            } else {                                                                      \
                ::hk::diag::abortImpl(                                                    \
                    HAS_NNSDK(::hk::svc::BreakReason_User,)                               \
                    _result_temp,                                                         \
                    _hk_sourceLoc.file(),                                                 \
                    _hk_sourceLoc.line(),                                                 \
                    _hk_sourceLoc.column() - COL_SUB,                                     \
                    ::hk::diag::cAbortUnlessResultFormat,                                 \
                    _result_temp.getModule() + 2000,                                      \
                    _result_temp.getDescription(),                                        \
                    _result_temp.getValue(),                                              \
                    EXPR);                                                                \
            }                                                                             \
        }                                                                                 \
    } while (0)

#define HK_ABORT_UNLESS_R_WITH_LOCATION(LOC, RESULT, ...) _HK_ABORT_UNLESS_R_IMPL(LOC, #RESULT __VA_OPT__(", ") #__VA_ARGS__, __builtin_strlen(#RESULT __VA_OPT__(", ") #__VA_ARGS__), RESULT __VA_OPT__(,) __VA_ARGS__)
#define HK_ABORT_UNLESS_R(RESULT, ...) _HK_ABORT_UNLESS_R_IMPL(::hk::diag::SourceLocation::current(), #RESULT __VA_OPT__(", ") #__VA_ARGS__, __builtin_strlen(#RESULT __VA_OPT__(", ") #__VA_ARGS__), RESULT __VA_OPT__(,) __VA_ARGS__)

#define _HK_DUMP_IMPL(LOC, EXPR, COL_SUB, RESULT, ...)                                                                        \
    ({                                                                                                                        \
        auto&& _value_temp = RESULT __VA_OPT__(, ) __VA_ARGS__;                                                               \
        ::hk::diag::SourceLocation _hk_sourceLoc = LOC;                                                                       \
        const u16 _column = _hk_sourceLoc.column() - COL_SUB;                                                                 \
        ::hk::Result _result_temp = MAKE_RESULT_IMPL(_value_temp, EXPR, _hk_sourceLoc.file(), _hk_sourceLoc.line(), _column); \
        if (_result_temp.failed())                                                                                            \
            ::hk::diag::dumpImpl(_result_temp, EXPR, _hk_sourceLoc.file(), _hk_sourceLoc.line(), _column);                    \
        ::move(_value_temp);                                                                                                  \
    })

#define HK_DUMP_WITH_LOCATION(LOC, RESULT, ...) _HK_DUMP_IMPL(LOC, #RESULT __VA_OPT__(", ") #__VA_ARGS__, __builtin_strlen(#LOC ", " #RESULT __VA_OPT__(", ") #__VA_ARGS__), RESULT __VA_OPT__(,) __VA_ARGS__)
#define HK_DUMP(RESULT, ...) _HK_DUMP_IMPL(::hk::diag::SourceLocation::current(), #RESULT __VA_OPT__(", ") #__VA_ARGS__, __builtin_strlen(#RESULT __VA_OPT__(", ") #__VA_ARGS__), RESULT __VA_OPT__(,) __VA_ARGS__)

#define HK_TODO(...) \
    HK_ABORT("TODO" __VA_OPT__(": ") __VA_ARGS__)
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
