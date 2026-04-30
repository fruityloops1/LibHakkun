#pragma once

#include "hk/diag/diag.h"
#include "hk/prim/traits/Integer.h"
#include "hk/prim/traits/Iterator.h"

namespace hk::util {

    namespace detail {

        enum _MultiIteratorEnd { MultiIteratorEnd };

        template <typename To, size Index, typename... Types>
        struct MultiIteratorElement {
            constexpr MultiIteratorElement(Types&&...) { }
            constexpr MultiIteratorElement(_MultiIteratorEnd, Types&&...)
                : mCurIteratorIdx(Index) { }

            SmallestUnsignedInteger<Index> mCurIteratorIdx = 0;

            constexpr void next() { }
            constexpr bool isEqual(const MultiIteratorElement& other) const { return mCurIteratorIdx == other.mCurIteratorIdx; }

            constexpr To& getReference() const {
                HK_ABORT("hk::util::detail::MultiIteratorElement: reached base");
            }

            constexpr To* getPointer() const {
                HK_ABORT("hk::util::detail::MultiIteratorElement: reached base");
            }
        };

        template <typename To, size Index, typename Cur, typename... Rest>
        struct MultiIteratorElement<To, Index, Cur, Rest...> : MultiIteratorElement<To, Index + 1, Rest...> {
            IterableTraits<Cur>::Iterator mCurIter;
            const IterableTraits<Cur>::Iterator mCurIterEnd;

            using Super = MultiIteratorElement<To, Index + 1, Rest...>;

            using Super::mCurIteratorIdx;
            using Super::Super;

            constexpr MultiIteratorElement(Cur&& cur, Rest&&... rest)
                : Super(forward<Rest>(rest)...)
                , mCurIter(cur.begin())
                , mCurIterEnd(cur.end()) {
            }

            constexpr MultiIteratorElement(_MultiIteratorEnd, Cur&& cur, Rest&&... rest)
                : Super(MultiIteratorEnd, forward<Rest>(rest)...)
                , mCurIter(cur.end())
                , mCurIterEnd(cur.end()) { }

            constexpr void next() {
                if (mCurIteratorIdx == Index) {
                    ++mCurIter;
                    if (mCurIter == mCurIterEnd)
                        mCurIteratorIdx++;
                } else
                    Super::next();
            }

            constexpr bool isEqual(const MultiIteratorElement& other) const {
                return static_cast<const Super&>(*this).isEqual(static_cast<const Super&>(other))
                    && mCurIter == mCurIterEnd;
            }

            constexpr To& getReference() const {
                return mCurIteratorIdx == Index ? mCurIter.operator*()
                                                : Super::getReference();
            }

            constexpr To* getPointer() const {
                return mCurIteratorIdx == Index ? mCurIter.operator->()
                                                : Super::getPointer();
            }
        };

    } // namespace detail

    template <typename To, typename... Types>
    struct MultiIterator : public detail::MultiIteratorElement<To, 0, Types...> {
        using Super = detail::MultiIteratorElement<To, 0, Types...>;

    public:
        constexpr MultiIterator(Types&&... containers)
            : Super(forward<Types>(containers)...) { }
        constexpr MultiIterator(detail::_MultiIteratorEnd, Types&&... containers)
            : Super(detail::MultiIteratorEnd, forward<Types>(containers)...) { }

        constexpr MultiIterator& operator++() {
            Super::next();
            return *this;
        }

        constexpr bool operator==(const MultiIterator& other) const { return Super::isEqual(static_cast<const Super&>(other)); }
        constexpr bool operator!=(const MultiIterator& other) const { return !(*this == other); }

        constexpr To& operator*() const { return Super::getReference(); }
        constexpr To* operator->() const { return Super::getPointer(); }

        constexpr operator To*() const { return Super::getPointer(); }
    };

    template <typename To, typename... Types>
    class MultiIteratable {
        using Iterator = MultiIterator<To, Types...>;

        const Iterator mBegin;
        const Iterator mEnd;

    public:
        constexpr MultiIteratable(Types&&... containers)
            : mBegin(forward<Types>(containers)...)
            , mEnd(detail::MultiIteratorEnd, forward<Types>(containers)...) { }

        constexpr Iterator begin() { return mBegin; }
        constexpr Iterator end() { return mEnd; }
        constexpr Iterator begin() const { return mBegin; }
        constexpr Iterator end() const { return mEnd; }
    };

    template <typename... Types>
    MultiIteratable(Types&&... containers) -> MultiIteratable<util::tRemoveReference<decltype(declval<typename IterableTraits<tIndex<0, Types...>>::Iterator>().operator*())>, Types...>;

    template <typename To, typename... Types>
    constexpr MultiIteratable<To, Types...> iterateMultiple(Types&&... containers) {
        return { forward<Types>(containers)... };
    }

} // namespace hk::util
