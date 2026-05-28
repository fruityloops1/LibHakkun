#pragma once

template <typename T>
constexpr T&& declval();

namespace hk {

    // TODO: make all the traits concepts

    template <typename T>
    concept AddableType = requires(T v) { v + v; };
    template <typename A, typename B>
    concept AddableTypes = requires(A a, B b) { a + b; };

    template <typename T>
    concept SubtractibleType = requires(T v) { v - v; };
    template <typename A, typename B>
    concept SubtractibleTypes = requires(A a, B b) { a - b; };

    template <typename T>
    concept MultiplicableType = requires(T v) { v * v; };
    template <typename A, typename B>
    concept MultiplicableTypes = requires(A a, B b) { a * b; };

    template <typename T>
    concept DivisibleType = requires(T v) { v / v; };
    template <typename A, typename B>
    concept DivisibleTypes = requires(A a, B b) { a / b; };

    template <typename L>
    concept LambdaType = requires() { &L::operator(); };

    template <typename L>
    concept LambdaDeduceThisType = requires() { &L::template operator()<L>; };

    template <typename L>
    concept LambdaNoCaptureType = LambdaType<L> and sizeof(L) == 1;

    template <typename L>
    concept LambdaNoCaptureDeduceThisType = LambdaDeduceThisType<L> and sizeof(L) == 1;

    template <typename T>
    concept IteratorType = requires(T it) { *it; ++it; };

    template <typename T>
    concept IterableType = requires(T container) { { container.begin() } -> IteratorType; { container.end() } -> IteratorType; };

    struct NonType;

} // namespace hk

namespace hk::util {

    template <typename... Types>
    struct TypeTraits;

    template <typename... Types>
    struct TypeTraitsVariadic;

    template <typename T>
    struct TypeTraits<T> : TypeTraitsVariadic<T> {
        template <bool Value>
        struct Bool {
            static constexpr bool cValue = Value;
        };

        using True = Bool<true>;
        using False = Bool<false>;

        template <typename C>
        struct Same;

        template <typename Derived>
        struct BaseOf : Bool<__is_base_of(T, Derived)> { };

        struct Reference;
        struct Pointer : Bool<__is_pointer(T)> { };
        struct Const : Bool<__is_const(T)> { };

        template <typename Rhs = NonType>
        struct Addable;
        template <typename Rhs = NonType>
        struct Subtractible;
        template <typename Rhs = NonType>
        struct Multiplicable;
        template <typename Rhs = NonType>
        struct Divisible;

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

        struct Polymorphic : Bool<__is_polymorphic(T)> { };

        struct Function : Bool<__is_function(T)> { };
        struct FunctionPointer;
        struct MemberFunctionPointer : Bool<__is_member_function_pointer(T)> { };
        struct Lambda : Bool<LambdaType<T>> { };
        struct LambdaDeduceThis : Bool<LambdaDeduceThisType<T>> { };
        struct LambdaCapture : Bool<LambdaType<T> and !(LambdaNoCaptureType<T> or LambdaNoCaptureDeduceThisType<T>)> { };

        struct Integral;
        struct FloatingPoint;

        template <typename _Tp>
        struct _Type {
            using Type = _Tp;
        };

        struct RemoveReference;
        struct RemovePointer;
        struct RemoveConst;
        struct RemoveQualifiers : TypeTraits<typename TypeTraits<typename RemovePointer::Type>::RemoveConst::Type>::RemoveReference { };

        struct Sum;
        struct Difference;
        struct Product;
        struct Quotient;

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

    template <typename A, typename B = NonType>
    using ttAddable = TypeTraits<A>::template Addable<B>;
    template <typename A, typename B = NonType>
    using ttSubtractible = TypeTraits<A>::template Subtractible<B>;
    template <typename A, typename B = NonType>
    using ttMultiplicable = TypeTraits<A>::template Multiplicable<B>;
    template <typename A, typename B = NonType>
    using ttDivisible = TypeTraits<A>::template Divisible<B>;

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
    using ttPolymorphic = TypeTraits<T>::Polymorphic;

    template <typename T>
    using ttFunction = TypeTraits<T>::Function;
    template <typename T>
    using ttFunctionPointer = TypeTraits<T>::FunctionPointer;
    template <typename T>
    using ttMemberFunctionPointer = TypeTraits<T>::MemberFunctionPointer;
    template <typename T>
    using ttLambda = TypeTraits<T>::Lambda;
    template <typename T>
    using ttLambdaDeduceThis = TypeTraits<T>::LambdaDeduceThis;
    template <typename T>
    using ttLambdaCapture = TypeTraits<T>::LambdaCapture;

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

    template <typename A, typename B = NonType>
    constexpr bool ctIsAddable = ttAddable<A, B>::cValue;
    template <typename A, typename B = NonType>
    constexpr bool ctIsSubtractible = ttSubtractible<A, B>::cValue;
    template <typename A, typename B = NonType>
    constexpr bool ctIsMultiplicable = ttMultiplicable<A, B>::cValue;
    template <typename A, typename B = NonType>
    constexpr bool ctIsDivisible = ttDivisible<A, B>::cValue;

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
    constexpr bool ctIsPolymorphic = ttPolymorphic<T>::cValue;

    template <typename T>
    constexpr bool ctIsFunction = ttFunction<T>::cValue;
    template <typename T>
    constexpr bool ctIsFunctionPointer = ttFunctionPointer<T>::cValue;
    template <typename T>
    constexpr bool ctIsMemberFunctionPointer = ttMemberFunctionPointer<T>::cValue;
    template <typename T>
    constexpr bool ctIsLambda = ttLambda<T>::cValue;
    template <typename T>
    constexpr bool ctIsLambdaDeduceThis = ttLambdaDeduceThis<T>::cValue;
    template <typename T>
    constexpr bool ctLambdaHasCapture = ttLambdaCapture<T>::cValue;

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
    using tRemoveQualifiers = TypeTraits<T>::RemoveQualifiers::Type;

    template <typename T>
    using tSum = TypeTraits<T>::Sum::Type;
    template <typename T>
    using tDifference = TypeTraits<T>::Difference::Type;
    template <typename T>
    using tProduct = TypeTraits<T>::Product::Type;
    template <typename T>
    using tQuotient = TypeTraits<T>::Quotient::Type;

    template <typename T>
    constexpr const char* ctPrintfFormatVerbose = TypeTraits<T>::PrintfFormatVerbose::cValue;

    template <typename... Types>
    struct TypeTraitsVariadic {
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

        template <typename Base>
        static constexpr bool containsDerived() {
            bool found = false;
            ([&] {
                using Cur = Types;
                if (ctIsBaseOf<Base, Cur>)
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

    template <typename... Types>
    struct TypeTraits : TypeTraitsVariadic<Types...> { };

    namespace detail {

        template <int Index, typename... Types>
        struct TypeIndexer;

        template <typename T, typename... Rest>
        struct TypeIndexer<0, T, Rest...> {
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

    template <typename Base, typename... In>
    constexpr static bool ctContainsDerived = TypeTraits<In...>::template containsDerived<Base>();

    template <typename... Types>
    constexpr static int ctAmount = TypeTraits<Types...>::amount();

    template <template <typename> typename Trait, typename... Types>
    constexpr static bool ctAllSatisfied = TypeTraits<Types...>::template all<Trait>();

    namespace detail {

        template <typename A, typename B>
        struct Same {
            constexpr static bool cValue = false;
        };

        template <typename T>
        struct Same<T, T> {
            constexpr static bool cValue = true;
        };

    } // namespace detail

    template <typename T>
    template <typename C>
    struct TypeTraits<T>::Same : detail::Same<T, C> { };

    template <typename T>
    template <typename Rhs>
    struct TypeTraits<T>::Addable : Bool<AddableType<T>> { };
    template <typename T>
    template <typename Rhs>
        requires(!ctIsSame<Rhs, NonType>)
    struct TypeTraits<T>::Addable<Rhs> : Bool<AddableTypes<T, Rhs>> { };

    template <typename T>
    template <typename Rhs>
    struct TypeTraits<T>::Subtractible : Bool<SubtractibleType<T>> { };
    template <typename T>
    template <typename Rhs>
        requires(!ctIsSame<Rhs, NonType>)
    struct TypeTraits<T>::Subtractible<Rhs> : Bool<SubtractibleTypes<T, Rhs>> { };

    template <typename T>
    template <typename Rhs>
    struct TypeTraits<T>::Multiplicable : Bool<MultiplicableType<T>> { };
    template <typename T>
    template <typename Rhs>
        requires(!ctIsSame<Rhs, NonType>)
    struct TypeTraits<T>::Multiplicable<Rhs> : Bool<MultiplicableTypes<T, Rhs>> { };

    template <typename T>
    template <typename Rhs>
    struct TypeTraits<T>::Divisible : Bool<DivisibleType<T>> { };
    template <typename T>
    template <typename Rhs>
        requires(!ctIsSame<Rhs, NonType>)
    struct TypeTraits<T>::Divisible<Rhs> : Bool<DivisibleTypes<T, Rhs>> { };

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

    namespace detail {

        template <typename T>
        struct FunctionPointer {
            constexpr static bool cValue = false;
        };

        template <typename R, typename... Args>
        struct FunctionPointer<R (*)(Args...)> {
            constexpr static bool cValue = true;
        };

        template <typename R, typename... Args>
        struct FunctionPointer<R (*)(Args..., ...)> {
            constexpr static bool cValue = true;
        };

    } // namespace detail

    template <typename T>
    struct TypeTraits<T>::FunctionPointer : detail::FunctionPointer<tRemoveConst<T>> { };

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

        template <typename T>
        struct RemovePointer {
            using Type = T;
        };

        template <typename T>
        struct RemovePointer<T*> : RemovePointer<T> { };

        template <typename T>
        struct RemoveConst {
            using Type = T;
        };

        template <typename T>
        struct RemoveConst<const T> {
            using Type = T;
        };

    } // namespace detail

    template <typename T>
    struct TypeTraits<T>::RemoveReference : detail::RemoveReference<T> { };

    template <typename T>
    struct TypeTraits<T>::RemovePointer : detail::RemovePointer<T> { };

    template <typename T>
    struct TypeTraits<T>::RemoveConst : detail::RemoveConst<T> { };

    namespace detail {

        template <AddableType T>
        struct Sum {
            using Type = decltype(declval<T>() + declval<T>());
        };

        template <SubtractibleType T>
        struct Difference {
            using Type = decltype(declval<T>() - declval<T>());
        };

        template <MultiplicableType T>
        struct Product {
            using Type = decltype(declval<T>() * declval<T>());
        };

        template <DivisibleType T>
        struct Quotient {
            using Type = decltype(declval<T>() / declval<T>());
        };

    } // namespace detail

    template <typename T>
    struct TypeTraits<T>::Sum : detail::Sum<T> { };

    template <typename T>
    struct TypeTraits<T>::Difference : detail::Difference<T> { };

    template <typename T>
    struct TypeTraits<T>::Product : detail::Product<T> { };

    template <typename T>
    struct TypeTraits<T>::Quotient : detail::Quotient<T> { };

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

namespace hk {

    template <typename T>
    concept FunctionType = util::ctIsFunction<T>;

    template <typename T>
    concept FunctionPointerType = util::ctIsFunctionPointer<T>;

    template <typename T>
    concept MemberFunctionPointerType = util::ctIsMemberFunctionPointer<T>;

    template <typename T>
    concept AnyFunctionType = util::ctIsFunction<T> or util::ctIsFunctionPointer<T> or util::ctIsMemberFunctionPointer<T>;

    template <typename T>
    concept AnyFunctionPointerType = util::ctIsFunctionPointer<T> or util::ctIsMemberFunctionPointer<T>;

} // namespace hk
