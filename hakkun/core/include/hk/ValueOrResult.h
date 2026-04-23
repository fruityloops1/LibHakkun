#pragma once

#include "hk/Result.h"
#include "hk/diag/diag.h"
#include "hk/diag/results.h"
#include "hk/prim/traits/Function.h"
#include "hk/util/Algorithm.h"
#include "hk/util/TemplateString.h"
#include "hk/util/TypeName.h"
#include <memory>
#include <type_traits>
#include <utility>

namespace hk {

    template <typename T>
    class ValueOrResult;

    template <typename T>
    constexpr ResultBase::operator ValueOrResult<T>() const { return ValueOrResult<T>(Result(*this)); }
    template <typename T>
    constexpr Result::operator ValueOrResult<T>() const { return ValueOrResult<T>(Result(*this)); }

    template <typename T>
    struct ValueOrResultMapTraits;

    template <typename T>
    struct ValueOrResultMapTraits<ValueOrResult<T>> {
        using Return = ValueOrResult<T>;
    };

    template <Derived<hk::Result> T>
    struct ValueOrResultMapTraits<ValueOrResult<T>> {
        using Return = Result;
    };

    /**
     * @brief Holds a Result and a value of type T, when the Result is ResultSuccess().
     *
     * @tparam T
     */
    template <typename T>
    class ValueOrResult {
        Result mResult = ResultSuccess();
        union { // this is a union because it allows you to control the destruction of the object within in constexpr
            T mValue;
        };

        constexpr T disown() {
            HK_ABORT_UNLESS(hasValue(), "hk::ValueOrResult<%s>::disown(): No value (%04d-%04d/0x%x)",
                util::getTypeName<T>(),
                mResult.getModule() + 2000,
                mResult.getDescription(),
                mResult.getValue());
            mResult = MAKE_RESULT(diag::ResultValueDisowned());

            return move(mValue);
        }

    public:
        using Type = T;

        explicit constexpr ValueOrResult(Result result)
            : mResult(result) {
            HK_ABORT_UNLESS(result.failed(), "hk::ValueOrResult<%s>(Result): Result must not be ResultSuccess()", util::getTypeName<T>());
        }

        constexpr ValueOrResult(const T& value) {
            construct_at(&mValue, value);
        }

        constexpr ValueOrResult(T&& value) {
            construct_at(&mValue, forward<T>(value));
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
         * @return ValueOrResult<ReturnType of L>
         */
        template <typename L>
        constexpr ValueOrResultMapTraits<ValueOrResult<typename util::FunctionTraits<L>::ReturnType>>::Return map(L func) {
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
         * @brief Get inner value without disowning.
         *
         * @return const T&
         */
        constexpr const T& getInnerValue() const {
            HK_ABORT_UNLESS(hasValue(), "hk::ValueOrResult<%s>::getInnerValue(): No value (%04d-%04d/0x%x)",
                util::getTypeName<T>(),
                mResult.getModule() + 2000,
                mResult.getDescription(),
                mResult.getValue());
            return mValue;
        }

        constexpr T operator or(T&& defaultValue) {
            return hasValue() ? move(disown()) : move(defaultValue);
        }

        constexpr T operator or(const T& defaultValue) {
            return hasValue() ? move(disown()) : defaultValue;
        }

        constexpr T orElse(T&& defaultValue) {
            return *this or defaultValue;
        }

        constexpr T orElse(const T& defaultValue) {
            return *this or defaultValue;
        }

        constexpr operator Result() const { return mResult; }
        constexpr operator T() { return move(disown()); }
    };

    /**
     * @brief Holds a Result and a reference to a value of type T, when the Result is ResultSuccess().
     *
     * @tparam T
     */
    template <typename T>
    class ValueOrResult<T&> {
        Result mResult = ResultSuccess();
        T* mValueReference = nullptr;

        constexpr T& get() {
            HK_ABORT_UNLESS(hasValue(), "hk::ValueOrResult<%s>::get(): No value (%04d-%04d/0x%x)",
                util::getTypeName<T&>(),
                mResult.getModule() + 2000,
                mResult.getDescription(),
                mResult.getValue());

            return *mValueReference;
        }

    public:
        using Type = T;

        explicit constexpr ValueOrResult(Result result)
            : mResult(result) {
            HK_ABORT_UNLESS(result.failed(), "hk::ValueOrResult<%s>(Result): Result must not be ResultSuccess()", util::getTypeName<T&>());
        }

        constexpr ValueOrResult(T* ptr)
            : mValueReference(ptr) {
            if (mValueReference == nullptr)
                mResult = ResultNoValue();
        }

        constexpr ValueOrResult(T& value)
            : ValueOrResult(&value) {
        }

        constexpr bool hasValue() const { return mResult.succeeded(); }

        /**
         * @brief If a value is contained, call func with the value and return its result.
         *
         * @tparam L
         * @param func
         * @return ValueOrResult<ReturnType of L>
         */
        template <typename L>
        constexpr ValueOrResultMapTraits<ValueOrResult<typename util::FunctionTraits<L>::ReturnType>>::Return map(L func) {
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
         * @brief Get inner value without disowning.
         *
         * @return T&
         */
        constexpr T& getInnerValue() const {
            HK_ABORT_UNLESS(hasValue(), "hk::ValueOrResult<%s>::getInnerValue(): No value (%04d-%04d/0x%x)",
                util::getTypeName<T&>(),
                mResult.getModule() + 2000,
                mResult.getDescription(),
                mResult.getValue());
            return get();
        }

        constexpr T& operator or(T& defaultValue) const {
            return hasValue() ? get() : defaultValue;
        }

        constexpr T orElse(T& defaultValue) {
            return *this or defaultValue;
        }

        constexpr operator Result() const { return mResult; }
        constexpr operator T&() { return get(); }
        constexpr T* operator->() { return &get(); }
    };

    template <>
    class ValueOrResult<void> : public Result {
    public:
        using Type = Result;
        constexpr ValueOrResult() = default;

        constexpr ValueOrResult(Result result)
            : Result(result) {
        }
    };

    namespace detail {

        template <typename Result, typename T, util::TemplateString Expr, util::TemplateString File, int Line>
        struct ResultChecker<Result, ValueOrResult<T>, Expr, File, Line> {
            hk_alwaysinline static constexpr Result check(RvalueRefWithMsg<ValueOrResult<T>>& value) {
                return MAKE_RESULT_IMPL(Result(value.ref), Expr.value, File.value, Line
#if HK_RESULT_ADVANCED
                    ,
                    value.msg
#endif
                );
            }
        };

        template <typename Result, typename T, util::TemplateString Expr, util::TemplateString File, int Line>
        struct ResultChecker<Result, ValueOrResult<T&>, Expr, File, Line> {
            hk_alwaysinline static constexpr Result check(RvalueRefWithMsg<ValueOrResult<T&>>& value) {
                return MAKE_RESULT_IMPL(Result(value), Expr.value, File.value, Line
#if HK_RESULT_ADVANCED
                    ,
                    value.msg
#endif
                );
            }
        };

        template <typename T, util::TemplateString Expr, util::TemplateString File, int Line>
        struct UnwrapChecker;

        template <typename T, util::TemplateString Expr, util::TemplateString File, int Line>
        struct UnwrapChecker<T*, Expr, File, Line> {
            static hk_alwaysinline T* check(RvalueRefWithMsg<T*>& value) {
                const Result _result_temp = ResultChecker<Result, T*, Expr, File, Line>::check(value);

                if (value.ref == nullptr) {
#if defined(HK_RELEASE) and not defined(HK_RELEASE_DEBINFO)
                    diag::abortReleaseImpl<File, Line>(_result_temp);
#else
                    diag::abortImpl(
                        HAS_NNSDK(svc::BreakReason_User, )
                            _result_temp,
                        File.value,
                        Line,
                        diag::cNullptrUnwrapFormat,
                        Expr.value);
#endif
                }
                return value.ref;
            }
        };

        template <typename T, util::TemplateString Expr, util::TemplateString File, int Line>
        struct UnwrapChecker<ValueOrResult<T>, Expr, File, Line> {
            static hk_alwaysinline T check(RvalueRefWithMsg<ValueOrResult<T>>& value) {
                const Result _result_temp = ResultChecker<Result, ValueOrResult<T>, Expr, File, Line>::check(value);

                if (_result_temp.failed()) {
#if defined(HK_RELEASE) and not defined(HK_RELEASE_DEBINFO)
                    diag::abortReleaseImpl<File, Line>(_result_temp);
#else
                    const char* _result_temp_name = diag::getResultName(_result_temp);
                    if (_result_temp_name != nullptr) {
                        diag::abortImpl(
                            HAS_NNSDK(svc::BreakReason_User, )
                                _result_temp,
                            File.value,
                            Line,
                            diag::cUnwrapResultFormatWithName,
                            _result_temp.getModule() + 2000,
                            _result_temp.getDescription(),
                            _result_temp_name,
                            Expr.value);
                    } else {
                        diag::abortImpl(
                            HAS_NNSDK(svc::BreakReason_User, )
                                _result_temp,
                            File.value,
                            Line,
                            diag::cUnwrapResultFormat,
                            _result_temp.getModule() + 2000,
                            _result_temp.getDescription(),
                            _result_temp.getValue(),
                            Expr.value);
                    }
#endif
                }
                return move((T)value.ref);
            }
        };

        template <typename T, util::TemplateString Expr, util::TemplateString File, int Line>
        struct UnwrapChecker<ValueOrResult<T&>, Expr, File, Line> {
            static hk_alwaysinline ValueOrResult<T&> check(RvalueRefWithMsg<ValueOrResult<T&>>& value) {
                const Result _result_temp = ResultChecker<Result, ValueOrResult<T&>, Expr, File, Line>::check(value);

                if (_result_temp.failed()) {
#if defined(HK_RELEASE) and not defined(HK_RELEASE_DEBINFO)
                    diag::abortReleaseImpl<File, Line>(_result_temp);
#else
                    const char* _result_temp_name = diag::getResultName(_result_temp);
                    if (_result_temp_name != nullptr) {
                        diag::abortImpl(
                            HAS_NNSDK(svc::BreakReason_User, )
                                _result_temp,
                            File.value,
                            Line,
                            diag::cUnwrapResultFormatWithName,
                            _result_temp.getModule() + 2000,
                            _result_temp.getDescription(),
                            _result_temp_name,
                            Expr.value);
                    } else {
                        diag::abortImpl(
                            HAS_NNSDK(svc::BreakReason_User, )
                                _result_temp,
                            File.value,
                            Line,
                            diag::cUnwrapResultFormat,
                            _result_temp.getModule() + 2000,
                            _result_temp.getDescription(),
                            _result_temp.getValue(),
                            Expr.value);
                    }
#endif
                }
                return move(value.ref);
            }
        };

/**
 * @brief Retrieve the value of a ValueOrResult<T>, abort if the Result is unsuccessful.
 *        When VALUE is a pointer, abort if it is nullptr.
 *
 */
#define HK_UNWRAP(VALUE, ...)                                                                                               \
    ({                                                                                                                      \
        ::hk::detail::RvalueRefWithMsg _hk_unwrap_v = { VALUE __VA_OPT__(, ) __VA_ARGS__ };                                 \
        using _ValueT = decltype(_hk_unwrap_v)::Type;                                                                       \
        ::hk::detail::UnwrapChecker<_ValueT, #VALUE __VA_OPT__(",") #__VA_ARGS__, __FILE__, __LINE__>::check(_hk_unwrap_v); \
    })

        template <typename T>
        inline hk_alwaysinline constexpr T getTryExpressionValue(T&& value) {
            return move(value);
        }
        template <typename T>
        inline hk_alwaysinline constexpr T getTryExpressionValue(ValueOrResult<T>&& value) {
            return move((T)value);
        }
        template <typename T>
        inline hk_alwaysinline constexpr ValueOrResult<T&> getTryExpressionValue(ValueOrResult<T&>&& value) {
            return move(value);
        }

    } // namespace detail

/**
 * @brief Return if Result within expression is unsuccessful. Returns value of ValueOrResult when applicable.
 * Function must return Result.
 */
#undef HK_TRY
#define HK_TRY(VALUE, ...)                                                                                                                                         \
    ({                                                                                                                                                             \
        ::hk::detail::RvalueRefWithMsg _value_temp = { VALUE __VA_OPT__(, ) __VA_ARGS__ };                                                                         \
        using _ValueT = decltype(_value_temp)::Type;                                                                                                               \
        using _ResultT = ::hk::detail::TryResultType<_ValueT>::Type;                                                                                               \
                                                                                                                                                                   \
        const _ResultT _result_temp = ::hk::detail::ResultChecker<_ResultT, _ValueT, #VALUE __VA_OPT__(",") #__VA_ARGS__, __FILE__, __LINE__>::check(_value_temp); \
        if (_result_temp.failed())                                                                                                                                 \
            return _result_temp;                                                                                                                                   \
        ::hk::detail::getTryExpressionValue(::move(_value_temp.ref));                                                                                              \
    })

} // namespace hk
