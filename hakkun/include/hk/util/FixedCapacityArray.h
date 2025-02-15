#pragma once

#include "hk/diag/diag.h"
#include "hk/types.h"
#include <algorithm>
#include <array>

namespace hk::util {

    template <typename T, size Capacity>
    class FixedCapacityArray {
        std::array<T, Capacity> mData;
        size mSize = 0;

    public:
        void add(T value) {
            HK_ABORT_UNLESS(mSize < Capacity, "hk::util::FixedCapacityArray<T, %zu>::add: Full", Capacity);
            mData[mSize++] = value;
        }

        template <typename Callback>
        void forEach(Callback func) {
            for (size i = 0; i < mSize; i++)
                func(mData[i]);
        }

        bool empty() const { return mSize == 0; }

        void sort() {
            std::sort(mData.begin(), mData.begin() + mSize);
        }

        template <typename Compare>
        void sort(Compare comp) {
            std::sort(mData.begin(), mData.begin() + mSize, comp);
        }
    };

} // namespace hk::util
