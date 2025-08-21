#ifndef HK_DISABLE_SAIL

#include "hk/sail/init.h"
#include "hk/diag/diag.h"
#include "hk/init/module.h"
#include "hk/ro/ElfUtil.h"
#include "hk/ro/RoUtil.h"
#include "hk/sail/detail.h"
#include "hk/svc/api.h"
#include "hk/util/Algorithm.h"
#include "hk/util/hash.h"

namespace hk::sail {

    static volatile u64 sTimeElapsedLoadSymbols;

    void loadSymbols() {
        detail::VersionLoader::loadVersions();

        sTimeElapsedLoadSymbols = svc::getSystemTick();

        {
            const auto data = ro::parseDynamic(init::getModuleStart(), init::_DYNAMIC);
            data.forEachPlt([&](const Elf_Rela& entry) {
                Elf_Addr* ptr = cast<Elf_Addr*>(init::getModuleStart() + entry.r_offset);
                Elf_Xword symIndex = ELF_R_SYM(entry.r_info);
                const Elf_Sym& sym = data.dynsym[symIndex];
                if (sym.st_name) {
                    bool abort = ELF_ST_BIND(sym.st_info) != STB_WEAK;

                    if constexpr (sail::sUsePrecalcHashes)
                        *ptr = lookupSymbolFromDb<true>(cast<const u32*>(data.dynstr + sym.st_name), abort);
                    else
                        *ptr = lookupSymbolFromDb<false>(data.dynstr + sym.st_name, abort);
                }
            });
        }

        sTimeElapsedLoadSymbols = svc::getSystemTick() - sTimeElapsedLoadSymbols;

        float s = sTimeElapsedLoadSymbols / float(svc::getSystemTickFrequency());
        float ms = s * 1000.f;
        u64 µs = ms * 1000.f;
        diag::debugLog("hk::sail: loaded symbols in: %zutix / %.2fms / %zuus", sTimeElapsedLoadSymbols, ms, µs);
    }

} // namespace hk::sail

#endif