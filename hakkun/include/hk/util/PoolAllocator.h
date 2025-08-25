#pragma once

#include "hk/diag/diag.h"
#include "hk/types.h"
#include "hk/util/BitArray.h"
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

        T* getData(size index) const {
            HK_ABORT_UNLESS(index >= 0 && index < Capacity, "PoolAllocator<%s, %zu>: invalid index (%d not in buffer)", getTypeName<T>(), Capacity, index);
            return mBuffer + index;
        }

    public:
        PoolAllocator(void* buffer)
            : mBuffer(cast<T*>(buffer)) { }

        s32 allocateIdx() {
            for (size i = 0; i < Capacity; i++) {
                if (mAllocations[i] == false) {
                    mAllocations[i] = true;
                    return i;
                }
            }
            return -1;
        }

        T* allocate() {
            s32 idx = allocateIdx();
            return idx == -1 ? nullptr : getData(idx);
        }

        void freeIdx(s32 index) {
            HK_ABORT_UNLESS(index >= 0 && index < Capacity, "PoolAllocator<%s, %zu>: invalid free (%d not in buffer)", getTypeName<T>(), Capacity, index);
            HK_ABORT_UNLESS(mAllocations[index] == true, "PoolAllocator<%s, %zu>: double free (idx %d)", getTypeName<T>(), Capacity, index);

            mAllocations[index] = false;
        }

        void free(T* data) {
            size index = data - mBuffer;
            HK_ABORT_UNLESS(index >= 0 && index < Capacity, "PoolAllocator<%s, %zu>: invalid free (%p not in buffer)", getTypeName<T>(), Capacity, data);
            HK_ABORT_UNLESS(mAllocations[index] == true, "PoolAllocator<%s, %zu>: double free (ptr %p, idx %d)", getTypeName<T>(), Capacity, data, index);

            freeIdx(index);
        }
    };

    template <typename T, size Capacity>
    class BufferPoolAllocator : public PoolAllocator<T, Capacity> {
        u8 mData[sizeof(T) * Capacity] { 0 };

    public:
        BufferPoolAllocator()
            : PoolAllocator<T, Capacity>(mData) { }
    };

} // namespace hk::util
