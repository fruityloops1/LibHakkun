#include "hk/hook/MapUtil.h"
#include "hk/diag/diag.h"
#include "hk/svc/api.h"
#include "hk/svc/types.h"
#include "hk/types.h"
#include "hk/util/Random.h"

namespace hk::hook {

    static bool regionsOverlap(ptr a, size aSize, ptr b, size bSize) {
        return a < b + bSize && b < a + aSize;
    }

    static hk_alwaysinline ptr findMap(size searchSize, svc::InfoType searchRegionAddr, svc::InfoType searchRegionSize) {
        searchSize = alignUpPage(searchSize);

        const u64 regionStart = HK_UNWRAP(svc::GetInfo(searchRegionAddr, svc::PseudoHandle::CurrentProcess, 0));
        const u64 regionSize = HK_UNWRAP(svc::GetInfo(searchRegionSize, svc::PseudoHandle::CurrentProcess, 0));

        const u64 aliasStart = HK_UNWRAP(svc::GetInfo(svc::InfoType_AliasRegionAddress, svc::PseudoHandle::CurrentProcess, 0));
        const u64 aliasSize = HK_UNWRAP(svc::GetInfo(svc::InfoType_AliasRegionSize, svc::PseudoHandle::CurrentProcess, 0));

        const u64 heapStart = HK_UNWRAP(svc::GetInfo(svc::InfoType_HeapRegionAddress, svc::PseudoHandle::CurrentProcess, 0));
        const u64 heapSize = HK_UNWRAP(svc::GetInfo(svc::InfoType_HeapRegionSize, svc::PseudoHandle::CurrentProcess, 0));

        size maxPage = (regionSize - searchSize) / cPageSize;

        while (true) {
            size randomPage = util::getRandomU64() % maxPage;

            ptr attempt = regionStart + randomPage * cPageSize;
            svc::MemoryInfo memInfo;
            u32 pageInfo;

            HK_ABORT_UNLESS_R(svc::QueryMemory(&memInfo, &pageInfo, attempt));

            if (memInfo.state == svc::MemoryState_Free && attempt + searchSize <= memInfo.base_address + memInfo.size
                && !regionsOverlap(attempt, searchSize, aliasStart, aliasSize)
                && !regionsOverlap(attempt, searchSize, heapStart, heapSize))
                return attempt;
        }
    }

    ptr findAslr(size searchSize) {
        return findMap(searchSize, svc::InfoType_AslrRegionAddress, svc::InfoType_AslrRegionSize);
    }

    ptr findStack(size searchSize) {
        return findMap(searchSize, svc::InfoType_StackRegionAddress, svc::InfoType_StackRegionSize);
    }

    ValueOrResult<ptr> mapRoToRw(ptr addr, size mapSize) {
        ptr srcAligned = alignDownPage(addr);
        ptrdiff ptrToAlignedDiff = addr - srcAligned;

        size uppedSize = alignUpPage(mapSize + ptrToAlignedDiff);
        ptr dest = findAslr(uppedSize);

        Handle curProcess = HK_TRY(svc::getProcessHandleMesosphere());

        HK_TRY(svc::MapProcessMemory(dest, curProcess, srcAligned, uppedSize));

        return dest + ptrToAlignedDiff;
    }

} // namespace hk::hook
