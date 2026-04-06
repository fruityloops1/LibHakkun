#pragma once

#include "hk/diag/diag.h"
#include "hk/types.h"
#include "hk/util/Algorithm.h"
#include <span>

namespace hk::util {

    namespace detail {

        template <typename T>
        class SpanBase {
        protected:
            T* mData = nullptr;
            size mSize = 0;

            template <bool IsVoid = std::is_void_v<T>>
            struct IndexOperator_;

            template <>
            struct IndexOperator_<true> {
                using ReturnType = T*;

                static ReturnType index(const SpanBase* thiz, ::size idx) { return const_cast<ReturnType>(&thiz->mData[idx]); }
            };

            template <>
            struct IndexOperator_<false> {
                using ReturnType = T&;

                static ReturnType index(const SpanBase* thiz, ::size idx) { return const_cast<ReturnType>(thiz->mData[idx]); }
            };

            using IndexOperator = IndexOperator_<>;
            friend IndexOperator;

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

            const IndexOperator::ReturnType operator[](::size index) const {
                HK_ABORT_UNLESS(index < mSize, "hk::util::Span<%s>::operator[%zu]: out of range (size: %zu)", getTypeName<T>(), index, mSize);
                return IndexOperator::index(this, index);
            }

            const T* begin() const { return &mData[0]; }
            const T* end() const { return &mData[mSize]; }

            template <typename ST, typename GetFunc>
            ::size binarySearch(GetFunc getValue, ST searchValue, bool findBetween = false) const {
                return util::binarySearch([this, getValue](s32 index) {
                    const T* value = mData[index];
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

        Span() = default;
        Span(T* data, size size)
            : detail::SpanBase<T>(data, size) { }

        Span(const Span<const T>& other) = delete;
        Span(const Span& other)
            : detail::SpanBase<T>(other.mData, other.mSize) { }

        Span(const std::span<const T>& other) = delete;
        Span(const std::span<T>& other)
            : detail::SpanBase<T>(other.data(), other.size()) { }

        detail::SpanBase<T>::IndexOperator::ReturnType operator[](::size index) {
            HK_ABORT_UNLESS(index < mSize, "hk::util::Span<%s>::operator[%zu]: out of range (size: %zu)", getTypeName<T>(), index, mSize);
            return detail::SpanBase<T>::IndexOperator::index(this, index);
        }

        T* begin() { return &mData[0]; }
        T* end() { return &mData[mSize]; }

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

} // namespace hk::util
