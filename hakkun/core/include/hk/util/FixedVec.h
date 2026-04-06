#pragma once

#include "hk/diag/diag.h"
#include "hk/types.h"
#include "hk/util/Algorithm.h"
#include "hk/util/Array.h"
#include "hk/util/Span.h"
#include "hk/util/TypeName.h"
#include <algorithm>
#include <initializer_list>
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
                new (valueAt(i)) T(::move(*other.valueAt(i)));
                other.valueAt(i)->~T();
            }
            other.mSize = 0;
        }

        FixedVec(std::initializer_list<T> list)
            : mSize(std::min(list.size(), Capacity)) {
            std::move(list.begin(), list.end(), begin());
        }

        FixedVec(const util::Array<T, Capacity>& array)
            : mSize(Capacity) {
            for (::size i = 0; i < Capacity; i++) {
                new (valueAt(i)) T(array[i]);
            }
        }

        FixedVec(util::Array<T, Capacity>&& array)
            : mSize(Capacity) {
            for (::size i = 0; i < Capacity; i++) {
                new (valueAt(i)) T(::move(array[i]));
            }
        }

        FixedVec& operator=(const FixedVec& other) {
            this->~FixedVec();
            new (this) FixedVec(other);
            return *this;
        }

        FixedVec& operator=(FixedVec&& other) {
            this->~FixedVec();
            new (this) FixedVec(move(other));
            return *this;
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
            new (valueAt(mSize++)) T(::move(value));
        }

        T& insert(const T& value, size index = 0) {
            HK_ABORT_UNLESS(mSize < Capacity, "hk::util::FixedVec<%s, %zu>::add: Full", getTypeName<T>(), Capacity);
            HK_ABORT_UNLESS(index <= mSize, "hk::util::FixedVec<%s, %zu>::insert(index: %zu): out of range (size: %zu)", getTypeName<T>(), Capacity, index, mSize);
            move(index + 1, index, mSize++ - index);
            return *new (valueAt(index)) T(value);
        }

        T& insert(T&& value, size index = 0) {
            HK_ABORT_UNLESS(mSize < Capacity, "hk::util::FixedVec<%s, %zu>::add: Full", getTypeName<T>(), Capacity);
            HK_ABORT_UNLESS(index <= mSize, "hk::util::FixedVec<%s, %zu>::insert(index: %zu): out of range (size: %zu)", getTypeName<T>(), Capacity, index, mSize);
            move(index + 1, index, mSize++ - index);
            return *new (valueAt(index)) T(::move(value));
        }

        void move(size dstIdx, size srcIdx, size toMove) {
            Span<T>(*this).move(dstIdx, srcIdx, toMove);
        }

        T remove(size index) {
            HK_ABORT_UNLESS(index < mSize, "hk::util::FixedVec<%s, %zu>::remove(%zu): out of range (size: %zu)", getTypeName<T>(), Capacity, index, mSize);

            T removedValue = T(::move(*valueAt(index)));
            valueAt(index)->~T();

            if (index < mSize - 1)
                move(index, index + 1, mSize - index - 1);

            mSize--;

            return ::move(removedValue);
        }

        void extend(size newSize, const T& extendValue = T()) {
            HK_ABORT_UNLESS(newSize >= mSize && newSize <= Capacity, "hk::util::FixedVec<%s>::extend(%zu): out of range (size: %zu)", Capacity, newSize, mSize);
            std::fill(valueAt(mSize), valueAt(newSize), extendValue);
            mSize = newSize;
        }

        void truncate(size newSize) {
            HK_ABORT_UNLESS(newSize <= mSize, "hk::util::FixedVec<%s>::truncate(%zu): out of range (size: %zu)", Capacity, newSize, mSize);
            for (::size i = newSize; i < mSize; i++)
                valueAt(i)->~T();
            mSize = newSize;
        }

        void resize(size newSize, const T& extendValue = T()) {
            if (mSize == newSize)
                return;

            if (mSize > newSize)
                truncate(newSize);
            else
                extend(newSize, extendValue);
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
            Span<T>(*this).forEach(func);
        }

        template <typename Callback>
        void forEach(Callback func) const {
            Span<T>(*this).forEach(func);
        }

        void clear() {
            forEach([](T& data) -> void {
                data.~T();
            });
            mSize = 0;
        }

        bool empty() const { return mSize == 0; }

        void sort() {
            Span<T>(*this).sort();
        }

        template <typename Compare>
        void sort(Compare comp) {
            Span<T>(*this).sort(comp);
        }

        template <bool FindBetweenIdx = false, typename ST, typename GetFunc>
        s32 binarySearch(GetFunc getValue, ST searchValue, bool findBetween = false) const {
            return Span<T>(*this).binarySearch(getValue, searchValue, findBetween);
        }

        ::size size() const { return mSize; }
        constexpr static ::size capacity() { return Capacity; }

        T* begin() { return valueAt(0); }
        T* end() { return valueAt(mSize); }
        const T* begin() const { return valueAt(0); }
        const T* end() const { return valueAt(mSize); }

        operator Span<T>() { return { valueAt(0), mSize }; }
        operator Span<const T>() const { return { valueAt(0), mSize }; }
    };

} // namespace hk::util
