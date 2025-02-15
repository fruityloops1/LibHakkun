#pragma once

namespace hk::util {

    template <typename L>
    struct FunctionTraits : public FunctionTraits<decltype(&L::operator())> { };

    template <typename Class, typename Return, typename... Args>
    struct FunctionTraits<Return (Class::*)(Args...) const> {
        using ReturnType = Return;
        using FuncPtrType = ReturnType (*)(Args...);
    };

    template <typename L>
    struct LambdaHasCapture {
        constexpr static bool value = sizeof(L) != 1;
    };

} // namespace hk::util
