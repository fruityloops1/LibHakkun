#pragma once

#include "hk/diag/diag.h"
#include "hk/types.h"
#include "hk/util/Algorithm.h"
#include "hk/util/TypeName.h"
#include <span>

namespace hk::util {

    namespace detail {

        template <typename T>
        class SpanBase {
        protected:
            T* mData = nullptr;
            size mSize = 0;

        public:
            SpanBase() = default;
            SpanBase(T* data, size size)
                : mData(data)
                , mSize(size) { }

            void set(T* data, size size) {
                mData = data;
                mSize = size;
            }

            T* data() { return mData; }
            const T* data() const { return mData; }
            size size_bytes() const { return mSize * sizeof(T); }
            size size() const { return mSize; }
            bool empty() const { return mSize == 0; }

            class ConstIterator {
                const T* mCur;

            public:
                ConstIterator(const T* cur)
                    : mCur(cur) { }

                ConstIterator& operator++() {
                    mCur++;
                    return *this;
                }

                bool operator==(const ConstIterator& other) const { return mCur == other.mCur; }
                bool operator!=(const ConstIterator& other) const { return !(*this == other); }

                const T& operator*() const { return *mCur; }
                const T* operator->() const { return mCur; }
                operator const T*() const { return mCur; }
            };

            ConstIterator begin() const { return { mData }; }
            ConstIterator end() const { return { mData + mSize }; }

            const T& operator[](::size index) const {
                HK_ABORT_UNLESS(index < mSize, "hk::util::Span<%s>::operator[%zu]: out of range (size: %zu)", getTypeName<T>(), index, mSize);
                return mData[index];
            }

            const T& at(::size index) const {
                HK_ABORT_UNLESS(index < mSize, "hk::util::Span<%s>::at(%zu): out of range (size: %zu)", getTypeName<T>(), index, mSize);
                return mData[index];
            }

            template <typename ST, typename GetFunc>
            ::size binarySearch(GetFunc getValue, ST searchValue, bool findBetween = false) const {
                return util::binarySearch([this, getValue](s32 index) {
                    const T* value = &mData[index];
                    return getValue(*value);
                },
                    0, mSize - 1, searchValue, findBetween);
            }

            const T& first() const {
                HK_ABORT_UNLESS(!empty(), "hk::util::Span<%s>::first(): empty", getTypeName<T>());
                return mData[0];
            }

            const T& last() const {
                HK_ABORT_UNLESS(!empty(), "hk::util::Span<%s>::last(): empty", getTypeName<T>());
                return mData[mSize - 1];
            }

            template <typename Callback>
            void forEach(Callback func) const {
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

        Span() = default;
        Span(T* data, size size)
            : detail::SpanBase<T>(data, size) { }

        Span(const Span<const T>& other) = delete;
        Span(const Span& other)
            : detail::SpanBase<T>(other.mData, other.mSize) { }

        Span(const std::span<const T>& other) = delete;
        Span(const std::span<T>& other)
            : detail::SpanBase<T>(other.data(), other.size()) { }

        T& operator[](::size index) {
            HK_ABORT_UNLESS(index < mSize, "hk::util::Span<%s>::operator[%zu]: out of range (size: %zu)", getTypeName<T>(), index, mSize);
            return mData[index];
        }

        T& at(::size index) {
            HK_ABORT_UNLESS(index < mSize, "hk::util::Span<%s>::at(%zu): out of range (size: %zu)", getTypeName<T>(), index, mSize);
            return mData[index];
        }

        void set(::size index, const T& value) {
            HK_ABORT_UNLESS(index < mSize, "hk::util::Span<%s>::set(%zu): out of range (size: %zu)", getTypeName<T>(), index, mSize);
            mData[index] = value;
        }

        void set(::size index, T&& value) {
            HK_ABORT_UNLESS(index < mSize, "hk::util::Span<%s>::set(%zu): out of range (size: %zu)", getTypeName<T>(), index, mSize);
            new (&mData[index]) T(value);
        }

        class Iterator {
            T* mCur;

        public:
            Iterator(T* cur)
                : mCur(cur) { }

            Iterator& operator++() {
                mCur++;
                return *this;
            }

            bool operator==(const Iterator& other) const { return mCur == other.mCur; }
            bool operator!=(const Iterator& other) const { return !(*this == other); }

            T& operator*() const { return *mCur; }
            T* operator->() const { return mCur; }
            operator T*() const { return mCur; }
        };

        Iterator begin() { return { mData }; }
        Iterator end() { return { mData + mSize }; }

        T& first() {
            HK_ABORT_UNLESS(!empty(), "hk::util::Span<%s>::first(): empty", getTypeName<T>());
            return mData[0];
        }

        T& last() {
            HK_ABORT_UNLESS(!empty(), "hk::util::Span<%s>::last(): empty", getTypeName<T>());
            return mData[mSize - 1];
        }

        template <typename Callback>
        void forEach(Callback func) {
            for (::size i = 0; i < mSize; i++)
                func((*this)[i]);
        }

        void sort() {
            std::sort(&mData[0], &mData[mSize]);
        }

        template <typename Compare>
        void sort(Compare comp) {
            std::sort(&mData[0], &mData[mSize], comp);
        }

        void move(size dstIdx, size srcIdx, size toMove) {
            if (dstIdx == srcIdx or toMove == 0)
                return;

            if constexpr (std::is_trivially_move_constructible_v<T> and std::is_trivially_destructible_v<T>)
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
        Span() = default;
        Span(const T* data, size size)
            : detail::SpanBase<const T>(data, size) { }

        Span(const Span<T>& other)
            : detail::SpanBase<const T>(other.data(), other.size()) { }
        Span(const Span& other)
            : detail::SpanBase<const T>(other.mData, other.mSize) { }

        Span(const std::span<const T>& other)
            : detail::SpanBase<const T>(other.data(), other.size()) { }
        Span(const std::span<T>& other)
            : detail::SpanBase<const T>(other.data(), other.size()) { }
    };

    template <>
    class Span<void> : public Span<u8> {
    public:
        Span() = default;
        Span(void* data, ::size size)
            : Span<u8>(cast<u8*>(data), size) { }
    };

    template <>
    class Span<const void> : public Span<const u8> {
    public:
        Span() = default;
        Span(const void* data, ::size size)
            : Span<const u8>(cast<const u8*>(data), size) { }
    };

} // namespace hk::util
