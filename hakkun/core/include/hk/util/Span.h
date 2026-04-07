#pragma once

#include "hk/diag/diag.h"
#include "hk/types.h"
#include "hk/util/Algorithm.h"
#include "hk/util/TypeName.h"
#include <iterator>
#include <span>
#include <type_traits>

namespace hk::util {

    namespace detail {

        template <typename T>
        class SpanBase {
        protected:
            T* mData = nullptr;
            size mSize = 0;

        public:
            constexpr SpanBase() = default;
            constexpr SpanBase(T* data, size size)
                : mData(data)
                , mSize(size) { }

            constexpr void set(T* data, size size) {
                mData = data;
                mSize = size;
            }

            constexpr T* data() { return mData; }
            constexpr const T* data() const { return mData; }
            constexpr size size_bytes() const { return mSize * sizeof(T); }
            constexpr size size() const { return mSize; }
            constexpr bool empty() const { return mSize == 0; }

            class ConstIterator {
            public:
                using iterator_category = std::forward_iterator_tag;
                using difference_type = ptrdiff;
                using value_type = T;
                using pointer = const T*;
                using reference = const T&;

            private:
                pointer mCur;

            public:
                constexpr ConstIterator(pointer cur)
                    : mCur(cur) { }

                constexpr ConstIterator& operator++() {
                    mCur++;
                    return *this;
                }
                constexpr ConstIterator operator++(s32) { ++*this; }

                constexpr bool operator==(const ConstIterator& other) const { return mCur == other.mCur; }
                constexpr bool operator!=(const ConstIterator& other) const { return !(*this == other); }

                constexpr reference operator*() const { return *mCur; }
                constexpr pointer operator->() const { return mCur; }
                constexpr operator pointer() const { return mCur; }
            };

            constexpr ConstIterator begin() const { return { mData }; }
            constexpr ConstIterator end() const { return { mData + mSize }; }

            constexpr const T& operator[](::size index) const {
                HK_ABORT_UNLESS(index < mSize, "hk::util::Span<%s>::operator[%zu]: out of range (size: %zu)", getTypeName<T>(), index, mSize);
                return mData[index];
            }

            constexpr const T& at(::size index) const {
                HK_ABORT_UNLESS(index < mSize, "hk::util::Span<%s>::at(%zu): out of range (size: %zu)", getTypeName<T>(), index, mSize);
                return mData[index];
            }

            template <typename ST, typename GetFunc>
            constexpr ::size binarySearch(GetFunc getValue, ST searchValue, bool findBetween = false) const {
                return util::binarySearch([this, getValue](s32 index) {
                    const T* value = &mData[index];
                    return getValue(*value);
                },
                    0, mSize - 1, searchValue, findBetween);
            }

            constexpr const T& first() const {
                HK_ABORT_UNLESS(!empty(), "hk::util::Span<%s>::first(): empty", getTypeName<T>());
                return mData[0];
            }

            constexpr const T& last() const {
                HK_ABORT_UNLESS(!empty(), "hk::util::Span<%s>::last(): empty", getTypeName<T>());
                return mData[mSize - 1];
            }

            template <typename Callback>
            constexpr void forEach(Callback func) const {
                for (::size i = 0; i < mSize; i++)
                    func((*this)[i]);
            }
        };

    } // namespace detail

    template <typename T>
    class Span : public detail::SpanBase<T> {
    protected:
        using detail::SpanBase<T>::mData;
        using detail::SpanBase<T>::mSize;

    public:
        using detail::SpanBase<T>::empty;

        using detail::SpanBase<T>::operator[];
        using detail::SpanBase<T>::at;
        using detail::SpanBase<T>::first;
        using detail::SpanBase<T>::last;
        using detail::SpanBase<T>::begin;
        using detail::SpanBase<T>::end;
        using detail::SpanBase<T>::set;

        constexpr Span() = default;
        constexpr Span(T* data, size size)
            : detail::SpanBase<T>(data, size) { }

        constexpr Span(const Span<const T>& other) = delete;
        constexpr Span(const Span& other)
            : detail::SpanBase<T>(other.mData, other.mSize) { }

        constexpr Span(const std::span<const T>& other) = delete;
        constexpr Span(const std::span<T>& other)
            : detail::SpanBase<T>(other.data(), other.size()) { }

        constexpr T& operator[](::size index) {
            HK_ABORT_UNLESS(index < mSize, "hk::util::Span<%s>::operator[%zu]: out of range (size: %zu)", getTypeName<T>(), index, mSize);
            return mData[index];
        }

        constexpr T& at(::size index) {
            HK_ABORT_UNLESS(index < mSize, "hk::util::Span<%s>::at(%zu): out of range (size: %zu)", getTypeName<T>(), index, mSize);
            return mData[index];
        }

        constexpr void set(::size index, const T& value) {
            HK_ABORT_UNLESS(index < mSize, "hk::util::Span<%s>::set(%zu): out of range (size: %zu)", getTypeName<T>(), index, mSize);
            mData[index] = value;
        }

        constexpr void set(::size index, T&& value) {
            HK_ABORT_UNLESS(index < mSize, "hk::util::Span<%s>::set(%zu): out of range (size: %zu)", getTypeName<T>(), index, mSize);
            new (&mData[index]) T(value);
        }

        class Iterator {
            using iterator_category = std::forward_iterator_tag;
            using difference_type = ptrdiff;
            using value_type = T;
            using pointer = T*;
            using reference = T&;

            T* mCur;

        public:
            constexpr Iterator(pointer cur)
                : mCur(cur) { }

            constexpr Iterator& operator++() {
                mCur++;
                return *this;
            }

            constexpr bool operator==(const Iterator& other) const { return mCur == other.mCur; }
            constexpr bool operator!=(const Iterator& other) const { return !(*this == other); }

            constexpr reference operator*() const { return *mCur; }
            constexpr pointer operator->() const { return mCur; }
            constexpr operator pointer() const { return mCur; }
        };

        constexpr Iterator begin() { return { mData }; }
        constexpr Iterator end() { return { mData + mSize }; }

        constexpr T& first() {
            HK_ABORT_UNLESS(!empty(), "hk::util::Span<%s>::first(): empty", getTypeName<T>());
            return mData[0];
        }

        constexpr T& last() {
            HK_ABORT_UNLESS(!empty(), "hk::util::Span<%s>::last(): empty", getTypeName<T>());
            return mData[mSize - 1];
        }

        template <typename Callback>
        constexpr void forEach(Callback func) {
            for (::size i = 0; i < mSize; i++)
                func((*this)[i]);
        }

        constexpr void sort() {
            std::sort(&mData[0], &mData[mSize]);
        }

        template <typename Compare>
        constexpr void sort(Compare comp) {
            std::sort(&mData[0], &mData[mSize], comp);
        }

        constexpr void move(size dstIdx, size srcIdx, size toMove) {
            if (dstIdx == srcIdx or toMove == 0)
                return;

            if (!std::is_constant_evaluated() and std::is_trivially_move_constructible_v<T> and std::is_trivially_destructible_v<T>)
                std::memmove(&mData[dstIdx], &mData[srcIdx], toMove * sizeof(T));
            else {
                if (dstIdx < srcIdx)
                    for (::size i = 0; i < toMove; i++) {
                        T* to = &mData[dstIdx + i];
                        T* from = &mData[srcIdx + i];
                        new (to) T(::move(*from));
                        from->~T();
                    }
                else
                    for (::size i = toMove; i != 0; i--) {
                        T* to = &mData[dstIdx + i - 1];
                        T* from = &mData[srcIdx + i - 1];
                        new (to) T(::move(*from));
                        from->~T();
                    }
            }
        }
    };

    template <typename T>
    class Span<const T> : public detail::SpanBase<const T> {
    public:
        constexpr Span() = default;
        constexpr Span(const T* data, size size)
            : detail::SpanBase<const T>(data, size) { }

        constexpr Span(const Span<T>& other)
            : detail::SpanBase<const T>(other.data(), other.size()) { }
        constexpr Span(const Span& other)
            : detail::SpanBase<const T>(other.mData, other.mSize) { }

        constexpr Span(const std::span<const T>& other)
            : detail::SpanBase<const T>(other.data(), other.size()) { }
        constexpr Span(const std::span<T>& other)
            : detail::SpanBase<const T>(other.data(), other.size()) { }
    };

    template <>
    class Span<void> : public Span<u8> {
    public:
        constexpr Span() = default;
        constexpr Span(void* data, ::size size)
            : Span<u8>(cast<u8*>(data), size) { }
    };

    template <>
    class Span<const void> : public Span<const u8> {
    public:
        constexpr Span() = default;
        constexpr Span(const void* data, ::size size)
            : Span<const u8>(cast<const u8*>(data), size) { }
    };

} // namespace hk::util
