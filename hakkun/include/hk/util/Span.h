#pragma once

#include "hk/Result.h"
#include "hk/diag/diag.h"
#include "hk/types.h"
#include "hk/util/Allocator.h"
#include <span>
#include <type_traits>

namespace hk::util {

    namespace detail {

        template <typename T>
        class SpanBase {
        protected:
            T* mPtr = nullptr;
            size mSize = 0;

        public:
            SpanBase() = default;
            SpanBase(T* ptr, size size)
                : mPtr(ptr)
                , mSize(size) { }

            void set(T* ptr, size size) {
                mPtr = ptr;
                mSize = size;
            }

            T* data() { return mPtr; }
            const T* data() const { return mPtr; }
            size size_bytes() const { return mSize * sizeof(T); }
            size size() const { return mSize; }
            bool empty() const { return mSize == 0; }

            T& operator[](::size index) {
                HK_ABORT_UNLESS(index < mSize, "hk::util::Span<%s>::operator[%zu]: out of range (size: %zu)", index, mSize);
                return mPtr[index];
            }

            const T& operator[](::size index) const {
                HK_ABORT_UNLESS(index < mSize, "hk::util::Span<%s>::operator[%zu]: out of range (size: %zu)", index, mSize);
                return mPtr[index];
            }

            T* begin() { return &mPtr[0]; }
            T* end() { return &mPtr[mSize]; }
            const T* begin() const { return &mPtr[0]; }
            const T* end() const { return &mPtr[mSize]; }
        };

    } // namespace detail

    template <typename T>
    class Span : public detail::SpanBase<T> {
    public:
        Span() = default;
        Span(T* ptr, size size)
            : detail::SpanBase<T>(ptr, size) { }

        Span(const Span<const T>& other) = delete;
        Span(const Span& other)
            : detail::SpanBase<T>(other.mPtr, other.mSize) { }

        Span(const std::span<const T>& other) = delete;
        Span(const std::span<T>& other)
            : detail::SpanBase<T>(other.data(), other.size()) { }
    };

    template <typename T>
    class Span<const T> : public detail::SpanBase<const T> {
    public:
        Span() = default;
        Span(const T* ptr, size size)
            : detail::SpanBase<const T>(ptr, size) { }

        Span(const Span<T>& other)
            : detail::SpanBase<const T>(other.mPtr, other.mSize) { }
        Span(const Span& other)
            : detail::SpanBase<const T>(other.mPtr, other.mSize) { }

        Span(const std::span<const T>& other)
            : detail::SpanBase<const T>(other.data(), other.size()) { }
        Span(const std::span<T>& other)
            : detail::SpanBase<const T>(other.data(), other.size()) { }
    };

} // namespace hk::util
