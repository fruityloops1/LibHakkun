#pragma once

#include "hk/diag/diag.h"
#include "hk/types.h"
#include "hk/util/Algorithm.h"
#include "hk/util/Allocator.h"
#include "hk/util/Array.h"
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
    class Vec : public Span<T> {
        size mCapacity = 0;

        using Span<T>::mData;
        using Span<T>::mSize;

        T* valueAt(size index) {
            return &mData[index];
        }

        const T* valueAt(size index) const {
            return &mData[index];
        }

        static Span<T> allocate(size size, ::size capacity) {
            capacity = std::min(capacity, ReserveSize);

            T* data = cast<T*>(Allocator::allocate(capacity * sizeof(T), alignof(T)));

            HK_ABORT_UNLESS(data != nullptr, "hk::util::Vec<%s>::allocate(): allocation failed (Capacity = %zu)", getTypeName<T>(), capacity);

            return Span<T>(data, size);
        }

    public:
        using Span<T>::empty;
        using Span<T>::begin;
        using Span<T>::move;
        using Span<T>::forEach;

        Vec()
            : Span<T>(allocate(0, ReserveSize)) { }

        Vec(const Vec& other)
            : Span<T>(allocate(other.size(), other.capacity()))
            , mCapacity(other.mCapacity) {
            for (::size i = 0; i < other.mSize; i++)
                new (valueAt(i)) T(*other.valueAt(i));
        }

        Vec(Vec&& other)
            : Span<T>(other.mData, other.size())
            , mCapacity(other.mCapacity) {
            other.mCapacity = 0;
            other.set(nullptr, 0);
        }

        Vec(std::initializer_list<T> list)
            : Span<T>(allocate(list.size(), list.size()))
            , mCapacity(list.size()) {
            std::move(list.begin(), list.end(), begin());
        }

        template <size Capacity>
        Vec(const util::Array<T, Capacity>& array)
            : Span<T>(allocate(Capacity, Capacity))
            , mCapacity(Capacity) {
            for (::size i = 0; i < Capacity; i++) {
                new (valueAt(i)) T(array[i]);
            }
        }

        template <size Capacity>
        Vec(util::Array<T, Capacity>&& array)
            : Span<T>(allocate(Capacity, Capacity))
            , mCapacity(Capacity) {
            for (::size i = 0; i < Capacity; i++) {
                new (valueAt(i)) T(::move(array[i]));
            }
        }

        Vec& operator=(const Vec& other) {
            this->~Vec();
            new (this) Vec(other);
            return *this;
        }

        Vec& operator=(Vec&& other) {
            this->~Vec();
            new (this) Vec(move(other));
            return *this;
        }

        ~Vec() {
            if (mData != nullptr) {
                clear();

                Allocator::free(mData);
                mData = nullptr;
            }
        }

        void reserve(size newCapacity) {
            if (mCapacity >= newCapacity)
                return;

            T* newData = cast<T*>(Allocator::allocate(newCapacity * sizeof(T), alignof(T)));
            for (::size i = 0; i < mSize; i++) {
                new (&newData[i]) T(::move(*valueAt(i)));
                valueAt(i)->~T();
            }

            Allocator::free(mData);
            mData = newData;
            newCapacity = newCapacity;
        }

        void add(const T& value) {
            if (mSize >= mCapacity)
                reserve(mCapacity + ReserveSize);
            new (valueAt(mSize++)) T(value);
        }

        void add(T&& value) {
            if (mSize >= mCapacity)
                reserve(mCapacity + ReserveSize);
            new (valueAt(mSize++)) T(::move(value));
        }

        T& insert(const T& value, size index = 0) {
            if (mSize >= mCapacity)
                reserve(mCapacity + ReserveSize);

            HK_ABORT_UNLESS(index <= mSize, "hk::util::Vec<%s>::insert(index: %zu): out of range (size: %zu)", getTypeName<T>(), index, mSize);
            move(index + 1, index, mSize++ - index);
            return *new (valueAt(index)) T(value);
        }

        T& insert(T&& value, size index = 0) {
            if (mSize >= mCapacity)
                reserve(mCapacity + ReserveSize);

            HK_ABORT_UNLESS(index <= mSize, "hk::util::Vec<%s>::insert(index: %zu): out of range (size: %zu)", getTypeName<T>(), index, mSize);
            move(index + 1, index, mSize++ - index);
            return *new (valueAt(index)) T(::move(value));
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

        void extend(size newSize, const T& extendValue = T()) {
            HK_ABORT_UNLESS(newSize >= mSize, "hk::util::Vec<%s>::extend(%zu): out of range (size: %zu)", getTypeName<T>(), newSize, mSize);
            reserve(newSize);
            std::fill(valueAt(mSize), valueAt(newSize), extendValue);
            mSize = newSize;
        }

        void truncate(size newSize) {
            HK_ABORT_UNLESS(newSize <= mSize, "hk::util::Vec<%s>::truncate(%zu): out of range (size: %zu)", getTypeName<T>(), newSize, mSize);
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

        void clear() {
            forEach([](T& data) -> void {
                data.~T();
            });
            mSize = 0;
        }

        ::size capacity() const { return mCapacity; }
    };

} // namespace hk::util
