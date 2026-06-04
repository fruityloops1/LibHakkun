#pragma once

#include "hk/ro/RoUtil.h"
#include "hk/sail/detail.h" // IWYU pragma: keep
#include "hk/types.h"
#include "hk/util/TemplateString.h"

namespace hk::util {

    /**
     * @brief Looks up symbol from sail, or from RTLD modules if sail is disabled.
     *
     * @tparam Symbol Symbol to look up
     * @return
     */
    template <util::TemplateString Symbol>
    hk_alwaysinline ptr lookupSymbol() {
#ifdef HK_DISABLE_SAIL
        return ro::lookupSymbol(Symbol.value);
#else
        static ptr addr = 0;
        if (addr)
            return addr;

        if constexpr (sail::sUsePrecalcHashes) {
            constinit static const u32 symMurmur = util::hashMurmur(Symbol.value);
            addr = sail::lookupSymbolFromDb<true>(&symMurmur);
        } else {
            addr = sail::lookupSymbolFromDb<false>(Symbol.value);
        }
        return addr;
#endif
    }

    namespace detail {

        using VisitReturnAddrFuncPtr = void (*)(void*, ptr, int);

        hk_noinline void visitReturnAddressesImpl(VisitReturnAddrFuncPtr func, void* userData, int n = IntegerTraits<int>::cMax);

    } // namespace detail

    /**
     * @brief Get nth return address from stack frame.
     *
     * @param n
     * @return ptr
     */
    hk_noinline ptr getReturnAddress(int n);

    template <LambdaType L>
    hk_alwaysinline void visitReturnAddresses(L&& func) {
        detail::visitReturnAddressesImpl([](void* userData, ptr addr, int level) {
            L& func = *cast<L*>(userData);

            func(addr, level);
        },
            &func);
    }

} // namespace hk::util
