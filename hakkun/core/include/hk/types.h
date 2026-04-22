#pragma once

#include "hk/prim/traits/Type.h"
#if !__GNUC__
#error ""
#endif

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>

#define INCLUDE_HK_DETAIL_PLATFORM
#include "hk/detail/platform.h"

#define INCLUDE_HK_DETAIL_MACROS
#include "hk/detail/macros.h"

#include "hk/prim/traits/Integer.h"

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

using std::forward;
using std::move;

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

namespace hk {

    constexpr bool is32Bit() {
        return
#ifdef __aarch64__
            false
#else
            true
#endif
            ;
    }

    constexpr bool is64Bit() { return !is32Bit(); }

    struct Handle {
        u32 value = 0;

        constexpr Handle() = default;
        constexpr Handle(u32 value)
            : value(value) { }

        constexpr operator u32() const { return value; }
    };

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

/**
 * @brief Defer execution of a lambda until the end of the current scope.
 */
#define defer auto CONCAT(CONCAT(scope_exit_guard_, __LINE__), __COUNTER__) = ::hk::ScopeGuardOnExit(true) + [&]()

/**
 * @brief Defer execution of a lambda until the end of the current scope, if COND is true.
 */
#define defer_if(COND) auto CONCAT(CONCAT(scope_exit_guard_, __LINE__), __COUNTER__) = ::hk::ScopeGuardOnExit(COND) + [&]()

    template <typename T, typename Base>
    concept Derived = util::ctIsBaseOf<Base, T>;

    template <typename T>
    concept ReferenceType = util::ctIsReference<T>;

    template <typename T>
    concept PointerType = util::ctIsPointer<T>;

} // namespace hk

#include "hk/Result.h"
