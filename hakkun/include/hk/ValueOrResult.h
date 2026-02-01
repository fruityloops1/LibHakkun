#pragma once

#include "hk/Result.h"
#include "hk/diag/diag.h"
#include "hk/diag/results.h"
#include "hk/util/Lambda.h"
#include "hk/util/TemplateString.h"
#include "hk/util/TypeName.h"
#include <memory>
#include <type_traits>
#include <utility>

namespace hk {

    template <typename T>
    class ValueOrResult;

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
         * @brief If a value is contained, call func to convert it to a result.
         *
         * @tparam L
         * @param func
         * @return Result
         */
        template <typename L>
        constexpr Result mapToResult(L func) {
            if (hasValue())
                return func(disown());

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

    /**
     * @brief Holds a Result and a reference to a value of type T, when the Result is ResultSuccess().
     *
     * @tparam T
     */
    template <ReferenceType T>
    class ValueOrResult<T> {
        using BaseType = std::remove_reference_t<T>;

        Result mResult = ResultSuccess();
        BaseType* mValueReference = nullptr;

        constexpr BaseType& get() {
            HK_ABORT_UNLESS(hasValue(), "hk::ValueOrResult<%s>::get(): No value (%04d-%04d/0x%x)",
                util::getTypeName<T>(),
                mResult.getModule() + 2000,
                mResult.getDescription(),
                mResult.getValue());

            return *mValueReference;
        }

    public:
        using Type = T;

        constexpr ValueOrResult(Result result)
            : mResult(result) {
            HK_ABORT_UNLESS(result.failed(), "hk::ValueOrResult<%s>(Result): Result must not be ResultSuccess()", util::getTypeName<T>());
        }

        constexpr ValueOrResult(BaseType* ptr)
            : mValueReference(ptr) {
            if (mValueReference == nullptr)
                mResult = ResultNoValue();
        }

        constexpr ValueOrResult(BaseType&& value)
            : ValueOrResult(&value) {
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
                    func(get());
                    return ResultSuccess();
                } else
                    return func(get());
            } else
                return mResult;
        }

        /**
         * @brief If a value is contained, call func to convert it to a result.
         *
         * @tparam L
         * @param func
         * @return Result
         */
        template <typename L>
        constexpr Result mapToResult(L func) {
            if (hasValue())
                return func(get());

            return mResult;
        }

        /**
         * @brief Retrieves the value, if valid. Aborts if not.
         *
         * @return T&
         */
        constexpr BaseType& value() const {
            HK_ABORT_UNLESS(hasValue(), "hk::ValueOrResult<%s>::value(): No value (%04d-%04d/0x%x)",
                util::getTypeName<T>(),
                mResult.getModule() + 2000,
                mResult.getDescription(),
                mResult.getValue());
            return get();
        }

        constexpr operator Result() const { return mResult; }
        constexpr operator T&() { return get(); }
        constexpr BaseType* operator->() { return &get(); }
    };

    template <>
    class ValueOrResult<void> : public Result {
    public:
        using Type = Result;
        ValueOrResult(Result result)
            : Result(result) { }

        ValueOrResult() = default;
    };

    template <typename T, util::TemplateString AbortMsg, util::TemplateString File, int Line>
    struct UnwrapChecker;

    template <PointerType T, util::TemplateString AbortMsg, util::TemplateString File, int Line>
    struct UnwrapChecker<T, AbortMsg, File, Line> {
        static hk_alwaysinline T check(T value) {
            if (value == nullptr) {
                diag::abortImpl(
                    svc::BreakReason_User,
                    ResultNoValue(),
                    File.value,
                    Line,
                    diag::cNullptrUnwrapFormat,
                    AbortMsg.value);
            }
            return value;
        }
    };

    template <typename T, util::TemplateString AbortMsg, util::TemplateString File, int Line>
    struct UnwrapChecker<ValueOrResult<T>, AbortMsg, File, Line> {
        static hk_alwaysinline T check(ValueOrResult<T>&& value) {
            const Result _result_temp = value;
            if (_result_temp.failed()) {
                const char* _result_temp_name = diag::getResultName(_result_temp);
                if (_result_temp_name != nullptr) {
                    diag::abortImpl(
                        svc::BreakReason_User,
                        _result_temp,
                        File.value,
                        Line,
                        diag::cUnwrapResultFormatWithName,
                        _result_temp.getModule() + 2000,
                        _result_temp.getDescription(),
                        _result_temp_name,
                        AbortMsg.value);
                } else {
                    diag::abortImpl(
                        svc::BreakReason_User,
                        _result_temp,
                        File.value,
                        Line,
                        diag::cUnwrapResultFormat,
                        _result_temp.getModule() + 2000,
                        _result_temp.getDescription(),
                        _result_temp.getValue(),
                        AbortMsg.value);
                }
            }
            return move((typename ValueOrResult<T>::Type)value);
        }
    };

/**
 * @brief Retrieve the value of a ValueOrResult<T>, abort if the Result is unsuccessful.
 *        When VALUE is a pointer, abort if it is nullptr.
 *
 */
#define HK_UNWRAP(VALUE)                                                                                   \
    ({                                                                                                     \
        auto&& _hk_unwrap_v = VALUE;                                                                       \
        using _ValueT = std::remove_reference_t<decltype(_hk_unwrap_v)>;                                   \
        ::hk::UnwrapChecker<_ValueT, #VALUE, __FILE__, __LINE__>::check(::forward<_ValueT>(_hk_unwrap_v)); \
    })

    namespace detail {

        template <int Module, int Description>
        inline hk_alwaysinline constexpr void getTryExpressionValue(const hk::ResultV<Module, Description>&&) { }
        inline hk_alwaysinline constexpr void getTryExpressionValue(const hk::Result&&) { }
        template <typename T>
        inline hk_alwaysinline constexpr T getTryExpressionValue(ValueOrResult<T>&& value) {
            return move((T)value);
        }

    } // namespace detail

/**
 * @brief Return if Result within expression is unsuccessful. Returns value of ValueOrResult when applicable.
 * Function must return Result.
 */
#undef HK_TRY
#define HK_TRY(VALUE)                                             \
    ({                                                            \
        auto&& _value_temp = VALUE;                               \
                                                                  \
        ::hk::Result _result_temp = _value_temp;                  \
        if (_result_temp.failed())                                \
            return _result_temp;                                  \
                                                                  \
        ::hk::detail::getTryExpressionValue(::move(_value_temp)); \
    })

} // namespace hk
