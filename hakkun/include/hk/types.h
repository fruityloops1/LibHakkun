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

    constexpr size sPageSize = 4_KB;

    template <typename T>
    constexpr T alignUp(T from, size alignment) { return T((ptr(from) + (alignment - 1)) & ~(alignment - 1)); }
    template <typename T>
    constexpr T alignDown(T from, size alignment) { return T(ptr(from) & ~(alignment - 1)); }

    template <typename T>
    constexpr T alignUpPage(T from) { return alignUp(from, sPageSize); }
    template <typename T>
    constexpr T alignDownPage(T from) { return alignDown(from, sPageSize); }

} // namespace hk

#include "hk/Result.h"
