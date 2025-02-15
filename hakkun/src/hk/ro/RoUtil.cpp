#include "hk/ro/RoUtil.h"
#include "hk/Result.h"
#include "hk/diag/diag.h"
#include "hk/init/module.h"
#include "rtld/RoModuleList.h"

namespace nn::ro::detail {

    RoModuleList* g_pAutoLoadList = nullptr;

} // namespace nn::ro::detail

using namespace nn::ro::detail;

namespace hk::ro {
    struct BuildId {
        Result findResult {};
        u8 data[sBuildIdSize] { 0 };
    };

    static Result findBuildId(const RoModule& module, u8* out) {

        constexpr char sGnuHashMagic[] = { 'G', 'N', 'U', '\0' };
        ptr rodataEnd = module.rodata.end();
        for (ptr search = rodataEnd; search >= rodataEnd - hk::sPageSize * 2; search--) {
            if (__builtin_memcmp((void*)search, sGnuHashMagic, sizeof(sGnuHashMagic)) == 0) {
                const u8* buildId = (const u8*)(search + sizeof(sGnuHashMagic));
                __builtin_memcpy(out, buildId, sBuildIdSize);
                return ResultSuccess();
            }
        }

        return ResultGnuHashMissing();
    }

    static RoModule sModules[sMaxModuleNum] {};
    static BuildId sBuildIds[sMaxModuleNum] {};
    static size sNumModules = 0;
    static int sSelfModuleIdx = -1;

    void initModuleList() {
        __builtin_memset(sModules, 0, sizeof(sModules));
        __builtin_memset(sBuildIds, 0, sizeof(sBuildIds));
        sNumModules = 0;

        for (nn::ro::detail::RoModule* rtldModule : *nn::ro::detail::g_pAutoLoadList) {
            sModules[sNumModules++].module = rtldModule;
        }

        for (int i = 0; i < sNumModules; i++) {
            auto& module = sModules[i];

            if (module.module == &init::hkRtldModule)
                sSelfModuleIdx = i;

            Result rc = ResultSuccess();
            if (module.module == nullptr || module.module->m_Base == 0)
                rc = ResultRtldModuleInvalid();
            HK_ABORT_UNLESS_R(rc);

            HK_ABORT_UNLESS_R(module.findRanges());
            HK_ABORT_UNLESS_R(module.mapRw());

            auto& buildId = sBuildIds[i];
            buildId.findResult = findBuildId(module, buildId.data);
        }
    }

    hk_alwaysinline size getNumModules() { return sNumModules; }

    const hk_alwaysinline RoModule* getModuleByIndex(int idx) {
        if (idx >= sNumModules)
            return nullptr;
        return &sModules[idx];
    }

    const hk_alwaysinline RoModule* getSelfModule() { return getModuleByIndex(sSelfModuleIdx); }
    const hk_alwaysinline RoModule* getRtldModule() { return getModuleByIndex(0); }
    const hk_alwaysinline RoModule* getMainModule() { return getModuleByIndex(1); }
#ifndef TARGET_IS_STATIC
    const hk_alwaysinline RoModule* getSdkModule() { return getModuleByIndex(sNumModules - 1); }
#endif

    const RoModule* getModuleContaining(ptr addr) {
        for (int i = 0; i < sNumModules; i++) {
            auto* module = &sModules[i];
            if (addr >= module->range().start() && addr <= module->range().end())
                return module;
        }
        return nullptr;
    }

    Result getModuleBuildIdByIndex(int idx, u8* out) {
        // assert(idx < sNumModules);
        auto& entry = sBuildIds[idx];
        if (entry.findResult.succeeded())
            __builtin_memcpy(out, entry.data, sizeof(entry.data));
        return entry.findResult;
    }

    ptr lookupSymbol(const char* symbol) {
        for (int i = 0; i < sNumModules; i++) {
            auto* module = sModules[i].module;
            Elf_Sym* sym = module->GetSymbolByName(symbol);
            if (sym) {
                ptr value = module->m_Base + sym->st_value;
                return value;
            }
        }
        return 0;
    }

    ptr lookupSymbol(uint64_t bucketHash, uint32_t murmurHash) {
        for (int i = 0; i < sNumModules; i++) {
            auto* module = sModules[i].module;
            Elf_Sym* sym = module->GetSymbolByHashes(bucketHash, murmurHash);
            if (sym) {
                ptr value = module->m_Base + sym->st_value;
                return value;
            }
        }
        return 0;
    }

} // namespace hk::ro
