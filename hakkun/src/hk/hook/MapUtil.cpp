#include "hk/hook/MapUtil.h"
#include "hk/diag/diag.h"
#include "hk/hook/results.h"
#include "hk/svc/api.h"
#include "hk/svc/types.h"
#include "hk/types.h"
#include "hk/util/Random.h"

namespace hk::hook {

    static bool regionsOverlap(ptr a, size aSize, ptr b, size bSize) {
        return a < b + bSize && b < a + aSize;
    }

    ptr findAslr(size searchSize) {
        searchSize = alignUpPage(searchSize);

        u64 aslrStart = 0;
        HK_ABORT_UNLESS_R(svc::GetInfo(&aslrStart, svc::InfoType_AslrRegionAddress, svc::PseudoHandle::CurrentProcess, 0));
        u64 aslrSize = 0;
        HK_ABORT_UNLESS_R(svc::GetInfo(&aslrSize, svc::InfoType_AslrRegionSize, svc::PseudoHandle::CurrentProcess, 0));

        u64 aliasStart = 0;
        HK_ABORT_UNLESS_R(svc::GetInfo(&aliasStart, svc::InfoType_AliasRegionAddress, svc::PseudoHandle::CurrentProcess, 0));
        u64 aliasSize = 0;
        HK_ABORT_UNLESS_R(svc::GetInfo(&aliasSize, svc::InfoType_AliasRegionSize, svc::PseudoHandle::CurrentProcess, 0));

        u64 heapStart = 0;
        HK_ABORT_UNLESS_R(svc::GetInfo(&heapStart, svc::InfoType_HeapRegionAddress, svc::PseudoHandle::CurrentProcess, 0));
        u64 heapSize = 0;
        HK_ABORT_UNLESS_R(svc::GetInfo(&heapSize, svc::InfoType_HeapRegionSize, svc::PseudoHandle::CurrentProcess, 0));

        size maxPage = (aslrSize - searchSize) >> 12;

        while (true) {
            size randomPage = util::getRandomU64() % maxPage;

            ptr attempt = aslrStart + randomPage * cPageSize;
            svc::MemoryInfo memInfo;
            u32 pageInfo;

            HK_ABORT_UNLESS_R(svc::QueryMemory(&memInfo, &pageInfo, attempt));

            if (memInfo.state == svc::MemoryState_Free && attempt + searchSize <= memInfo.base_address + memInfo.size
                && !regionsOverlap(attempt, searchSize, aliasStart, aliasSize)
                && !regionsOverlap(attempt, searchSize, heapStart, heapSize))
                return attempt;
        }
    }

    Result mapRoToRw(ptr addr, size mapSize, ptr* outRw) {
        ptr srcAligned = alignDownPage(addr);
        ptrdiff ptrToAlignedDiff = addr - srcAligned;

        size uppedSize = alignUpPage(mapSize + ptrToAlignedDiff);
        ptr dest = findAslr(uppedSize);

        Handle curProcess;
        HK_ABORT_UNLESS_R(svc::getProcessHandleMesosphere(&curProcess));

        // HK_TRY(svc::MapProcessMemory(dest, curProcess, srcAligned, uppedSize));
        HK_ABORT_UNLESS_R(svc::MapProcessMemory(dest, curProcess, srcAligned, uppedSize));

        *outRw = dest + ptrToAlignedDiff;

        return ResultSuccess();
    }

    Result mapRwToRx(ptr rw, size mapSize, ptr* outRx) {
        if (not isAlignedPage(rw) or not isAlignedPage(mapSize) or (*outRx != 0 and not isAlignedPage(*outRx)))
            return ResultAddressNotAligned();

        if (*outRx != 0) {
            svc::MemoryInfo info;
            u32 page;
            HK_ABORT_UNLESS_R(svc::QueryMemory(&info, &page, *outRx));

            if (info.state != svc::MemoryState_Free or info.size < mapSize)
                return ResultMapAddressNotViable();
        } else
            *outRx = findAslr(mapSize);

        Handle curProcess;
        HK_ABORT_UNLESS_R(svc::getProcessHandleMesosphere(&curProcess));

        // diag::debugLog("svc::MapProcessCodeMemory(%x, %zx, %zx, %zx)", curProcess, *outRx, rw, mapSize);
        // HK_TRY(svc::MapProcessCodeMemory(curProcess, *outRx, rw, mapSize));

        // diag::debugLog("svc::MapMemory(%zx, %zx, %zx)", *outRx, rw, mapSize);
        // HK_TRY(svc::MapMemory(*outRx, rw, mapSize));
        //
        // diag::debugLog("svc::SetProcessMemoryPermission(%x, %zx, %zx, %d)", curProcess, *outRx, mapSize, svc::MemoryPermission_ReadExecute);
        // HK_TRY(svc::SetProcessMemoryPermission(curProcess, *outRx, mapSize, svc::MemoryPermission_ReadExecute));

        return ResultSuccess();
    }

    Result unmapRwToRx(ptr rw, size mapSize, ptr rx) {
        Handle curProcess;
        HK_ABORT_UNLESS_R(svc::getProcessHandleMesosphere(&curProcess));

        HK_TRY(svc::UnmapProcessCodeMemory(curProcess, rx, rw, mapSize));

        return ResultSuccess();
    }

} // namespace hk::hook
