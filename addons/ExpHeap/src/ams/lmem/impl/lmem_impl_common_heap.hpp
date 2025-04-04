/*
 * Copyright (c) Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include "ams/lmem/impl/lmem_impl_common.hpp"
#include "ams/lmem/lmem_common.hpp"
#include "ams/util/util_fourcc.hpp"
#include "hk/types.h"

namespace ams::lmem::impl {

    constexpr inline u32 ExpHeapMagic = util::ReverseFourCC<'E', 'X', 'P', 'H'>::Code;
    constexpr inline u32 FrameHeapMagic = util::ReverseFourCC<'F', 'R', 'M', 'H'>::Code;
    constexpr inline u32 UnitHeapMagic = util::ReverseFourCC<'U', 'N', 'T', 'H'>::Code;

    class ScopedHeapLock {
        NON_COPYABLE(ScopedHeapLock);
        NON_MOVABLE(ScopedHeapLock);

    private:
        HeapHandle m_handle;

    public:
        explicit ScopedHeapLock(HeapHandle h)
            : m_handle(h) {
            if (m_handle->option & CreateOption_ThreadSafe) {
                m_handle->mutex.lock();
            }
        }

        ~ScopedHeapLock() {
            if (m_handle->option & CreateOption_ThreadSafe) {
                m_handle->mutex.unlock();
            }
        }
    };

    hk_alwaysinline inline MemoryRange MakeMemoryRange(void* address, size_t size) {
        return MemoryRange { .address = reinterpret_cast<uintptr_t>(address), .size = size };
    }

    hk_alwaysinline inline void* GetHeapStartAddress(HeapHandle handle) {
        return handle->heap_start;
    }

    hk_alwaysinline inline size_t GetPointerDifference(const void* start, const void* end) {
        return reinterpret_cast<uintptr_t>(end) - reinterpret_cast<uintptr_t>(start);
    }

    constexpr hk_alwaysinline size_t GetPointerDifference(uintptr_t start, uintptr_t end) {
        return end - start;
    }

    void InitializeHeapHead(HeapHead* out, u32 magic, void* start, void* end, u32 option);
    void FinalizeHeap(HeapHead* heap);
    bool ContainsAddress(HeapHandle handle, const void* address);
    size_t GetHeapTotalSize(HeapHandle handle);

    /* Debug Fill */
    u32 GetDebugFillValue(FillType type);
    u32 SetDebugFillValue(FillType type, u32 value);

    inline void FillMemory(void* dst, u32 fill_value, size_t size) {
        /* All heap blocks must be at least 32-bit aligned. */
        HK_ASSERT(hk::isAligned(reinterpret_cast<uintptr_t>(dst), alignof(u32)));
        HK_ASSERT(hk::isAligned(size, sizeof(u32)));
        for (size_t i = 0; i < size / sizeof(fill_value); i++) {
            reinterpret_cast<u32*>(dst)[i] = fill_value;
        }
    }

    inline void FillUnallocatedMemory(HeapHead* heap, void* address, size_t size) {
        if (heap->option & CreateOption_DebugFill) {
            FillMemory(address, impl::GetDebugFillValue(FillType_Unallocated), size);
        }
    }

    inline void FillAllocatedMemory(HeapHead* heap, void* address, size_t size) {
        if (heap->option & CreateOption_ZeroClear) {
            FillMemory(address, 0, size);
        } else if (heap->option & CreateOption_DebugFill) {
            FillMemory(address, impl::GetDebugFillValue(FillType_Allocated), size);
        }
    }

    inline void FillFreedMemory(HeapHead* heap, void* address, size_t size) {
        if (heap->option & CreateOption_DebugFill) {
            FillMemory(address, impl::GetDebugFillValue(FillType_Freed), size);
        }
    }

}