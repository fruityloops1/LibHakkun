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
        union {
            T mValue;
            Result mResult;
        };
        bool mHasValue;

        constexpr T disown() {
            HK_ABORT_UNLESS_R(Result(*this));

            T value = move(mValue);
            mResult = MAKE_RESULT(diag::ResultValueDisowned());
            return move(value);
        }

    public:
        using Type = T;

        explicit constexpr ValueOrResult(Result result, diag::SourceLocation loc = diag::SourceLocation::current())
            : mResult(result)
            , mHasValue(false) {
            HK_ABORT_UNLESS_WITH_LOCATION(loc, result.failed(), "hk::ValueOrResult<%s>(Result): Result must not be ResultSuccess()", util::getTypeName<T>());
        }

        constexpr ValueOrResult(const T& value)
            : mHasValue(true) {
            construct_at(&mValue, value);
        }

        constexpr ValueOrResult(T&& value)
            : mHasValue(true) {
            construct_at(&mValue, forward<T>(value));
        }

        constexpr ~ValueOrResult() {
            if (hasValue())
                mValue.~T();
        }

        constexpr bool hasValue() const { return mHasValue; }

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
                return Result(*this);
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

            return Result(*this);
        }

        /**
         * @brief Get inner value without disowning.
         *
         * @return const T&
         */
        constexpr const T& getInnerValue(diag::SourceLocation loc = diag::SourceLocation::current()) const {
            HK_ABORT_UNLESS_R_WITH_LOCATION(loc, Result(*this));
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

        constexpr operator Result() const { return mHasValue ? ResultSuccess() : mResult; }
        constexpr operator T() { return move(disown()); }
    };

    /**
     * @brief Holds a Result and a reference to a value of type T, when the Result is ResultSuccess().
     *
     * @tparam T
     */
    template <typename T>
    class ValueOrResult<T&> {
        union {
            T* mValueReference;
            Result mResult;
        };
        bool mHasValue;

        constexpr T& get() {
            HK_ABORT_UNLESS_R(Result(*this));

            return *mValueReference;
        }

    public:
        using Type = T;

        explicit constexpr ValueOrResult(Result result, diag::SourceLocation loc = diag::SourceLocation::current())
            : mResult(result)
            , mHasValue(false) {
            HK_ABORT_UNLESS_WITH_LOCATION(loc, result.failed(), "hk::ValueOrResult<%s>(Result): Result must not be ResultSuccess()", util::getTypeName<T&>());
        }

        constexpr ValueOrResult(T* ptr)
            : mHasValue(ptr != nullptr) {
            if (ptr != nullptr)
                mValueReference = ptr;
            else
                mResult = ResultNoValue();
        }

        constexpr ValueOrResult(T& value)
            : ValueOrResult(&value) { }

        constexpr bool hasValue() const { return mHasValue; }

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
                return Result(*this);
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

            return Result(*this);
        }

        /**
         * @brief Get inner value without disowning.
         *
         * @return T&
         */
        constexpr const T& getInnerValue(diag::SourceLocation loc = diag::SourceLocation::current()) const {
            HK_ABORT_UNLESS_R_WITH_LOCATION(loc, Result(*this));
            return get();
        }

        constexpr T& operator or(T& defaultValue) {
            return hasValue() ? get() : defaultValue;
        }

        constexpr T orElse(T& defaultValue) {
            return *this or defaultValue;
        }

        constexpr operator Result() const { return mHasValue ? ResultSuccess() : mResult; }
        constexpr operator T&() { return get(); }
        constexpr T* operator->() { return &get(); }
        constexpr T* ptr() { return hasValue() ? &get() : nullptr; }
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

        template <typename Result, typename T, util::TemplateString Expr, util::TemplateString File, u32 Line, u16 Col>
        struct ResultChecker<Result, ValueOrResult<T>, Expr, File, Line, Col> {
            hk_alwaysinline static constexpr Result check(const ValueOrResult<T>& value) { return MAKE_RESULT_IMPL(Result(value), Expr.value, File.value, Line, Col); }
        };

        template <typename Result, typename T, util::TemplateString Expr, util::TemplateString File, u32 Line, u16 Col>
        struct ResultChecker<Result, ValueOrResult<T&>, Expr, File, Line, Col> {
            hk_alwaysinline static constexpr Result check(const ValueOrResult<T&>& value) { return MAKE_RESULT_IMPL(Result(value), Expr.value, File.value, Line, Col); }
        };

        template <typename T, util::TemplateString Expr, util::TemplateString File, u32 Line, u16 Col>
        struct UnwrapChecker;

        template <typename T, util::TemplateString Expr, util::TemplateString File, u32 Line, u16 Col>
        struct UnwrapChecker<T*, Expr, File, Line, Col> {
            static hk_alwaysinline T* check(T* value) {
                const Result _result_temp = ResultChecker<Result, T*, Expr, File, Line, Col>::check(value);

                if (value == nullptr) {
#if defined(HK_RELEASE) and not defined(HK_RELEASE_DEBINFO)
                    diag::abortReleaseImpl<File, Line, Col>(_result_temp);
#else
                    diag::abortImpl(
                        HAS_NNSDK(svc::BreakReason_User, )
                            _result_temp,
                        File.value,
                        Line,
                        Col,
                        diag::cNullptrUnwrapFormat,
                        Expr.value);
#endif
                }
                return value;
            }
        };

        template <typename T, util::TemplateString Expr, util::TemplateString File, u32 Line, u16 Col>
        struct UnwrapChecker<ValueOrResult<T>, Expr, File, Line, Col> {
            static hk_alwaysinline T check(ValueOrResult<T>&& value) {
                const Result _result_temp = ResultChecker<Result, ValueOrResult<T>, Expr, File, Line, Col>::check(value);

                if (_result_temp.failed()) {
#if defined(HK_RELEASE) and not defined(HK_RELEASE_DEBINFO)
                    diag::abortReleaseImpl<File, Line, Col>(_result_temp);
#else
                    const char* _result_temp_name = diag::getResultName(_result_temp);
                    if (_result_temp_name != nullptr) {
                        diag::abortImpl(
                            HAS_NNSDK(svc::BreakReason_User, )
                                _result_temp,
                            File.value,
                            Line,
                            Col,
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
                            Col,
                            diag::cUnwrapResultFormat,
                            _result_temp.getModule() + 2000,
                            _result_temp.getDescription(),
                            _result_temp.getValue(),
                            Expr.value);
                    }
#endif
                }
                return move((T)value);
            }
        };

        template <typename T, util::TemplateString Expr, util::TemplateString File, u32 Line, u16 Col>
        struct UnwrapChecker<ValueOrResult<T&>, Expr, File, Line, Col> {
            static hk_alwaysinline ValueOrResult<T&> check(ValueOrResult<T&>&& value) {
                const Result _result_temp = ResultChecker<Result, ValueOrResult<T&>, Expr, File, Line, Col>::check(value);

                if (_result_temp.failed()) {
#if defined(HK_RELEASE) and not defined(HK_RELEASE_DEBINFO)
                    diag::abortReleaseImpl<File, Line, Col>(_result_temp);
#else
                    const char* _result_temp_name = diag::getResultName(_result_temp);
                    if (_result_temp_name != nullptr) {
                        diag::abortImpl(
                            HAS_NNSDK(svc::BreakReason_User, )
                                _result_temp,
                            File.value,
                            Line,
                            Col,
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
                            Col,
                            diag::cUnwrapResultFormat,
                            _result_temp.getModule() + 2000,
                            _result_temp.getDescription(),
                            _result_temp.getValue(),
                            Expr.value);
                    }
#endif
                }
                return move(value);
            }
        };

/**
 * @brief Retrieve the value of a ValueOrResult<T>, abort if the Result is unsuccessful.
 *        When VALUE is a pointer, abort if it is nullptr.
 *
 */
#define HK_UNWRAP(VALUE, ...)                                                                                                                                                                   \
    ({                                                                                                                                                                                          \
        auto&& _hk_unwrap_v = VALUE __VA_OPT__(, ) __VA_ARGS__;                                                                                                                                 \
        using _ValueT = ::hk::util::tRemoveReference<decltype(_hk_unwrap_v)>;                                                                                                                   \
        ::hk::detail::UnwrapChecker<_ValueT, #VALUE __VA_OPT__(",") #__VA_ARGS__, __FILE__, __LINE__, ::hk::diag::SourceLocation::current().column()>::check(::forward<_ValueT>(_hk_unwrap_v)); \
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
#define HK_TRY(VALUE, ...)                                                                                                                                                                                                             \
    ({                                                                                                                                                                                                                                 \
        auto&& _value_temp = VALUE __VA_OPT__(, ) __VA_ARGS__;                                                                                                                                                                         \
        using _ValueT = ::hk::util::tRemoveReference<decltype(_value_temp)>;                                                                                                                                                           \
        using _ResultT = ::hk::detail::TryResultType<_ValueT>::Type;                                                                                                                                                                   \
                                                                                                                                                                                                                                       \
        const _ResultT _result_temp = ::hk::detail::ResultChecker<_ResultT, _ValueT, #VALUE __VA_OPT__(",") #__VA_ARGS__, __FILE__, __LINE__, ::hk::diag::SourceLocation::current().column()>::check(::forward<_ValueT>(_value_temp)); \
        if (_result_temp.failed())                                                                                                                                                                                                     \
            return _result_temp;                                                                                                                                                                                                       \
        ::hk::detail::getTryExpressionValue(::move(_value_temp));                                                                                                                                                                      \
    })

} // namespace hk
