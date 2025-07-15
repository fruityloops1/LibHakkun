#pragma once

#include "hk/ro/RoModule.h"
#include "hk/ro/results.h"
#include "hk/types.h"
#include "rtld/RoModule.h"

namespace hk::ro {

    constexpr static size cMaxModuleNum = 1 /* rtld */ + 1 /* main */ + 10 /* subsdk0-subsdk9 */ + 1 /* nnsdk */;
    constexpr static size cBuildIdSize = 0x10; // 0x20 but 0x10 because that's usually the minimum size and the linker Loves to not give it 0x20 space and put some SDK+MW balls garbage right into the build id instwead of letting it pad the Zeroes

    size getNumModules();
    const RoModule* getModuleByIndex(int idx);

    const RoModule* getMainModule();
    const RoModule* getSelfModule();
    const RoModule* getRtldModule();
#ifndef TARGET_IS_STATIC
    const RoModule* getSdkModule();
#endif

    const RoModule* getModuleContaining(ptr addr);

    Result getModuleBuildIdByIndex(int idx, u8* out);

    ptr lookupSymbol(const char* symbol);
    ptr lookupSymbol(uint64_t bucketHash, uint32_t djb2Hash, uint32_t murmurHash);

    struct RoUtil {
        static void initModuleList();
    };

} // namespace hk::ro
