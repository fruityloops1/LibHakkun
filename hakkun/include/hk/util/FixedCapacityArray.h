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

        T& operator[](size index) {
            return mData[index];
        }
        const T& operator[](size index) const {
            return mData[index];
        }

        template <typename Callback>
        void forEach(Callback func) {
            for (::size i = 0; i < mSize; i++)
                func(mData[i]);
        }

        void clear() {
            forEach([](T& data) -> void {
                data.~T();
            });
            mSize = 0;
        }

        bool empty() const { return mSize == 0; }

        void sort() {
            std::sort(valueAt(0), valueAt(mSize));
        }

        template <typename Compare>
        void sort(Compare comp) {
            std::sort(valueAt(0), valueAt(mSize), comp);
        }

        ::size size() { return mSize; }
        constexpr static ::size capacity() { return Capacity; }
    };

} // namespace hk::util
