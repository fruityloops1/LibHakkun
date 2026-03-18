#pragma once

#include "hk/diag/diag.h"
#include "hk/types.h"
#include "hk/util/Algorithm.h"
#include "hk/util/Allocator.h"
#include "hk/util/Span.h"
#include "hk/util/TypeName.h"
#include <algorithm>
#include <cstdlib>
#include <type_traits>

namespace hk::util {

    /**
     * @brief Vector.
     *
     * @tparam T
     * @tparam ReserveSize Amount of elements to reserve by default or when pushing elements past the current capacity
     */
    template <typename T, size ReserveSize = 16, AllocatorType Allocator = DefaultAllocator>
    class Vec : Span<T> {
        size mSize = 0;

        using Span<T>::mPtr;

        T* valueAt(size index) {
            return &cast<T*>(mPtr)[index];
        }

        const T* valueAt(size index) const {
            return &cast<const T*>(mPtr)[index];
        }

    public:
        Vec() {
            mPtr = cast<T*>(Allocator::allocate(ReserveSize * sizeof(T), alignof(T)));
            HK_ABORT_UNLESS(mPtr != nullptr, "hk::util::Vec<%s>::Vec(): allocation failed (ReserveSize = %zu)", getTypeName<T>(), ReserveSize);
        }

        Vec(const Vec& other)
            : Span<T>(mData(cast<T*>(Allocator::allocate(other.mCapacity * sizeof(T), alignof(T)))), other.capacity())
            , mSize(other.mSize) {
            HK_ABORT_UNLESS(mPtr != nullptr, "hk::util::Vec<%s>::Vec(const Vec&): allocation failed (Capacity = %zu)", getTypeName<T>(), capacity());
            for (::size i = 0; i < other.mSize; i++)
                new (valueAt(i)) T(*other.valueAt(i));
        }

        Vec(Vec&& other)
            : Span<T>(other.mPtr, other.capacity())
            , mSize(other.mSize) {
            other.mPtr = nullptr;
            other.set(nullptr, 0);
        }

        ~Vec() {
            if (mPtr != nullptr) {
                clear();

                Allocator::free(mPtr);
            }
        }

        void reserve(size newCapacity) {
            if (capacity() >= newCapacity)
                return;

            T* newData = cast<T*>(Allocator::allocate(newCapacity * sizeof(T), alignof(T)));
            for (::size i = 0; i < mSize; i++) {
                new (&newData[i]) T(::move(*valueAt(i)));
                valueAt(i)->~T();
            }

            Allocator::free(mPtr);
            mPtr = newData;
            newCapacity = newCapacity;
        }

        void add(const T& value) {
            if (mSize >= capacity())
                reserve(capacity() + ReserveSize);
            new (valueAt(mSize++)) T(value);
        }

        void add(T&& value) {
            if (mSize >= capacity())
                reserve(capacity() + ReserveSize);
            new (valueAt(mSize++)) T(::move(value));
        }

        T& insert(const T& value, size index = 0) {
            if (mSize >= capacity())
                reserve(capacity() + ReserveSize);

            HK_ABORT_UNLESS(index <= mSize, "hk::util::Vec<%s>::insert(index: %zu): out of range (size: %zu)", getTypeName<T>(), index, mSize);
            move(index + 1, index, mSize++ - index);
            return *new (valueAt(index)) T(value);
        }

        T& insert(T&& value, size index = 0) {
            if (mSize >= capacity())
                reserve(capacity() + ReserveSize);

            HK_ABORT_UNLESS(index <= mSize, "hk::util::Vec<%s>::insert(index: %zu): out of range (size: %zu)", getTypeName<T>(), index, mSize);
            move(index + 1, index, mSize++ - index);
            return *new (valueAt(index)) T(::move(value));
        }

        void move(size dstIdx, size srcIdx, size toMove) {
            if (dstIdx == srcIdx or toMove == 0)
                return;

            if constexpr (std::is_trivially_move_constructible_v<T> and std::is_trivially_destructible_v<T>)
                std::memmove(valueAt(dstIdx), valueAt(srcIdx), toMove * sizeof(T));
            else {
                if (dstIdx < srcIdx)
                    for (::size i = 0; i < toMove; i++) {
                        T* to = valueAt(dstIdx + i);
                        T* from = valueAt(srcIdx + i);
                        new (to) T(::move(*from));
                        from->~T();
                    }
                else
                    for (::size i = toMove; i != 0; i--) {
                        T* to = valueAt(dstIdx + i - 1);
                        T* from = valueAt(srcIdx + i - 1);
                        new (to) T(::move(*from));
                        from->~T();
                    }
            }
        }

        T remove(size index) {
            HK_ABORT_UNLESS(index < mSize, "hk::util::Vec<%s>::remove(%zu): out of range (size: %zu)", getTypeName<T>(), index, mSize);

            T removedValue = T(::move(*valueAt(index)));
            valueAt(index)->~T();

            if (index < mSize - 1)
                move(index, index + 1, mSize - index - 1);

            mSize--;

            return ::move(removedValue);
        }

        T& operator[](size index) {
            HK_ABORT_UNLESS(index < mSize, "hk::util::Vec<%s>::operator[](%zu): out of range (size: %zu)", getTypeName<T>(), index, mSize);
            return *valueAt(index);
        }

        const T& operator[](size index) const {
            HK_ABORT_UNLESS(index < mSize, "hk::util::Vec<%s>::operator[](%zu): out of range (size: %zu)", getTypeName<T>(), index, mSize);

            return *valueAt(index);
        }

        T& first() {
            HK_ABORT_UNLESS(!empty(), "hk::util::Vec<%s>::first(): empty", getTypeName<T>());
            return *valueAt(0);
        }

        const T& first() const {
            HK_ABORT_UNLESS(!empty(), "hk::util::Vec<%s>::first(): empty", getTypeName<T>());
            return *valueAt(0);
        }

        T& last() {
            HK_ABORT_UNLESS(!empty(), "hk::util::Vec<%s>::last(): empty", getTypeName<T>());
            return *valueAt(mSize - 1);
        }

        const T& last() const {
            HK_ABORT_UNLESS(!empty(), "hk::util::Vec<%s>::last(): empty", getTypeName<T>());
            return *valueAt(mSize - 1);
        }

        template <typename Callback>
        void forEach(Callback func) {
            for (::size i = 0; i < mSize; i++)
                func((*this)[i]);
        }

        template <typename Callback>
        void forEach(Callback func) const {
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

        template <typename ST, typename GetFunc>
        size binarySearch(GetFunc getValue, ST searchValue, bool findBetween = false) const {
            return util::binarySearch([this, getValue](s32 index) {
                const T* value = valueAt(index);
                return getValue(*value);
            },
                0, mSize - 1, searchValue, findBetween);
        }

        ::size size() const { return mSize; }
        ::size capacity() const { return Span<T>::size(); }

        T* begin() { return valueAt(0); }
        T* end() { return valueAt(mSize); }
        const T* begin() const { return valueAt(0); }
        const T* end() const { return valueAt(mSize); }
    };

} // namespace hk::util
