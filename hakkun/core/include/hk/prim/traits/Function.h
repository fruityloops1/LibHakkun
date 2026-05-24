#pragma once

#include "hk/prim/traits/Type.h"
#include "hk/types.h"

namespace hk {

    namespace util {

        template <typename...>
        struct FunctionTraits;

        /**
         * @brief Traits of lambda function
         *
         * FunctionTraits::ReturnType: Return type of lambda
         * FunctionTraits::FuncPtrType: Function pointer type of lambda
         *
         * @tparam L
         */
        template <LambdaType L>
        struct FunctionTraits<L> : FunctionTraits<decltype(&L::operator())> {
        private:
            using Super = FunctionTraits<decltype(&L::operator())>;

        public:
            static Super::FuncPtrTypeStatic fromLambda(L&& func) {
                static_assert(!ctLambdaHasCapture<L>);
                return (typename Super::FuncPtrTypeStatic)func;
            }
        };

        template <LambdaDeduceThisType L>
        struct FunctionTraits<L> : FunctionTraits<L, decltype(&L::template operator()<L>)> { };

        template <typename L, typename Return, typename Self, typename... Args>
        struct FunctionTraits<L, Return (*)(Self, Args...)> : FunctionTraits<Return (*)(Args...)> {
            using Super = FunctionTraits<Return (*)(Args...)>;

            static hk_noinline Return invoke(Args... args) {
                using FuncPtrTypeStatic = typename Super::FuncPtrTypeStatic;
                return L()(forward<Args>(args)...);
            }
        };

        template <typename L, typename Return, typename Self, typename... Args>
        struct FunctionTraits<L, Return (*)(Self, Args..., ...)> : FunctionTraits<Return (*)(Args..., ...)> {
            using Super = FunctionTraits<Return (*)(Args..., ...)>;

            static hk_noinline Return invoke(Args... args) {
                return L()(forward<Args>(args)...);
            }
        };

        template <typename L, typename Class, typename Return, typename Self, typename... Args>
        struct FunctionTraits<L, Return (Class::*)(Self, Args...)> : FunctionTraits<Return (Class::*)(Args...)> {
            using Super = FunctionTraits<Return (Class::*)(Args...)>;

            static hk_noinline Return invoke(Args... args) {
                return L()(forward<Args>(args)...);
            }
        };

        template <typename L, typename Class, typename Return, typename Self, typename... Args>
        struct FunctionTraits<L, Return (Class::*)(Self, Args..., ...)> : FunctionTraits<Return (Class::*)(Args..., ...)> {
            using Super = FunctionTraits<Return (Class::*)(Args..., ...)>;

            static hk_noinline Return invoke(Args... args) {
                return L()(forward<Args>(args)...);
            }
        };

        template <typename Return, typename... Args>
        struct FunctionTraits<Return (*)(Args...)> {
            using ReturnType = Return;
            using FuncPtrType = ReturnType (*)(Args...);
            using FuncPtrTypeStatic = FuncPtrType;

            static hk_alwaysinline ptr getAddress(FuncPtrType func) { return ptr(func); }
        };

        template <typename Return, typename... Args>
        struct FunctionTraits<Return (*)(Args..., ...)> {
            using ReturnType = Return;
            using FuncPtrType = ReturnType (*)(Args..., ...);
            using FuncPtrTypeStatic = FuncPtrType;

            static hk_alwaysinline ptr getAddress(FuncPtrType func) { return ptr(func); }
        };

        template <typename _Class, typename Return, typename... Args>
        struct FunctionTraits<Return (_Class::*)(Args...)> {
            using ReturnType = Return;
            using FuncPtrType = ReturnType (_Class::*)(Args...);
            using FuncPtrTypeStatic = ReturnType (*)(Args...);

            using Class = _Class;

            static ptr getAddress(FuncPtrType func) {
                ptr address = pun<ptr>(func);
                if (address < cMinimumMappedAddress) // likely virtual PTMF (or nullptr)
                    return 0;
                return address;
            }
        };

        template <typename _Class, typename Return, typename... Args>
        struct FunctionTraits<Return (_Class::*)(Args...) const> : FunctionTraits<tRemoveConst<Return (_Class::*)(Args...)>> { };

        template <typename _Class, typename Return, typename... Args>
        struct FunctionTraits<Return (_Class::*)(Args..., ...)> {
            using ReturnType = Return;
            using FuncPtrType = ReturnType (_Class::*)(Args..., ...);
            using FuncPtrTypeStatic = ReturnType (*)(Args..., ...);

            using Class = _Class;

            static ptr getAddress(FuncPtrType func) {
                ptr address = pun<ptr>(func);
                if (address < cMinimumMappedAddress) // likely virtual PTMF (or nullptr)
                    return 0;
                return address;
            }
        };

        template <typename _Class, typename Return, typename... Args>
        struct FunctionTraits<Return (_Class::*)(Args..., ...) const> : FunctionTraits<tRemoveConst<Return (_Class::*)(Args..., ...)>> { };

    } // namespace util

} // namespace hk
