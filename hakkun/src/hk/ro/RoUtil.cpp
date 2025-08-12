#include "hk/ro/RoUtil.h"
#include "hk/Result.h"
#include "hk/diag/diag.h"
#include "hk/init/module.h"
#include "rtld/RoModuleList.h"
#include <algorithm>

namespace nn::ro::detail {

    RoModuleList* g_pAutoLoadList = nullptr;

} // namespace nn::ro::detail

using namespace nn::ro::detail;

namespace hk::ro {

    static RoModule sModules[cMaxModuleNum] {};
    static size sNumModules = 0;
    static int sSelfModuleIdx = -1;

    void RoUtil::initModuleList() {
        sNumModules = 0;

        for (nn::ro::detail::RoModule* rtldModule : *nn::ro::detail::g_pAutoLoadList) {
            sModules[sNumModules++].mModule = rtldModule;
        }

        diag::debugLog("hk::ro: %d modules collected from g_pAutoLoadList:", sNumModules);

        std::sort(sModules, sModules + sNumModules, [](const RoModule& a, const RoModule& b) {
            if (a.getNnModule() == nullptr || a.getNnModule()->m_Base == 0 || b.getNnModule() == nullptr || b.getNnModule()->m_Base == 0)
                HK_ABORT_UNLESS_R(ResultRtldModuleInvalid());
            return a.getNnModule()->m_Base < b.getNnModule()->m_Base;
        });

        for (int i = 0; i < sNumModules; i++) {
            auto& module = sModules[i];
            auto* nnModule = module.getNnModule();

            if (nnModule == &init::hkRtldModule)
                sSelfModuleIdx = i;

            Result rc = ResultSuccess();
            if (nnModule == nullptr || nnModule->m_Base == 0)
                rc = ResultRtldModuleInvalid();
            HK_ABORT_UNLESS_R(rc);

            HK_ABORT_UNLESS_R(module.findRanges());
            HK_ABORT_UNLESS_R(module.mapRw());
            module.findBuildId();

            {
                const auto& range = module.range();
                const auto& text = module.text();
                const auto& rodata = module.rodata();
                const auto& data = module.data();

                diag::debugLog("Module[%d]:", i);
                diag::debugLog("\tName: %s", module.getModuleName());
                if (module.getBuildId() != nullptr) {
                    const u8* d = module.getBuildId();

                    static_assert(cBuildIdSize == 0x10);
                    diag::debugLog("\tBuildId: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x (...)",
                        d[0], d[1], d[2], d[3],
                        d[4], d[5], d[6], d[7],
                        d[8], d[9], d[10], d[11],
                        d[12], d[13], d[14], d[15]);
                } else {
                    diag::debugLog("\tBuildId: NotFound");
                }
                diag::debugLog("\tRange: %p-%p", range.start(), range.end() - 1);
                diag::debugLog("\tText: %p-%p", text.start(), text.end() - 1);
                diag::debugLog("\tRodata: %p-%p", rodata.start(), rodata.end() - 1);
                diag::debugLog("\tData/Bss: %p-%p", data.start(), data.end() - 1);
                diag::debugLog("\tnn::ro::detail::RoModule*: %p", nnModule);
            }
        }

        HK_ASSERT(sSelfModuleIdx != -1);
    }

    hk_alwaysinline size getNumModules() { return sNumModules; }

    hk_alwaysinline RoModule* getModuleByIndex(int idx) {
        if (idx >= sNumModules)
            return nullptr;
        return &sModules[idx];
    }

    hk_alwaysinline RoModule* getSelfModule() { return getModuleByIndex(sSelfModuleIdx); }
    hk_alwaysinline RoModule* getRtldModule() { return getModuleByIndex(0); }
    hk_alwaysinline RoModule* getMainModule() { return getModuleByIndex(1); }
#ifndef TARGET_IS_STATIC
    hk_alwaysinline RoModule* getSdkModule() { return getModuleByIndex(sNumModules - 1); }
#endif

    RoModule* getModuleContaining(ptr addr) {
        for (int i = 0; i < sNumModules; i++) {
            auto* module = &sModules[i];
            if (addr >= module->range().start() && addr <= module->range().end())
                return module;
        }
        return nullptr;
    }

    Result getModuleBuildIdByIndex(int idx, u8* out) {
        // assert(idx < sNumModules);

        const u8* buildId = getModuleByIndex(idx)->getBuildId();
        if (buildId != nullptr) {
            __builtin_memcpy(out, buildId, cBuildIdSize);
            return ResultSuccess();
        }
        return ResultGnuHashMissing();
    }

    ptr lookupSymbol(const char* symbol) {
        for (int i = 0; i < sNumModules; i++) {
            auto* module = sModules[i].getNnModule();
            Elf_Sym* sym = module->GetSymbolByName(symbol);
            if (sym) {
                ptr value = module->m_Base + sym->st_value;
                return value;
            }
        }
        return 0;
    }

    ptr lookupSymbol(uint64_t bucketHash, uint32_t djb2Hash, uint32_t murmurHash) {
        for (int i = 0; i < sNumModules; i++) {
            auto* module = sModules[i].getNnModule();
            Elf_Sym* sym = module->GetSymbolByHashes(bucketHash, djb2Hash, murmurHash);
            if (sym) {
                ptr value = module->m_Base + sym->st_value;
                return value;
            }
        }
        return 0;
    }

} // namespace hk::ro
