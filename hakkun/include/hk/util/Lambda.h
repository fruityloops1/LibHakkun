#pragma once

namespace hk::util {

    /**
     * @brief Traits of lambda function
     *
     * FunctionTraits::ReturnType: Return type of lambda
     * FunctionTraits::FuncPtrType: Function pointer type of lambda
     *
     * @tparam L
     */
    template <typename L>
    struct FunctionTraits : public FunctionTraits<decltype(&L::operator())> { };

    template <typename Return, typename... Args>
    struct FunctionTraits<Return (*)(Args...)> {
        using ReturnType = Return;
        using FuncPtrType = ReturnType (*)(Args...);
    };

    template <typename Class, typename Return, typename... Args>
    struct FunctionTraits<Return (Class::*)(Args...) const> {
        using ReturnType = Return;
        using FuncPtrType = ReturnType (*)(Args...);
    };

    template <typename Class, typename Return, typename... Args>
    struct FunctionTraits<Return (Class::*)(Args..., ...) const> {
        using ReturnType = Return;
        using FuncPtrType = ReturnType (*)(Args..., ...);
    };

    template <typename L>
    struct LambdaHasCapture {
        constexpr static bool value = sizeof(L) != 1;
    };

} // namespace hk::util
