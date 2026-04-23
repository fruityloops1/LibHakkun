#pragma once

#include "hk/prim/traits/Type.h"
#include "hk/util/Algorithm.h"
#include "hk/util/TypeName.h"

#define INCLUDE_HK_DETAIL_MACROS
#include "hk/detail/macros.h"

namespace hk {

    template <typename... Types>
    struct OneOf;

    namespace detail {

        template <typename T>
        struct OneOfTypeEnum { };

    } // namespace detail

    template <typename T>
    constexpr static detail::OneOfTypeEnum<T> OneOfType = detail::OneOfTypeEnum<T>();
    constexpr static detail::OneOfTypeEnum<void> OneOfNone = OneOfType<void>;

    namespace detail {

        template <typename... Types>
        union OneOfElement { };

        template <typename First, typename... Rest>
        union OneOfElement<First, Rest...> {
            using Type = First;

            First first;
            OneOfElement<Rest...> rest;
        };

        template <size Index, typename T, typename... Types>
        hk_alwaysinline constexpr void constructAt(OneOfElement<Types...>* elem, T&& value) {
            if constexpr (Index == 0)
                construct_at(&elem->first, value);
            else {
                construct_at(&elem->rest);
                constructAt<Index - 1>(&elem->rest, value);
            }
        }

        template <size Index, typename T, typename... Types>
        hk_alwaysinline constexpr void constructAt(OneOfElement<Types...>* elem, const T& value) {
            if constexpr (Index == 0)
                construct_at(&elem->first, value);
            else {
                construct_at(&elem->rest);
                constructAt<Index - 1>(&elem->rest, value);
            }
        }

        template <size Index, typename... Types>
        hk_alwaysinline constexpr auto* grabPtr(OneOfElement<Types...>* value) {
            if constexpr (Index == 0)
                return &value->first;
            else
                return grabPtr<Index - 1>(&value->rest);
        }

        template <size Index, typename... Types>
        const hk_alwaysinline constexpr auto* grabPtr(const OneOfElement<Types...>* value) {
            if constexpr (Index == 0)
                return &value->first;
            else
                return grabPtr<Index - 1>(&value->rest);
        }

        template <typename Top, typename Parent, size Index, typename... Types>
        struct OneOfOperations {
            template <typename>
                requires false
            constexpr void construct() { }

            constexpr void constructDefaultByIndex() { }
            constexpr void destroyByIndex() { }
            constexpr void copyConstructByIndex(Parent* to) const { }
            constexpr void moveConstructByIndex(Parent* to) { }
            constexpr const char* getTypeNameByIndex() const { return "None"; }
            template <typename L>
            constexpr void map(L&& func) { }
            template <typename L>
            constexpr void map(L&& func) const { }
        };

        template <typename Top, typename Parent, size Index, typename First, typename... Rest>
        struct OneOfOperations<Top, Parent, Index, First, Rest...> : OneOfOperations<Top, Parent, Index + 1, Rest...> {
            using Mine = First;
            using Super = OneOfOperations<Top, Parent, Index + 1, Rest...>;

            constexpr OneOfOperations() = default;

            using Super::construct;
            constexpr void construct(const Mine& value) {
                destroy();
                static_cast<Top*>(this)->template constructAt<Index>(value);
                setIndex(Index);
            }

            constexpr void construct(Mine&& value) {
                destroy();
                static_cast<Top*>(this)->template constructAt<Index>(forward<Mine>(value));
                setIndex(Index);
            }

            constexpr operator Mine&() {
                checkMe();
                return *grabMine();
            }

            constexpr operator const Mine&() const {
                checkMe();
                return *grabMine();
            }

            constexpr operator Mine*() { return isMe() ? grabMine() : nullptr; }
            constexpr operator const Mine*() const { return isMe() ? grabMine() : nullptr; }

            template <typename = Mine>
                requires util::ctIsTriviallyDefaultConstructible<Mine>
            hk_alwaysinline constexpr void constructDefaultByIndex() {
                if (isMe())
                    static_cast<Top*>(this)->template constructAt<Index>(Mine());
                else
                    Super::constructDefaultByIndex();
            }

            hk_alwaysinline constexpr void destroyByIndex() {
                if (isMe())
                    grabMine()->~Mine();
                else
                    Super::destroyByIndex();
            }

            hk_alwaysinline constexpr void copyConstructByIndex(Parent* to) const {
                if (isMe())
                    static_cast<Top*>(to)->template constructAt<Index>(*grabMine());
                else
                    Super::copyConstructByIndex(to);
            }

            hk_alwaysinline constexpr void moveConstructByIndex(Parent* to) {
                if (isMe())
                    static_cast<Top*>(to)->template constructAt<Index>(forward<Mine>(*grabMine()));
                else
                    Super::moveConstructByIndex(to);
            }

            const hk_alwaysinline constexpr char* getTypeNameByIndex() const {
                if (isMe())
                    return util::getTypeName<Mine>();
                else
                    return Super::getTypeNameByIndex();
            }

            template <typename L>
            hk_alwaysinline constexpr void map(L&& func) {
                if (isMe())
                    func(*grabMine());
                else
                    Super::map(func);
            }

            template <typename L>
            hk_alwaysinline constexpr void map(L&& func) const {
                if (isMe())
                    func(*grabMine());
                else
                    Super::map(func);
            }

        private:
            constexpr void checkMe();

            constexpr void destroy() { static_cast<Top*>(this)->destroy(); }
            constexpr size toIndex() const { return static_cast<const Top*>(this)->toIndex(); }
            constexpr void setIndex(size index) { static_cast<Top*>(this)->setIndex(index); }
            constexpr Mine* grabMine() { return static_cast<Top*>(this)->template grabPtr<Index>(); }
            constexpr const Mine* grabMine() const { return static_cast<const Top*>(this)->template grabPtr<Index>(); }
            hk_alwaysinline constexpr bool isMe() const { return toIndex() == Index; }
        };

        template <typename Parent, typename... Types>
        struct OneOfOperationsTop : OneOfOperations<OneOfOperationsTop<Parent, Types...>, Parent, 0, Types...> {
            using Super = OneOfOperations<OneOfOperationsTop<Parent, Types...>, Parent, 0, Types...>;

            // up
            constexpr void destroy() { static_cast<Parent*>(this)->destroy(); }
            constexpr void setIndex(size index) { static_cast<Parent*>(this)->setIndex(index); }
            constexpr size toIndex() const { return static_cast<const Parent*>(this)->toIndex(); }
            template <size Index>
            constexpr auto* grabPtr() { return static_cast<Parent*>(this)->template grabPtr<Index>(); }
            template <size Index>
            constexpr const auto* grabPtr() const { return static_cast<const Parent*>(this)->template grabPtr<Index>(); }
            template <size Index, typename T>
            constexpr void constructAt(T&& value) { return static_cast<Parent*>(this)->template constructAt<Index>(value); }
            template <size Index, typename T>
            constexpr void constructAt(const T& value) { return static_cast<Parent*>(this)->template constructAt<Index>(value); }

            // down
            const char* getCurrentTypeName() const { return Super::getTypeNameByIndex(); }
        };

    } // namespace detail

    template <typename... Types>
    class OneOf : public detail::OneOfOperationsTop<OneOf<Types...>, Types...> {
        using Super = detail::OneOfOperationsTop<OneOf<Types...>, Types...>;
        using TypeEnumUnderlying = SmallestUnsignedInteger<1 + util::ctAmount<Types...>>;

        constexpr static bool cTriviallyDefaultConstructible = util::ctAllSatisfied<util::ttTriviallyDefaultConstructible, Types...>;

        class TypeEnum {
            TypeEnumUnderlying mValue;

        public:
            constexpr TypeEnum(TypeEnumUnderlying value)
                : mValue(value) { }

            constexpr operator int() const { return mValue; }
        } __attribute__((packed));

        enum {
            TypeEnum_None = 0,
            TypeEnum_Start = 1
        };

        detail::OneOfElement<Types...> mValue;
        TypeEnum mType = TypeEnum_None;

        constexpr size toIndex() const { return size(mType - TypeEnum_Start); }
        constexpr void setIndex(size index) { mType = TypeEnum(TypeEnum_Start + index); }

        using typename Super::Mine;

        friend Super;
        using Super::constructDefaultByIndex;
        using Super::copyConstructByIndex;
        using Super::destroyByIndex;
        using Super::getTypeNameByIndex;
        using Super::map;
        using Super::moveConstructByIndex;

        template <size Index>
        constexpr auto* grabPtr() { return detail::grabPtr<Index>(&mValue); }
        template <size Index>
        constexpr auto* grabPtr() const { return detail::grabPtr<Index>(&mValue); }
        template <size Index, typename T>
        constexpr void constructAt(T&& value) { return detail::constructAt<Index>(&mValue, value); }
        template <size Index, typename T>
        constexpr void constructAt(const T& value) { return detail::constructAt<Index>(&mValue, value); }

    public:
        constexpr OneOf()
            : OneOf(OneOfNone) { }

        template <typename T>
            requires(util::ctContains<T, Types...>
                        and cTriviallyDefaultConstructible)
            or util::ctIsSame<T, void>
        constexpr OneOf(detail::OneOfTypeEnum<T>) {
            Super::construct(T());
        }

        template <>
        constexpr OneOf(detail::OneOfTypeEnum<void>) { }

        template <typename = void>
            requires cTriviallyDefaultConstructible
        constexpr OneOf(TypeEnum type)
            : mType(type) {
            Super::constructDefaultByIndex();
        }

        template <typename T>
            requires util::ctContains<T, Types...>
        constexpr OneOf(const T& value) {
            Super::construct(value);
        }

        template <typename T>
            requires util::ctContains<T, Types...>
        constexpr OneOf(T&& value) {
            Super::construct(forward<T>(value));
        }

        template <typename T>
            requires util::ctContains<T*, Types...>
        constexpr OneOf(T value[]) {
            Super::construct((T*)value);
        }

        constexpr OneOf(const OneOf& other)
            : mType(other.mType) {
            other.copyConstructByIndex(this);
        }

        constexpr OneOf& operator=(const OneOf& other) {
            mType = other.type();
            other.copyConstructByIndex(this);
            return *this;
        }

        constexpr OneOf(OneOf&& old)
            : mType(old.mType) {
            old.moveConstructByIndex(this);
            old.destroy();
        }

        constexpr OneOf& operator=(OneOf&& other) {
            mType = other.type();
            other.moveConstructByIndex(this);
            other.destroy();
            return *this;
        }

        constexpr ~OneOf() {
            destroy();
        }

        constexpr static TypeEnum None = TypeEnum_None;
        template <typename T>
            requires util::ctContains<T, Types...> or util::ctIsSame<T, void>
        constexpr static TypeEnum Type = TypeEnum(int(TypeEnum_Start) + util::ctIndexOf<T, Types...>);
        template <>
        constexpr TypeEnum Type<void> = None;

        constexpr TypeEnum type() const { return mType; }
        constexpr bool hasValue() const { return type() != TypeEnum_None; }

        constexpr void destroy() {
            bool has = hasValue();
            mType = TypeEnum_None;
            if (has)
                Super::destroyByIndex();
        }

        template <typename T>
            requires util::ctContains<T, Types...>
        constexpr T* to() {
            return Super::operator T*();
        }

        template <typename T>
            requires util::ctContains<T, Types...>
        constexpr const T* to() const {
            return Super::operator const T*();
        }

        template <typename T>
            requires util::ctContains<T, Types...>
        constexpr T& get() {
            return Super::operator T&();
        }

        template <typename T>
            requires util::ctContains<T, Types...>
        constexpr const T& get() const {
            return Super::operator const T&();
        }

        template <typename T>
            requires util::ctContains<T, Types...>
        constexpr bool is() const {
            if constexpr (util::ctIsSame<T, void>)
                return hasValue();
            else
                return toIndex() == util::ctIndexOf<T, Types...>;
        }

        template <typename L>
        constexpr void map(L&& func) {
            Super::map(func);
        }

        template <typename L>
        constexpr void map(L&& func) const {
            Super::map(func);
        }

        template <typename T>
        constexpr bool operator==(detail::OneOfTypeEnum<T>) const {
            return is<T>();
        }

        constexpr bool operator==(TypeEnum type) const { return mType == type; }
    };

} // namespace hk

#define INCLUDE_HK_DETAIL_PLATFORM
#include "hk/detail/platform.h"

#if !HK_RESULT_ADVANCED
#define INCLUDE_HK_CONTAINER_DETAIL_ONEOF
#include "hk/container/detail/OneOf.ih"
#endif
