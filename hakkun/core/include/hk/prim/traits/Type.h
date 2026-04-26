#pragma once

namespace hk::util {

    template <typename... Types>
    struct TypeTraits;

    template <typename T>
    struct TypeTraits<T> {
        template <bool Value>
        struct Bool {
            static constexpr bool cValue = Value;
        };

        using True = Bool<true>;
        using False = Bool<false>;

        template <typename C>
        struct Same : False { };
        template <>
        struct Same<T> : True { };

        template <typename Derived>
        struct BaseOf : Bool<__is_base_of(T, Derived)> { };

        struct Reference;
        struct Pointer : Bool<__is_pointer(T)> { };
        struct Const : Bool<__is_const(T)> { };

        template <typename... Args>
        struct Constructible : Bool<__is_constructible(T, Args...)> { };
        struct DefaultConstructible : Constructible<> { };
        struct CopyConstructible : Constructible<const T&> { };
        struct MoveConstructible : Constructible<T&&> { };
        struct Destructible : Bool<__is_destructible(T)> { };

        template <typename... Args>
        struct TriviallyConstructible : Bool<__is_trivially_constructible(T, Args...)> { };
        struct TriviallyDefaultConstructible : TriviallyConstructible<> { };
        struct TriviallyMoveConstructible : TriviallyConstructible<T&&> { };
        struct TriviallyCopyConstructible : TriviallyConstructible<const T&> { };
        struct TriviallyDestructible : Bool<__is_trivially_destructible(T)> { };

        struct Integral;
        struct FloatingPoint;

        template <typename _Tp>
        struct _Type {
            using Type = _Tp;
        };

        struct RemoveReference;
        struct RemovePointer : _Type<__remove_pointer(T)> { };
        struct RemoveConst : _Type<__remove_const(T)> { };

        struct PrintfFormatVerbose;
    };

    template <typename T, typename C>
    using ttSame = TypeTraits<T>::template Same<C>;

    template <typename Base, typename Derived>
    using ttBaseOf = TypeTraits<Base>::template BaseOf<Derived>;

    template <typename T>
    using ttReference = TypeTraits<T>::Reference;
    template <typename T>
    using ttPointer = TypeTraits<T>::Pointer;
    template <typename T>
    using ttConst = TypeTraits<T>::Const;

    template <typename T, typename... Args>
    using ttConstructible = TypeTraits<T>::template Constructible<Args...>;
    template <typename T>
    using ttDefaultConstructible = TypeTraits<T>::DefaultConstructible;
    template <typename T>
    using ttCopyConstructible = TypeTraits<T>::CopyConstructible;
    template <typename T>
    using ttMoveConstructible = TypeTraits<T>::MoveConstructible;
    template <typename T>
    using ttDestructible = TypeTraits<T>::Destructible;

    template <typename T, typename... Args>
    using ttTriviallyConstructible = TypeTraits<T>::template TriviallyConstructible<Args...>;
    template <typename T>
    using ttTriviallyDefaultConstructible = TypeTraits<T>::TriviallyDefaultConstructible;
    template <typename T>
    using ttTriviallyCopyConstructible = TypeTraits<T>::TriviallyCopyConstructible;
    template <typename T>
    using ttTriviallyMoveConstructible = TypeTraits<T>::TriviallyMoveConstructible;
    template <typename T>
    using ttTriviallyDestructible = TypeTraits<T>::TriviallyDestructible;

    template <typename T>
    using ttIntegral = TypeTraits<T>::Integral;
    template <typename T>
    using ttFloatingPoint = TypeTraits<T>::FloatingPoint;

    template <typename T, typename C>
    constexpr bool ctIsSame = ttSame<T, C>::cValue;

    template <typename Base, typename Derived>
    constexpr bool ctIsBaseOf = ttBaseOf<Base, Derived>::cValue;

    template <typename T>
    constexpr bool ctIsReference = ttReference<T>::cValue;
    template <typename T>
    constexpr bool ctIsPointer = ttPointer<T>::cValue;
    template <typename T>
    constexpr bool ctIsConst = ttConst<T>::cValue;

    template <typename T, typename... Args>
    constexpr bool ctIsConstructible = ttConstructible<T, Args...>::cValue;
    template <typename T>
    constexpr bool ctIsDefaultConstructible = ttDefaultConstructible<T>::cValue;
    template <typename T>
    constexpr bool ctIsCopyConstructible = ttCopyConstructible<T>::cValue;
    template <typename T>
    constexpr bool ctIsMoveConstructible = ttMoveConstructible<T>::cValue;
    template <typename T>
    constexpr bool ctIsDestructible = ttDestructible<T>::cValue;

    template <typename T, typename... Args>
    constexpr bool ctIsTriviallyConstructible = ttTriviallyConstructible<T, Args...>::cValue;
    template <typename T>
    constexpr bool ctIsTriviallyDefaultConstructible = ttTriviallyDefaultConstructible<T>::cValue;
    template <typename T>
    constexpr bool ctIsTriviallyCopyConstructible = ttTriviallyCopyConstructible<T>::cValue;
    template <typename T>
    constexpr bool ctIsTriviallyMoveConstructible = ttTriviallyMoveConstructible<T>::cValue;
    template <typename T>
    constexpr bool ctIsTriviallyDestructible = ttTriviallyDestructible<T>::cValue;

    template <typename T>
    constexpr bool ctIsIntegral = ttIntegral<T>::cValue;
    template <typename T>
    constexpr bool ctIsFloatingPoint = ttFloatingPoint<T>::cValue;

    template <typename T>
    using tRemoveReference = TypeTraits<T>::RemoveReference::Type;
    template <typename T>
    using tRemovePointer = TypeTraits<T>::RemovePointer::Type;
    template <typename T>
    using tRemoveConst = TypeTraits<T>::RemoveConst::Type;

    template <typename T>
    constexpr const char* ctPrintfFormatVerbose = TypeTraits<T>::PrintfFormatVerbose::cValue;

    template <typename... Types>
    struct TypeTraits {
        template <template <typename> typename Trait>
        static constexpr bool all() {
            bool satisfied = true;
            ([&] {
                using Cur = Types;

                satisfied &= Trait<Cur>::cValue;
            }(),
                ...);

            return satisfied;
        }

        template <typename T>
        static constexpr int findIndex() {
            int index = 0;
            int foundIdx = -1;
            ([&] {
                using Cur = Types;
                if (ctIsSame<T, Cur>)
                    foundIdx = index;
                index++;
            }(),
                ...);
            return foundIdx;
        }

        template <typename T>
        static constexpr bool contains() {
            bool found = false;
            ([&] {
                using Cur = Types;
                if (ctIsSame<T, Cur>)
                    found = true;
            }(),
                ...);

            return found;
        }

        static constexpr bool amount() {
            int amount = 0;
            ([&] {
                using Cur = Types;
                amount++;
            }(),
                ...);

            return amount;
        }
    };

    namespace detail {

        template <int Index, typename... Types>
        struct TypeIndexer;

        template <int Index, typename T>
        struct TypeIndexer<Index, T> {
            using Type = T;
        };

        template <int Index, typename First, typename... Rest>
        struct TypeIndexer<Index, First, Rest...> {
            using Type = typename TypeIndexer<Index - 1, Rest...>::Type;
        };

    } // namespace detail

    template <int Index, typename... In>
    using tIndex = detail::TypeIndexer<Index, In...>::Type;

    template <typename Of, typename... In>
    constexpr static int ctIndexOf = TypeTraits<In...>::template findIndex<Of>();

    template <typename T, typename... In>
    constexpr static bool ctContains = TypeTraits<In...>::template contains<T>();

    template <typename... Types>
    constexpr static int ctAmount = TypeTraits<Types...>::amount();

    template <template <typename> typename Trait, typename... Types>
    constexpr static bool ctAllSatisfied = TypeTraits<Types...>::template all<Trait>();

    namespace detail {

        template <typename T>
        struct Reference {
            constexpr static bool cValue = false;
        };

        template <typename T>
        struct Reference<T&> {
            constexpr static bool cValue = true;
        };

        template <typename T>
        struct Reference<T&&> {
            constexpr static bool cValue = true;
        };

    } // namespace detail

    template <typename T>
    struct TypeTraits<T>::Reference : detail::Reference<T> { };

    template <typename T>
    struct TypeTraits<T>::Integral : Bool<ctContains<T,
                                         bool,
                                         char,
                                         signed char,
                                         unsigned char,
                                         short,
                                         unsigned short,
                                         int,
                                         unsigned int,
                                         long,
                                         unsigned long,
                                         long long,
                                         unsigned long long>> { };

    template <typename T>
    struct TypeTraits<T>::FloatingPoint : Bool<ctContains<T,
                                              float,
                                              double,
                                              long double>> { };

    namespace detail {

        template <typename T>
        struct RemoveReference {
            using Type = T;
        };

        template <typename T>
        struct RemoveReference<T&> {
            using Type = T;
        };

        template <typename T>
        struct RemoveReference<T&&> {
            using Type = T;
        };

    } // namespace detail

    template <typename T>
    struct TypeTraits<T>::RemoveReference : detail::RemoveReference<T> { };

    namespace detail {

        template <typename T>
        struct PrintfFormatVerbose;

        template <>
        struct PrintfFormatVerbose<float> {
            static constexpr const char cValue[] = "f32: %f";
        };

        template <>
        struct PrintfFormatVerbose<double> {
            static constexpr const char cValue[] = "f64: %lf";
        };

        template <>
        struct PrintfFormatVerbose<char*> {
            static constexpr const char cValue[] = "\"%s\"";
        };
        template <>
        struct PrintfFormatVerbose<const char*> {
            static constexpr const char cValue[] = "\"%s\"";
        };

        template <>
        struct PrintfFormatVerbose<void*> {
            static constexpr const char cValue[] = "ptr: %p";
        };
        template <>
        struct PrintfFormatVerbose<const void*> {
            static constexpr const char cValue[] = "ptr: %p";
        };

    } // namespace detail

    template <typename T>
    struct TypeTraits<T>::PrintfFormatVerbose : detail::PrintfFormatVerbose<tRemoveConst<T>> { };

} // namespace hk::util
