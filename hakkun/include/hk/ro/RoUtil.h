#pragma once

#include "hk/ro/RoModule.h"
#include "hk/ro/results.h"
#include "hk/svc/api.h"
#include "hk/svc/types.h"
#include "hk/types.h"
#include "rtld/RoModule.h"

namespace hk::ro {

    constexpr static size cMaxModuleNum = 1 /* rtld */ + 1 /* main */ + 10 /* subsdk0-subsdk9 */ + 1 /* nnsdk */;
    constexpr static size cBuildIdSize = 0x10; // 0x20 but 0x10 because that's usually the minimum size and the linker Loves to not give it 0x20 space and put some SDK+MW balls garbage right into the build id instwead of letting it pad the Zeroes

    size getNumModules();
    RoModule* getModuleByIndex(int idx);

    RoModule* getMainModule();
    RoModule* getSelfModule();
    RoModule* getRtldModule();
#ifndef TARGET_IS_STATIC
    RoModule* getSdkModule();
#endif

    RoModule* getModuleContaining(ptr addr);

    Result getModuleBuildIdByIndex(int idx, u8* out);

    ptr lookupSymbol(const char* symbol);
    ptr lookupSymbol(uint64_t bucketHash, uint32_t djb2Hash, uint32_t murmurHash);

    struct RoUtil {
        static void initModuleList();
    };

    template <typename Func, typename QueryFunc>
    hk_alwaysinline inline Result forEachAutoLoadModuleImpl(ptr autoLoadBase, const Func& func, const QueryFunc& query) {
        ptr addr = autoLoadBase;
        while (true) {
            svc::MemoryInfo curRangeInfo;
            u32 page;

            HK_TRY(query(&curRangeInfo, &page, addr));

            if (curRangeInfo.state == svc::MemoryState_Free)
                break;
            HK_UNLESS(curRangeInfo.permission == svc::MemoryPermission_ReadExecute, ResultUnusualSectionLayout());

            ptr textBase = curRangeInfo.base_address;

            HK_TRY(query(&curRangeInfo, &page, curRangeInfo.base_address + curRangeInfo.size));
            HK_UNLESS(curRangeInfo.permission == svc::MemoryPermission_Read, ResultUnusualSectionLayout());

            ptr rodataBase = curRangeInfo.base_address;

            while (curRangeInfo.permission == svc::MemoryPermission_Read)
                HK_TRY(query(&curRangeInfo, &page, curRangeInfo.base_address + curRangeInfo.size));

            ptr bssBase = curRangeInfo.base_address;
            HK_UNLESS(curRangeInfo.permission == svc::MemoryPermission_ReadWrite, ResultUnusualSectionLayout());

            func(textBase, rodataBase, bssBase);

            addr = curRangeInfo.base_address + curRangeInfo.size;
        }

        return ResultSuccess();
    }

    template <typename Func>
    hk_alwaysinline inline Result forEachAutoLoadModule(ptr autoLoadBase, const Func& func) {
        return forEachAutoLoadModuleImpl(autoLoadBase, func, [](svc::MemoryInfo* outMemoryInfo, u32* outPageInfo, ptr address) -> Result { return svc::QueryMemory(outMemoryInfo, outPageInfo, address); });
    }

    template <typename Func>
    hk_alwaysinline inline Result forEachAutoLoadModule(ptr autoLoadBase, Handle debugHandle, const Func& func) {
        return forEachAutoLoadModuleImpl(autoLoadBase, func, [&](svc::MemoryInfo* outMemoryInfo, u32* outPageInfo, ptr address) -> Result { return svc::QueryDebugProcessMemory(outMemoryInfo, outPageInfo, debugHandle, address); });
    }

} // namespace hk::ro
