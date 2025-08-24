#pragma once

#include "hk/Result.h"
#include "hk/diag/diag.h"
#include "hk/diag/results.h"
#include "hk/util/Lambda.h"
#include <memory>
#include <type_traits>
#include <utility>

namespace hk {

    template <typename T>
    class ValueOrResult {
        Result mResult = ResultSuccess();
        union {
            T mValue;
        };

        constexpr bool hasValue() const { return mResult.succeeded(); }

        constexpr T disown() {
            HK_ABORT_UNLESS(hasValue(), "hk::ValueOrResult::disown(): No value (%04d-%04d/0x%x)",
                mResult.getModule() + 2000,
                mResult.getDescription(),
                mResult.getValue());
            mResult = diag::ResultValueDisowned();

            return move(mValue);
        }

    public:
        using Type = T;

        constexpr ValueOrResult(Result result)
            : mResult(result) {
            HK_ABORT_UNLESS(result.failed(), "hk::ValueOrResult(Result): Result must not be ResultSuccess()", 0);
        }

        constexpr ValueOrResult(T&& value) {
            std::construct_at(&mValue, std::forward<T>(value));
        }

        constexpr ~ValueOrResult() {
            if (hasValue())
                mValue.~T();
        }

        template <typename L>
        constexpr ValueOrResult<typename util::FunctionTraits<L>::ReturnType> map(L func) {
            using Return = typename util::FunctionTraits<L>::ReturnType;

            if (hasValue()) {
                if constexpr (std::is_same_v<Return, void>) {
                    func(disown());
                    return ResultSuccess();
                } else
                    return func(disown());
            } else
                return mResult;
        }

        constexpr const T& value() const {
            HK_ABORT_UNLESS(hasValue(), "hk::ValueOrResult::value(): No value (%04d-%04d/0x%x)",
                mResult.getModule() + 2000,
                mResult.getDescription(),
                mResult.getValue());
            return mValue;
        }

        constexpr operator Result() const { return mResult; }
        constexpr operator T() { return move(disown()); }
    };

    template <>
    class ValueOrResult<void> : public Result {
    public:
        using Type = Result;
        ValueOrResult(Result result)
            : Result(result) { }

        ValueOrResult() = default;
    };

#define HK_UNWRAP(VALUE)                                                                \
    ([&]() hk_alwaysinline {                                                            \
        auto&& _hk_unwrap_v = VALUE;                                                    \
        using _ValueOrResult_T = std::remove_reference_t<decltype(_hk_unwrap_v)>::Type; \
        ::hk::Result _result_temp = _hk_unwrap_v;                                       \
        if (_result_temp.succeeded())                                                   \
            return (_ValueOrResult_T)_hk_unwrap_v;                                      \
        else {                                                                          \
            ::hk::diag::abortImpl(                                                      \
                ::hk::svc::BreakReason_User,                                            \
                _result_temp,                                                           \
                __FILE__,                                                               \
                __LINE__,                                                               \
                ::hk::diag::cAbortUnlessResultFormat,                                   \
                _result_temp.getModule() + 2000,                                        \
                _result_temp.getDescription(),                                          \
                _result_temp.getValue(),                                                \
                "HK_UNWRAP(" #VALUE ")");                                               \
        }                                                                               \
    })()

} // namespace hk
