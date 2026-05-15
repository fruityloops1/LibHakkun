#pragma once

#include "hk/types.h"

namespace hk {

    namespace util {

        template <typename L>
        struct LambdaHasCapture {
            constexpr static bool value = sizeof(L) != 1;
        };

        /**
         * @brief Traits of lambda function
         *
         * FunctionTraits::ReturnType: Return type of lambda
         * FunctionTraits::FuncPtrType: Function pointer type of lambda
         *
         * @tparam L
         */
        template <typename L>
        struct FunctionTraits : FunctionTraits<decltype(&L::operator())> {
        private:
            using Super = FunctionTraits<decltype(&L::operator())>;

        public:
            static Super::FuncPtrTypeStatic fromLambda(L&& func) {
                static_assert(!LambdaHasCapture<L>::value);
                return (typename Super::FuncPtrTypeStatic)func;
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
