#pragma once

#include "hk/Result.h"
#include "hk/diag/diag.h"
#include "hk/diag/results.h"
#include "hk/util/Lambda.h"
#include <type_traits>
#include <utility>

namespace hk {

    template <typename T>
    class ValueOrResult {
        Result mResult = ResultSuccess();
        alignas(alignof(T)) u8 mStorage[sizeof(T)] { 0 };

        T* getPtr() { return cast<T*>(mStorage); }

        bool hasValue() const { return mResult.succeeded(); }

        T disown() {
            HK_ABORT_UNLESS(hasValue(), "hk::ValueOrResult::disown(): No value", 0);
            mResult = diag::ResultValueDisowned();

            return std::move(*getPtr());
        }

    public:
        using Type = T;

        ValueOrResult(Result result)
            : mResult(result) {
            HK_ABORT_UNLESS(result.failed(), "hk::ValueOrResult(Result): Result must not be ResultSuccess()", 0);
        }

        ValueOrResult(T&& value) {
            new (getPtr()) T(std::forward<T>(value));
        }

        ~ValueOrResult() {
            if (hasValue())
                getPtr()->~T();
        }

        template <typename L>
        ValueOrResult<typename util::FunctionTraits<L>::ReturnType> map(L func) {
            if (hasValue())
                return func(disown());
            else
                return mResult;
        }

        operator Result() const { return mResult; }
        operator T() { return std::move(disown()); }
    };

    template <>
    class ValueOrResult<void> : public Result {
    public:
        using Type = Result;
    };

#define HK_UNWRAP(VALUE)                                      \
    ([&]() {                                                  \
        auto&& v = VALUE;                                     \
        using T = std::remove_reference_t<decltype(v)>::Type; \
        ::hk::Result _result_temp = v;                        \
        if (_result_temp.succeeded())                         \
            return (T)v;                                      \
        else {                                                \
            ::hk::diag::abortImpl(                            \
                ::hk::svc::BreakReason_User,                  \
                _result_temp,                                 \
                __FILE__,                                     \
                __LINE__,                                     \
                ::hk::diag::cAbortUnlessResultFormat,         \
                _result_temp.getModule() + 2000,              \
                _result_temp.getDescription(),                \
                "HK_UNWRAP(" #VALUE ")");                     \
        }                                                     \
    })()

} // namespace hk
