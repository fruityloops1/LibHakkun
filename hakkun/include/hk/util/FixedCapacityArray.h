#pragma once

#include "hk/diag/diag.h"
#include "hk/types.h"
#include <algorithm>

namespace hk::util {

    template <typename T, size Capacity>
    class FixedCapacityArray {
        alignas(alignof(T)) u8 mStorage[sizeof(T) * Capacity] { 0 };
        size mSize = 0;

        T* valueAt(size index) {
            return &cast<T*>(mStorage)[index];
        }

        const T* valueAt(size index) const {
            return &cast<const T*>(mStorage)[index];
        }

    public:
        void add(T value) {
            HK_ABORT_UNLESS(mSize < Capacity, "hk::util::FixedCapacityArray<T, %zu>::add: Full", Capacity);
            *valueAt(mSize++) = value;
        }

        T remove(size index) {
            HK_ABORT_UNLESS(index < mSize, "hk::util::FixedCapacityArray<T, %zu>::remove(%zu): out of range (size: %zu)", Capacity, index, mSize);

            T removedValue = std::move(*valueAt(index));

            if (index < mSize - 1) {
                ::size toMove = mSize - index - 1;
                std::memmove(valueAt(index), valueAt(index + 1), toMove * sizeof(T));
            }

            mSize--;

            return std::move(removedValue);
        }

        T& operator[](size index) {
            HK_ABORT_UNLESS(index < mSize, "hk::util::FixedCapacityArray<T, %zu>::operator[](%zu): out of range (size: %zu)", Capacity, index, mSize);
            return *valueAt(index);
        }

        const T& operator[](size index) const {
            HK_ABORT_UNLESS(index < mSize, "hk::util::FixedCapacityArray<T, %zu>::operator[](%zu): out of range (size: %zu)", Capacity, index, mSize);

            return *valueAt(index);
        }

        template <typename Callback>
        void forEach(Callback func) {
            for (::size i = 0; i < mSize; i++)
                func(*this[i]);
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

        ::size size() const { return mSize; }
        constexpr static ::size capacity() { return Capacity; }

        T* begin() { return valueAt(0); }
        T* end() { return valueAt(mSize); }
        const T* begin() const { return valueAt(0); }
        const T* end() const { return valueAt(mSize); }
    };

} // namespace hk::util
