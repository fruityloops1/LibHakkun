#pragma once

#include "hk/types.h"
#include "hk/util/Context.h"
#include "hk/util/TemplateString.h"

namespace hk::util {

    namespace detail {

        extern "C" bool __cxa_guard_acquire(bool*);
        extern "C" bool __cxa_guard_release(bool*);

    } // namespace detail

    template <TemplateString Symbol = "">
    class InitializeGuard {
        bool* mGuard = 0;

    public:
        hk_alwaysinline constexpr InitializeGuard(ptr guardPtr)
            : mGuard(reinterpret_cast<bool*>(guardPtr)) {
        }

        hk_alwaysinline constexpr InitializeGuard()
            : InitializeGuard(lookupSymbol<Symbol>()) { }

        template <typename L>
        hk_alwaysinline void constexpr operator+(L&& func) {
            if (detail::__cxa_guard_acquire(mGuard)) {
                func();
                detail::__cxa_guard_release(mGuard);
            }
        }
    };

#define initialize_guard(SYM) ::hk::util::InitializeGuard<#SYM>() + [&]()
#define initialize_guard_ptr(PTR) ::hk::util::InitializeGuard(ptr(PTR)) + [&]()

} // namespace hk::util
