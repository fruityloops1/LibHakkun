#pragma once

#include "hk/prim/traits/Function.h"
#include "hk/prim/traits/Iterator.h"

namespace hk::util {

    template <typename Iter, typename L>
    class TransformIterator {
        Iter mCur;
        L mTransform;

    public:
        constexpr TransformIterator(const L& transform, const Iter& iter)
            : mCur(iter)
            , mTransform(transform) { }

        constexpr TransformIterator& operator++() {
            ++mCur;
            return *this;
        }

        constexpr FunctionTraits<L>::ReturnType operator*() const { return mTransform(mCur.operator*()); }
        constexpr bool operator==(const TransformIterator& other) const { return mCur == other.mCur; }
        constexpr bool operator!=(const TransformIterator& other) const { return !(*this == other); }
    };

    template <typename Container, typename L>
    class TransformIterable {
        using Iterator = TransformIterator<typename IterableTraits<Container>::Iterator, L>;

        const Iterator mBegin;
        const Iterator mEnd;

    public:
        constexpr TransformIterable(Container&& iterable, const L& func)
            : mBegin(func, iterable.begin())
            , mEnd(func, iterable.end()) { }

        constexpr const Iterator begin() { return mBegin; }
        constexpr const Iterator end() { return mEnd; }

        constexpr const Iterator begin() const { return mBegin; }
        constexpr const Iterator end() const { return mEnd; }
    };

    template <auto L, typename Container>
    constexpr TransformIterable<Container, decltype(L)> iterateTransform(Container&& iterable) {
        return { forward<Container>(iterable), L };
    }

} // namespace hk::util
