#pragma once

#include "hk/ro/RoUtil.h"
#include "hk/sail/detail.h"
#include "hk/types.h"
#include "hk/util/TemplateString.h"

namespace hk::util {

    template <util::TemplateString Symbol>
    hk_alwaysinline ptr lookupSymbol() {
#ifdef HK_DISABLE_SAIL
        return ro::lookupSymbol(Symbol.value);
#else
        static ptr addr = 0;
        if (addr)
            return addr;

        if constexpr (sail::sUsePrecalcHashes) {
            constexpr u32 symMurmur = util::hashMurmur(Symbol.value);
            addr = sail::lookupSymbolFromDb<true>(&symMurmur);
        } else {
            addr = sail::lookupSymbolFromDb<false>(Symbol.value);
        }
        return addr;
#endif
    }

    hk_noinline ptr getReturnAddress(int n);

} // namespace hk::util
