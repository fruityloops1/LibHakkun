#pragma once

#include "hk/prim/traits/Iterator.h"

namespace hk::util {

    template <typename Iter, typename L>
    class FilterIterator {
        Iter mCur;
        const Iter mEnd;
        L mFilter;

    public:
        constexpr FilterIterator(const L& filter, const Iter& begin, const Iter& end)
            : mCur(begin)
            , mEnd(end)
            , mFilter(filter) {
            while (mCur != mEnd && mFilter(mCur.operator*()) == false)
                ++mCur;
        }

        constexpr FilterIterator& operator++() {
            ++mCur;
            while (mCur != mEnd && mFilter(mCur.operator*()) == false)
                ++mCur;
            return *this;
        }

        constexpr IteratorTraits<Iter>::DerefType operator*() const { return mCur.operator*(); }
        constexpr IteratorTraits<Iter>::ArrowType operator->() const { return mCur.operator->(); }
        constexpr bool operator==(const FilterIterator& other) const { return mCur == other.mCur; }
        constexpr bool operator!=(const FilterIterator& other) const { return !(*this == other); }
    };

    template <typename Container, typename L>
    class FilterIterable {
        using Iterator = FilterIterator<typename IterableTraits<Container>::Iterator, L>;

        const Iterator mBegin;
        const Iterator mEnd;

    public:
        constexpr FilterIterable(Container&& iterable, const L& func)
            : mBegin(func, iterable.begin(), iterable.end())
            , mEnd(func, iterable.end(), iterable.end()) { }

        constexpr const Iterator begin() { return mBegin; }
        constexpr const Iterator end() { return mEnd; }

        constexpr const Iterator begin() const { return mBegin; }
        constexpr const Iterator end() const { return mEnd; }
    };

    template <auto L, typename Container>
    constexpr FilterIterable<Container, decltype(L)> iterateFilter(Container&& iterable) {
        return { forward<Container>(iterable), L };
    }

    namespace detail {

        constexpr auto cFilterNullptr = [](auto* ptr) { return ptr != nullptr; };

    } // namespace detail

    template <typename Container>
    constexpr FilterIterable<Container, decltype(detail::cFilterNullptr)> iterateNonNullptr(Container&& iterable) {
        return { forward<Container>(iterable), detail::cFilterNullptr };
    }

} // namespace hk::util
