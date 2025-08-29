#pragma once

#include "hk/Result.h"
#include "hk/diag/diag.h"
#include "hk/diag/results.h"
#include "hk/util/Lambda.h"
#include "hk/util/TypeName.h"
#include <memory>
#include <type_traits>
#include <utility>

namespace hk {

    /**
     * @brief Holds a Result and a value of type T, when the Result is ResultSuccess().
     *
     * @tparam T
     */
    template <typename T>
    class ValueOrResult {
        Result mResult = ResultSuccess();
        union {
            T mValue;
        };

        constexpr T disown() {
            HK_ABORT_UNLESS(hasValue(), "hk::ValueOrResult<%s>::disown(): No value (%04d-%04d/0x%x)",
                util::getTypeName<T>(),
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
            HK_ABORT_UNLESS(result.failed(), "hk::ValueOrResult<%s>(Result): Result must not be ResultSuccess()", util::getTypeName<T>());
        }

        constexpr ValueOrResult(T&& value) {
            std::construct_at(&mValue, std::forward<T>(value));
        }

        constexpr ~ValueOrResult() {
            if (hasValue())
                mValue.~T();
        }

        constexpr bool hasValue() const { return mResult.succeeded(); }

        /**
         * @brief If a value is contained, call func with the value and return its result.
         *
         * @tparam L
         * @param func
         * @return ValueOrResult<typename util::FunctionTraits<L>::ReturnType>
         */
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

        /**
         * @brief Retrieves the value, if valid. Aborts if not.
         *
         * @return const T&
         */
        constexpr const T& value() const {
            HK_ABORT_UNLESS(hasValue(), "hk::ValueOrResult<%s>::value(): No value (%04d-%04d/0x%x)",
                util::getTypeName<T>(),
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

/**
 * @brief Retrieve the value of a ValueOrResult<T>, abort if the Result is unsuccessful.
 *
 */
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
