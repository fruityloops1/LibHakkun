#pragma once

#include "hk/container/Span.h"
#include "hk/prim/SpanOperations.h"
#include "hk/types.h"

namespace hk {

    namespace detail {

        template <typename T, size Length>
        class ArrayStorage {
            T mData[Length] { };

        public:
            hk_alwaysinline constexpr T* getData() { return mData; }
            const hk_alwaysinline constexpr T* getDataConst() const { return mData; }
            hk_alwaysinline constexpr static size getSize() { return Length; }

            static constexpr char cTypeName[] = "hk::Array";
        };

    } // namespace detail

    template <typename T, size Length>
    class Array : public SpanOperationsCopyable<T, detail::ArrayStorage<T, Length>> {
    public:
        using Super = SpanOperationsCopyable<T, detail::ArrayStorage<T, Length>>;

        using Super::Super;

        template <typename = void>
            requires(Length == 1)
        constexpr Array(const T& value)
            : Super(&value, 1) { }

        friend Super;
        friend Super::Super;
        friend Super::Super::Super;

        static constexpr size cLength = Length;
    };

    template <typename T, size N>
    Array(T (&data)[N]) -> Array<T, N>;
    template <typename T, size N>
    Array(const T (&data)[N]) -> Array<T, N>;
    template <typename T>
    Array(const T& value) -> Array<T, 1>;
    template <typename T, size N>
    Span(const Array<T, N>&) -> Span<T>;

} // namespace hk
