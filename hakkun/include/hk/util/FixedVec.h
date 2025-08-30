#pragma once

#include "hk/diag/diag.h"
#include "hk/types.h"
#include "hk/util/TypeName.h"
#include <algorithm>
#include <type_traits>

namespace hk::util {

    /**
     * @brief Heapless vector.
     *
     * @tparam T
     * @tparam Capacity
     */
    template <typename T, size Capacity>
    class FixedVec {
        alignas(alignof(T)) u8 mStorage[sizeof(T) * Capacity] { 0 };
        size mSize = 0;

        T* valueAt(size index) {
            return &cast<T*>(mStorage)[index];
        }

        const T* valueAt(size index) const {
            return &cast<const T*>(mStorage)[index];
        }

    public:
        FixedVec() = default;
        FixedVec(const FixedVec& other)
            : mSize(other.mSize) {
            for (::size i = 0; i < other.mSize; i++)
                new (valueAt(i)) T(*other.valueAt(i));
        }

        FixedVec(FixedVec&& other)
            : mSize(other.mSize) {
            for (::size i = 0; i < other.mSize; i++) {
                new (valueAt(i)) T(move(*other.valueAt(i)));
                other.valueAt(i)->~T();
            }
        }

        ~FixedVec() {
            clear();
        }

        void add(const T& value) {
            HK_ABORT_UNLESS(mSize < Capacity, "hk::util::FixedVec<%s, %zu>::add: Full", getTypeName<T>(), Capacity);
            new (valueAt(mSize++)) T(value);
        }

        void add(T&& value) {
            HK_ABORT_UNLESS(mSize < Capacity, "hk::util::FixedVec<%s, %zu>::add: Full", getTypeName<T>(), Capacity);
            new (valueAt(mSize++)) T(move(value));
        }

        T remove(size index) {
            HK_ABORT_UNLESS(index < mSize, "hk::util::FixedVec<%s, %zu>::remove(%zu): out of range (size: %zu)", getTypeName<T>(), Capacity, index, mSize);

            T removedValue = T(move(*valueAt(index)));
            valueAt(index)->~T();

            if (index < mSize - 1) {
                ::size toMove = mSize - index - 1;
                if constexpr (std::is_trivially_move_constructible_v<T> and std::is_trivially_destructible_v<T>)
                    std::memmove(valueAt(index), valueAt(index + 1), toMove * sizeof(T));
                else
                    for (::size i = 0; i < toMove; i++) {
                        T* to = valueAt(index + i);
                        T* from = valueAt(index + i + 1);
                        new (to) T(move(*from));
                        from->~T();
                    }
            }

            mSize--;

            return move(removedValue);
        }

        T& operator[](size index) {
            HK_ABORT_UNLESS(index < mSize, "hk::util::FixedVec<%s, %zu>::operator[](%zu): out of range (size: %zu)", getTypeName<T>(), Capacity, index, mSize);
            return *valueAt(index);
        }

        const T& operator[](size index) const {
            HK_ABORT_UNLESS(index < mSize, "hk::util::FixedVec<%s, %zu>::operator[](%zu): out of range (size: %zu)", getTypeName<T>(), Capacity, index, mSize);

            return *valueAt(index);
        }

        T& first() {
            HK_ABORT_UNLESS(!empty(), "hk::util::FixedVec<%s, %zu>::first(): empty", getTypeName<T>(), Capacity);
            return *valueAt(0);
        }

        const T& first() const {
            HK_ABORT_UNLESS(!empty(), "hk::util::FixedVec<%s, %zu>::first(): empty", getTypeName<T>(), Capacity);
            return *valueAt(0);
        }

        T& last() {
            HK_ABORT_UNLESS(!empty(), "hk::util::FixedVec<%s, %zu>::last(): empty", getTypeName<T>(), Capacity);
            return *valueAt(mSize - 1);
        }

        const T& last() const {
            HK_ABORT_UNLESS(!empty(), "hk::util::FixedVec<%s, %zu>::last(): empty", getTypeName<T>(), Capacity);
            return *valueAt(mSize - 1);
        }

        template <typename Callback>
        void forEach(Callback func) {
            for (::size i = 0; i < mSize; i++)
                func((*this)[i]);
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
