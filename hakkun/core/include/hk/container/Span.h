#pragma once

#include "hk/prim/SpanOperations.h"

namespace hk {

    namespace detail {

        template <typename T>
        class SpanStorage : util::CustomTypeName {
            T* mData = nullptr;
            size mSize = 0;

        public:
            hk_alwaysinline constexpr void setData(T* data) { mData = data; }
            hk_alwaysinline constexpr T* getData() const { return mData; }
            const hk_alwaysinline constexpr T* getDataConst() const { return mData; }
            hk_alwaysinline constexpr void setSize(size size) { mSize = size; }
            hk_alwaysinline constexpr size getSize() const { return mSize; }

            static constexpr const char cTypeName[] = "hk::Span";
        };

    } // namespace detail

    template <typename T>
    class Span : public SpanOperationsWithBufferPointer<T, detail::SpanStorage<T>> {

    public:
        using Super = SpanOperationsWithBufferPointer<T, detail::SpanStorage<T>>;

        using Super::Super;

        friend Super;
        friend Super::Super;
        friend Super::Super::Super;
        friend Super::Super::Super::Super;
    };

    HK_SPANOPERATIONSWITHBUFFERPOINTER_DEDUCTION_GUIDE(Span, Span<T>);

    template <>
    class Span<void> : public Span<u8> {
    public:
        constexpr Span() = default;
        Span(void* data, ::size size)
            : Span<u8>(cast<u8*>(data), size) { }
    };

    template <>
    class Span<const void> : public Span<const u8> {
    public:
        constexpr Span() = default;
        Span(const void* data, ::size size)
            : Span<const u8>(cast<const u8*>(data), size) { }
    };

    static_assert(([]() consteval -> bool {
        u8 arr[128] { 4, 3, 2, 1 };

        std::span<u8> stl(arr);
        Span<u8> mut(arr);
        { Span<const u8> view(arr); }
        { Span<u8> view(stl); }
        { Span<const u8> view(stl); }

        mut.fill(64, 64, 42);
        mut.sort();

        HK_ASSERT(mut.first() == 0 && mut.last() == 42);

        Span<const u8> view(mut);
        mut.fill(132);

        return view[42] == 132;
    })());

} // namespace hk

template <typename To, typename From>
hk_alwaysinline hk::Span<To> cast(hk::Span<From> value) {
    return { cast<To*>(value.data()), value.size_bytes() / sizeof(To) };
}
