#pragma once

#include "hk/container/Span.h"
#include "hk/prim/SpanOperations.h"
#include "hk/types.h"

namespace hk {

    namespace detail {

        template <typename T, size Length>
        class ArrayStorage {
            T mData[Length];

        public:
            hk_alwaysinline constexpr T* getData() { return mData; }
            const hk_alwaysinline constexpr T* getDataConst() const { return mData; }
            hk_alwaysinline constexpr static size getSize() { return Length; }
        };

    } // namespace detail

    template <typename T, size Length>
    class Array : public SpanOperationsCopyable<T, detail::ArrayStorage<T, Length>> {
    public:
        using Super = SpanOperationsCopyable<T, detail::ArrayStorage<T, Length>>;

        using Super::Super;

        friend Super;
        friend Super::Super;
        friend Super::Super::Super;

        static constexpr size cLength = Length;
    };

} // namespace hk
