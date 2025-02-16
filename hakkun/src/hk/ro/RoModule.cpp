#include "hk/ro/RoModule.h"
#include "hk/hook/MapUtil.h"
#include "hk/ro/results.h"
#include "hk/svc/api.h"
#include "hk/svc/types.h"

namespace hk::ro {

    Result RoModule::findRanges() {

        svc::MemoryInfo curRangeInfo;
        u32 pageInfo;
        HK_TRY(svc::QueryMemory(&curRangeInfo, &pageInfo, module->m_Base));
        HK_UNLESS(curRangeInfo.permission == svc::MemoryPermission_ReadExecute, ResultUnusualSectionLayout());

        text = { curRangeInfo.base_address, curRangeInfo.size };

        HK_TRY(svc::QueryMemory(&curRangeInfo, &pageInfo, curRangeInfo.base_address + curRangeInfo.size));
        HK_UNLESS(curRangeInfo.permission == svc::MemoryPermission_Read, ResultUnusualSectionLayout());

        rodata = { curRangeInfo.base_address, curRangeInfo.size };

        while (curRangeInfo.permission == svc::MemoryPermission_Read)
            HK_TRY(svc::QueryMemory(&curRangeInfo, &pageInfo, curRangeInfo.base_address + curRangeInfo.size));

        HK_UNLESS(curRangeInfo.permission == svc::MemoryPermission_ReadWrite, ResultUnusualSectionLayout());

        data = { curRangeInfo.base_address, curRangeInfo.size };

        return ResultSuccess();
    }

    Result RoModule::mapRw() {
        HK_UNLESS(textRw.start() == 0, ResultAlreadyMapped());
        HK_UNLESS(rodataRw.start() == 0, ResultAlreadyMapped());

        ptr rw = 0;
        HK_TRY(hook::mapRoToRw(text.start(), text.size(), &rw));
        textRw = { rw, text.size() };
        HK_TRY(hook::mapRoToRw(rodata.start(), rodata.size(), &rw));
        rodataRw = { rw, rodata.size() };

        return ResultSuccess();
    }

    static RoWriteCallback sRoWriteCallback = nullptr;

    Result RoModule::writeRo(ptr offset, const void* source, size writeSize) const {
        HK_UNLESS(textRw.start() != 0, ResultNotMapped());
        HK_UNLESS(rodataRw.start() != 0, ResultNotMapped());

        ptr roPtr = text.start() + offset;

        HK_UNLESS(roPtr >= text.start(), ResultOutOfRange());
        HK_UNLESS(roPtr + writeSize <= rodata.end(), ResultOutOfRange());

        ptr rwPtr = roPtr >= rodata.start() ? ptr(rodataRw.start() + offset - text.size()) : ptr(textRw.start() + offset);

        if (sRoWriteCallback)
            sRoWriteCallback(this, offset, source, writeSize);
        __builtin_memcpy(cast<void*>(rwPtr), source, writeSize);

        return ResultSuccess();
    }

    void setRoWriteCallback(RoWriteCallback callback) {
        sRoWriteCallback = callback;
    }

} // namespace hk::ro
