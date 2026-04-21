#pragma once

#include "hk/container/BitArray.h"
#include "hk/diag/diag.h"
#include "hk/types.h"
#include "hk/util/TypeName.h"

namespace hk::util {

    /**
     * @brief Allocates/deallocates pointers from a sized buffer. Object construction/lifetime is up to the user.
     *
     * @tparam T
     * @tparam Capacity
     */
    template <typename T, size Capacity>
    class PoolAllocator {
        BitArray<Capacity> mAllocations;
        T* mBuffer = nullptr;

        constexpr T* getData(size index) const {
            HK_ABORT_UNLESS(index >= 0 && index < Capacity, "PoolAllocator<%s, %zu>: invalid index (%zu not in buffer)", getTypeName<T>(), Capacity, index);
            return mBuffer + index;
        }

    public:
        constexpr PoolAllocator(T* buffer)
            : mBuffer(buffer) { }

        constexpr s32 allocateIdx() {
            for (size i = 0; i < Capacity; i++) {
                if (mAllocations[i] == false) {
                    mAllocations[i] = true;
                    return i;
                }
            }
            return -1;
        }

        constexpr T* allocate() {
            s32 idx = allocateIdx();
            return idx == -1 ? nullptr : getData(idx);
        }

        constexpr void freeIdx(s32 index) {
            HK_ABORT_UNLESS(index >= 0 && index < Capacity, "PoolAllocator<%s, %zu>: invalid free (%d not in buffer)", getTypeName<T>(), Capacity, index);
            HK_ABORT_UNLESS(mAllocations[index] == true, "PoolAllocator<%s, %zu>: double free (idx %d)", getTypeName<T>(), Capacity, index);

            mAllocations[index] = false;
        }

        constexpr void free(T* data) {
            size index = data - mBuffer;
            HK_ABORT_UNLESS(index >= 0 && index < Capacity, "PoolAllocator<%s, %zu>: invalid free (%p not in buffer)", getTypeName<T>(), Capacity, data);
            HK_ABORT_UNLESS(mAllocations[index] == true, "PoolAllocator<%s, %zu>: double free (ptr %p, idx %zd)", getTypeName<T>(), Capacity, data, index);

            freeIdx(index);
        }
    };

    template <typename T, size Capacity>
    class BufferPoolAllocator : public PoolAllocator<T, Capacity> {
        alignas(T) u8 mData[sizeof(T) * Capacity] { 0 };

    public:
        constexpr BufferPoolAllocator()
            : PoolAllocator<T, Capacity>(cast<T*>(mData)) { }
    };

} // namespace hk::util
