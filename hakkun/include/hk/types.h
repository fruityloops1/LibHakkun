#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

#define hk_alwaysinline __attribute__((always_inline))
#define hk_noinline __attribute__((noinline))
#define hk_noreturn __attribute__((noreturn))
#define section(SECTION) __attribute__((section(#SECTION)))
#define STRINGIFY(X) #X
#define STR(X) STRINGIFY(X)
#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)

#define NON_COPYABLE(CLASS)       \
    CLASS(const CLASS&) = delete; \
    CLASS& operator=(const CLASS&) = delete

#define NON_MOVABLE(CLASS)   \
    CLASS(CLASS&&) = delete; \
    CLASS& operator=(CLASS&&) = delete

#ifdef HK_RELEASE
#define ralwaysinline hk_alwaysinline
#else
#define ralwaysinline
#endif

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using fu8 = uint_fast8_t;
using fu16 = uint_fast16_t;
using fu32 = uint_fast32_t;
using fu64 = uint_fast64_t;
using fs8 = int_fast8_t;
using fs16 = int_fast16_t;
using fs32 = int_fast32_t;
using fs64 = int_fast64_t;

using f32 = float;
using f64 = double;

using size = size_t;
using ptrdiff = ptrdiff_t;
using ptr = uintptr_t;

template <typename To, typename From>
hk_alwaysinline constexpr To cast(From val) {
    return reinterpret_cast<To>(val);
}

template <typename To, typename From>
hk_alwaysinline To pun(From val) {
    static_assert(sizeof(To) <= sizeof(From));

    To toValue;
    __builtin_memcpy(&toValue, &val, sizeof(To));
    return toValue;
}

constexpr size operator""_B(unsigned long long val) { return val; }
constexpr size operator""_KB(unsigned long long val) { return val * 1024; }
constexpr size operator""_MB(unsigned long long val) { return val * 1024 * 1024; }
constexpr size operator""_GB(unsigned long long val) { return val * 1024 * 1024 * 1024; }

namespace hk {

    using Handle = u32;

    constexpr size cPageSize = 4_KB;

    template <typename T>
    constexpr T alignUp(T from, size alignment) { return T((ptr(from) + (alignment - 1)) & ~(alignment - 1)); }
    template <typename T>
    constexpr T alignDown(T from, size alignment) { return T(ptr(from) & ~(alignment - 1)); }

    template <typename T>
    constexpr T alignUpPage(T from) { return alignUp(from, cPageSize); }
    template <typename T>
    constexpr T alignDownPage(T from) { return alignDown(from, cPageSize); }

    template <typename T>
    constexpr bool isAligned(T from, size alignment) { return alignDown(from, alignment) == from; }
    template <typename T>
    constexpr bool isAlignedPage(T from) { return alignDown(from, cPageSize) == from; }

    constexpr u64 bit(u8 n) { return 1ULL << n; }

    constexpr u64 bits(u8 n) {
        if (n == 64)
            return 0xFFFFFFFFFFFFFFFF;
        return bit(n) - 1;
    }

    template <typename L>
    class ScopeGuard {
        L mFunc;
        bool mExec = true;

    public:
        hk_alwaysinline explicit constexpr ScopeGuard(L func, bool exec)
            : mFunc(func)
            , mExec(exec) {
        }

        hk_alwaysinline constexpr ~ScopeGuard() {
            if (mExec)
                mFunc();
        }

        ScopeGuard(const ScopeGuard&) = delete;
        ScopeGuard& operator=(const ScopeGuard&) = delete;
    };

    class ScopeGuardOnExit {
        bool mExec = false;

    public:
        hk_alwaysinline constexpr ScopeGuardOnExit(bool condition)
            : mExec(condition) { }

        template <typename L>
        hk_alwaysinline ScopeGuard<L> constexpr operator+(L&& func) {
            return ScopeGuard(func, mExec);
        }
    };

#define defer auto CONCAT(CONCAT(scope_exit_guard_, __LINE__), __COUNTER__) = ::hk::ScopeGuardOnExit(true) + [&]()
#define defer_if(COND) auto CONCAT(CONCAT(scope_exit_guard_, __LINE__), __COUNTER__) = ::hk::ScopeGuardOnExit(COND) + [&]()

} // namespace hk

#include "hk/Result.h"
