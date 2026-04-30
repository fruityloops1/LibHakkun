#pragma once

#include "hk/prim/traits/Function.h"
#include "hk/prim/traits/Type.h"
#include "hk/types.h"

namespace hk::util {

    namespace detail {

        template <typename Iterated>
        struct IterableTraitsConst {
            using ConstIterator = decltype(declval<Iterated>().begin());

            static_assert(ctIsSame<ConstIterator, decltype(declval<Iterated>().end())>);
        };

    } // namespace detail

    template <typename Iterated>
    struct IterableTraits : detail::IterableTraitsConst<const Iterated> {
        using Iterator = decltype(declval<Iterated>().begin());

        static_assert(ctIsSame<Iterator, decltype(declval<Iterated>().end())>);
    };

    template <typename Iterated>
    struct IterableTraits<const Iterated> : detail::IterableTraitsConst<const Iterated> {
        using Iterator = detail::IterableTraitsConst<const Iterated>::ConstIterator;
    };

    template <typename Iterator>
    struct IteratorTraits {
        using DerefType = FunctionTraits<decltype(&Iterator::operator*)>::ReturnType;
        using ArrowType = FunctionTraits<decltype(&Iterator::operator->)>::ReturnType;
    };

} // namespace hk::util
