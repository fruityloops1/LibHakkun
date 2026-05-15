#pragma once

#include "hk/prim/traits/Function.h"
#include "hk/prim/traits/Iterator.h"

namespace hk::util {

    template <typename Iter>
    class IndexIterator {
        Iter mCur;
        size mIndex = 0;

    public:
        constexpr IndexIterator(const Iter& iter)
            : mCur(iter) { }

        constexpr IndexIterator& operator++() {
            ++mCur;
            mIndex++;
            return *this;
        }

        constexpr Tuple<size, typename IteratorTraits<Iter>::DerefType> operator*() const { return { mIndex, mCur.operator*() }; }
        constexpr IteratorTraits<Iter>::ArrowType operator->() const { return mCur.operator->(); }
        constexpr bool operator==(const IndexIterator& other) const { return mCur == other.mCur; }
        constexpr bool operator!=(const IndexIterator& other) const { return !(*this == other); }
    };

    template <typename Container>
    class IndexIterable {
        using Iterator = IndexIterator<typename IterableTraits<Container>::Iterator>;

        const Iterator mBegin;
        const Iterator mEnd;

    public:
        constexpr IndexIterable(Container&& iterable)
            : mBegin(iterable.begin())
            , mEnd(iterable.end()) { }

        constexpr const Iterator begin() { return mBegin; }
        constexpr const Iterator end() { return mEnd; }

        constexpr const Iterator begin() const { return mBegin; }
        constexpr const Iterator end() const { return mEnd; }
    };

    template <typename Container>
    constexpr IndexIterable<Container> iterateWithIdx(Container&& iterable) {
        return { forward<Container>(iterable) };
    }

} // namespace hk::util
