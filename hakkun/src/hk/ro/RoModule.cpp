#include "hk/ro/RoModule.h"
#include "hk/diag/diag.h"
#include "hk/hook/MapUtil.h"
#include "hk/ro/results.h"
#include "hk/svc/api.h"
#include "hk/svc/types.h"

namespace hk::ro {

    Result RoModule::findRanges() {
        svc::MemoryInfo curRangeInfo;
        u32 pageInfo;
        HK_TRY(svc::QueryMemory(&curRangeInfo, &pageInfo, mModule->m_Base));

        HK_ASSERT(curRangeInfo.base_address == mModule->m_Base);

        HK_UNLESS(curRangeInfo.permission == svc::MemoryPermission_ReadExecute, ResultUnusualSectionLayout());
#ifdef __aarch64__
        mTextRange = { curRangeInfo.base_address, curRangeInfo.size };
#else
        mTextRange = { uintptr_t(curRangeInfo.base_address), size(curRangeInfo.size) };
#endif

        auto prev = mModule->m_Base;
        HK_TRY(svc::QueryMemory(&curRangeInfo, &pageInfo, curRangeInfo.base_address + curRangeInfo.size));

        HK_UNLESS(curRangeInfo.permission == svc::MemoryPermission_Read, ResultUnusualSectionLayout());

#ifdef __aarch64__
        mRodataRange = { curRangeInfo.base_address, curRangeInfo.size };
#else
        mRodataRange = { uintptr_t(curRangeInfo.base_address), size(curRangeInfo.size) };
#endif

        while (curRangeInfo.permission == svc::MemoryPermission_Read)
            HK_TRY(svc::QueryMemory(&curRangeInfo, &pageInfo, curRangeInfo.base_address + curRangeInfo.size));

        HK_UNLESS(curRangeInfo.permission == svc::MemoryPermission_ReadWrite, ResultUnusualSectionLayout());

#ifdef __aarch64__
        mDataRange = { curRangeInfo.base_address, curRangeInfo.size };
#else
        mDataRange = { uintptr_t(curRangeInfo.base_address), size(curRangeInfo.size) };
#endif

        return ResultSuccess();
    }

    Result RoModule::mapRw() {
        HK_UNLESS(mTextRwMapping.start() == 0, ResultAlreadyMapped());
        HK_UNLESS(mRodataRwMapping.start() == 0, ResultAlreadyMapped());

        ptr rw = HK_TRY(hook::mapRoToRw(mTextRange.start(), mTextRange.size()));
        mTextRwMapping = { rw, mTextRange.size() };
        rw = HK_TRY(hook::mapRoToRw(mRodataRange.start(), mRodataRange.size()));
        mRodataRwMapping = { rw, mRodataRange.size() };

        return ResultSuccess();
    }

    Result RoModule::findBuildId() {
        constexpr char sGnuHashMagic[] = { 'G', 'N', 'U', '\0' };
        ptr rodataEnd = rodata().end();
        for (ptr search = rodataEnd; search >= rodataEnd - hk::cPageSize * 2; search--) {
            if (__builtin_memcmp((void*)search, sGnuHashMagic, sizeof(sGnuHashMagic)) == 0) {
                const u8* buildId = (const u8*)(search + sizeof(sGnuHashMagic));
                mBuildId = buildId;
                return ResultSuccess();
            }
        }

        return ResultGnuHashMissing();
    }

    static RoWriteCallback sRoWriteCallback = nullptr;

    Result RoModule::writeRo(ptr offset, const void* source, size writeSize) const {
        HK_UNLESS(mTextRwMapping.start() != 0, ResultNotMapped());
        HK_UNLESS(mRodataRwMapping.start() != 0, ResultNotMapped());

        ptr roPtr = mTextRange.start() + offset;

        HK_UNLESS(roPtr >= mTextRange.start(), ResultOutOfRange());
        HK_UNLESS(roPtr + writeSize <= mRodataRange.end(), ResultOutOfRange());

        bool isText = !(roPtr >= mRodataRange.start());

        ptr rwPtr = isText ? ptr(mTextRwMapping.start() + offset) : ptr(mRodataRwMapping.start() + offset - mTextRange.size());

        if (sRoWriteCallback)
            sRoWriteCallback(this, offset, source, writeSize);
        __builtin_memcpy(cast<void*>(rwPtr), source, writeSize);
        if (isText)
            svc::clearCache(roPtr, writeSize);

        return ResultSuccess();
    }

    void setRoWriteCallback(RoWriteCallback callback) {
        sRoWriteCallback = callback;
    }

} // namespace hk::ro
