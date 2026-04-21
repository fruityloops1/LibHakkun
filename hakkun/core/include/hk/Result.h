#pragma once

#include "hk/types.h"
#include "hk/util/TemplateString.h"

#if HK_RESULT_ADVANCED and defined(HK_RELEASE)
#define HK_RESULT_ADVANCED 0
#endif

#if HK_RESULT_ADVANCED
#include "hk/util/Tuple.h"
#endif

namespace hk {

#if HK_RESULT_ADVANCED
    namespace detail {

        /**
         * @brief Location where a result was constructed or checked.
         * Stored in a global array.
         */
        struct ResultDebugReference {
            using Ref = u16;

            const char* sourceFile = nullptr;
            const char* expr = nullptr;
            u32 sourceLine : 22 = 0;
            Ref parentRef : 10 = 0;

            const ResultDebugReference* getParent() const { return this->parentRef != 0 ? &get(this->parentRef) : nullptr; }
            size calcNumParents() const {
                const ResultDebugReference* next = getParent();
                size numParents = 0;

                while (next != nullptr) {
                    numParents++;
                    next = next->getParent();
                }

                return numParents;
            }

            constexpr static size cMaxDebugRefs = bit(10) - 1;

            static Tuple<Ref, ResultDebugReference&> allocate();
            static ResultDebugReference& get(Ref idx);
        } __attribute__((packed));

    } // namespace detail
#endif

    template <typename T>
    class ValueOrResult;

    struct Result;

    /**
     * @brief Value representing the result of an operation.
     *
     * When the value is ResultSuccess(), the operation was successful.
     * Otherwise, a module and description code can be retrieved to determine the error.
     */
    class ResultBase {

        // BOOOOOOOOOOOORIIIING
        static constexpr u32 makeResult(int module, int description, int debugRef) {
            return (module & bits(9)) | (description & bits(13)) << 9 | (debugRef & bits(10)) << 22;
        }

        static constexpr u32 maskOutDebugRef(u32 value) { return value & bits(22); }

#if HK_RESULT_ADVANCED
        constexpr detail::ResultDebugReference::Ref getDebugRef() const { return this->value >> 22; }

        static constexpr ResultBase makeWithInfo(const ResultBase& value, const detail::ResultDebugReference& inInfo) {
            if (value.getModule() == 0 && value.getModule() == value.getDescription())
                return makeResult(0, 0, 0);

            if consteval {
                return value;
            }

            auto [ref, info] = detail::ResultDebugReference::allocate();

            ResultBase out = makeResult(value.getDescription(), value.getModule(), ref);

            info = inInfo;
            info.parentRef = value.getDebugRef();

            return out;
        }
#endif

    public:
        friend Result;

        u32 value = 0;

        constexpr ResultBase() = default;
        constexpr ResultBase(const ResultBase& other) = default;
        constexpr ResultBase(const Result& other);
        constexpr ResultBase(u32 value)
            : value(value) { }
        constexpr ResultBase(int module, int description)
            : ResultBase(makeResult(module, description, 0)) { }

        constexpr u32 getValue() const { return this->value; }

        constexpr operator u32() const { return getValue(); }
        constexpr operator bool() const { return failed(); }
        template <typename T>
        constexpr operator ValueOrResult<T>() const;
        constexpr ResultBase operator and(const ResultBase& rhs) const { return failed() ? *this : rhs; }

        constexpr int getModule() const { return this->value & bits(9); }
        constexpr int getDescription() const { return (this->value >> 9) & bits(13); }

        constexpr bool operator==(const ResultBase& rhs) const { return rhs.getValue() == this->getValue(); }

        constexpr bool succeeded() const { return this->getValue() == 0; }
        constexpr bool failed() const { return !succeeded(); }
    };

    static_assert(std::is_trivially_copyable_v<ResultBase>);

#if NNSDK
    using ResultNN = ResultBase;
#endif

    struct Result : ResultBase {
        constexpr Result()
            : ResultBase() { }
        constexpr Result(const ResultBase& other)
            : ResultBase(other) { }
        constexpr Result(u32 value)
            : ResultBase(maskOutDebugRef(value)) { }
        constexpr Result(int module, int description)
            : ResultBase(makeResult(module, description, 0)) { }

        constexpr u32 getValue() const { return maskOutDebugRef(this->value); }

        constexpr operator u32() const { return getValue(); }
        using ResultBase::operator bool;
        template <typename T>
        constexpr operator ValueOrResult<T>() const;
        constexpr Result operator and(const Result& rhs) const { return failed() ? *this : rhs; }
        constexpr Result operator and(const ResultBase& rhs) const { return rhs.failed() ? rhs : *this; }

        using ResultBase::getDescription;
        using ResultBase::getModule;

        constexpr bool operator==(const Result& rhs) const { return rhs.getValue() == this->getValue(); }
        constexpr bool operator==(const ResultBase& rhs) const { return rhs.getValue() == this->getValue(); }

        using ResultBase::failed;
        using ResultBase::succeeded;

#if HK_RESULT_ADVANCED
        hk_noinline Result(const ResultBase& value, const char* expr, const char* sourceFile, int line)
            : ResultBase(ResultBase::makeWithInfo(value, { .sourceFile = sourceFile, .expr = expr, .sourceLine = u32(line) })) { }

        const detail::ResultDebugReference* getInfo() const { return this->getDebugRef() != 0 ? &detail::ResultDebugReference::get(this->getDebugRef()) : nullptr; }

        const char* getExpr() const {
            const auto* info = getInfo();
            return info != nullptr ? info->expr : nullptr;
        }

        const char* getFile() const {
            const auto* info = getInfo();
            return info != nullptr ? info->sourceFile : nullptr;
        }

        int getLine() const {
            const auto* info = getInfo();
            return info != nullptr ? info->sourceLine : -1;
        }
#endif
    };

    constexpr ResultBase::ResultBase(const Result& other)
        : value(maskOutDebugRef(other.value)) { }

#if HK_RESULT_ADVANCED
#define MAKE_RESULT_IMPL(VALUE, EXPR, FILE, LINE) ::hk::Result(VALUE, EXPR, FILE, LINE)
#define MAKE_RESULT(VALUE, ...) ::hk::Result(VALUE __VA_OPT__(, ) __VA_ARGS__, #VALUE __VA_OPT__(",") #__VA_ARGS__, __FILE__, __LINE__)
#else
#define MAKE_RESULT_IMPL(VALUE, EXPR, FILE, LINE) ::hk::Result(VALUE)
#define MAKE_RESULT(VALUE, ...) ::hk::Result(VALUE __VA_OPT__(, ) __VA_ARGS__)
#endif

#define MAKE_SUCCESS() ::hk::Result(0)

    template <int Module, int Description>
    class ResultV : public ResultBase {
    public:
        constexpr ResultV()
            : ResultBase(Module, Description) { }

        ResultV(u32 value) = delete;
        ResultV(int module, int description) = delete;
    };

    template <u32 Min, u32 Max>
    struct ResultRange {
        static constexpr bool includes(ResultBase value) {
            u32 intval = value;
            return intval >= Min && intval <= Max;
        }
    };

    template <int Module>
    struct ResultModule {
        constexpr static int cModule = Module;
    };

#define HK_RESULT_MODULE(ID)                   \
    namespace _hk_result_id_namespace {        \
        using Module = ::hk::ResultModule<ID>; \
    }

/**
 * @brief Define a range for result types to be used in current module.
 *
 */
#define HK_DEFINE_RESULT_RANGE(NAME, MIN, MAX) using ResultRange##NAME = ::hk::ResultRange<::hk::ResultV<_hk_result_id_namespace::Module::cModule, MIN>().getValue(), ::hk::ResultV<_hk_result_id_namespace::Module::cModule, MAX>().getValue()>;

#ifndef HK_DEFINE_RESULT
/**
 * @brief Define a result type for the current module.
 *
 */
#define HK_DEFINE_RESULT(NAME, DESCRIPTION) \
    using Result##NAME = ::hk::ResultV<_hk_result_id_namespace::Module::cModule, DESCRIPTION>;
#endif

    template <typename ResultType>
    hk_alwaysinline bool isResult(ResultBase value) {
        return value == ResultType();
    }

#ifndef INCLUDE_HK_DETAIL_DEFAULTRESULTS

#define INCLUDE_HK_DETAIL_DEFAULTRESULTS
#include "hk/detail/DefaultResults.ih"
#undef INCLUDE_HK_DETAIL_DEFAULTRESULTS

    namespace detail {

        template <typename Result, typename T, util::TemplateString Expr, util::TemplateString File, int Line>
        struct ResultChecker {
            hk_alwaysinline static constexpr Result check(T&& value) { return MAKE_RESULT_IMPL(Result(value), Expr.value, File.value, Line); }
        };

        template <typename Result, typename T, util::TemplateString Expr, util::TemplateString File, int Line>
        struct ResultChecker<Result, T*, Expr, File, Line> {
            hk_alwaysinline static constexpr Result check(T* value) {
                if (value != nullptr)
                    return MAKE_RESULT_IMPL(ResultSuccess(), Expr.value, File.value, Line);
                else
                    return MAKE_RESULT_IMPL(ResultNoValue(), Expr.value, File.value, Line);
            }
        };

        template <typename T>
        struct TryResultType {
            using Type = Result;
        };

        template <>
        struct TryResultType<ResultBase> {
            using Type = ResultBase;
        };

    } // namespace detail

#endif // INCLUDE_HK_DETAIL_DEFAULTRESULTS

/**
 * @brief Return if Result within expression is unsuccessful.
 * If expression is pointer, return ResultNoValue() if it is nullptr.
 * Function must return Result.
 */
#define HK_TRY(VALUE, ...)                                                                                                                                                             \
    ({                                                                                                                                                                                 \
        auto&& _value_temp = VALUE __VA_OPT__(, ) __VA_ARGS__;                                                                                                                         \
        using _ValueT = std::remove_reference_t<decltype(_value_temp)>;                                                                                                                \
        using _ResultT = ::hk::detail::TryResultType<_ValueT>::Type;                                                                                                                   \
                                                                                                                                                                                       \
        const _ResultT _result_temp = ::hk::detail::ResultChecker<_ResultT, _ValueT, #VALUE __VA_OPT__(",") #__VA_ARGS__, __FILE__, __LINE__>::check(::forward<_ValueT>(_value_temp)); \
        if (_result_temp.failed())                                                                                                                                                     \
            return _result_temp;                                                                                                                                                       \
        ::move(_value_temp);                                                                                                                                                           \
    })

/**
 * @brief Return if Result within expression is unsuccessful.
 * Function must return Result.
 */
#define HK_CHECK(VALUE, ...)                                                                                                                                                           \
    {                                                                                                                                                                                  \
        auto&& _value_temp = VALUE __VA_OPT__(, ) __VA_ARGS__;                                                                                                                         \
        using _ValueT = std::remove_reference_t<decltype(_value_temp)>;                                                                                                                \
        using _ResultT = ::hk::detail::TryResultType<_ValueT>::Type;                                                                                                                   \
                                                                                                                                                                                       \
        const _ResultT _result_temp = ::hk::detail::ResultChecker<_ResultT, _ValueT, #VALUE __VA_OPT__(",") #__VA_ARGS__, __FILE__, __LINE__>::check(::forward<_ValueT>(_value_temp)); \
        if (_result_temp.failed())                                                                                                                                                     \
            return _result_temp;                                                                                                                                                       \
    }

/**
 * @brief Return a Result expression if CONDITION is false.
 * Function must return Result.
 */
#define HK_UNLESS(CONDITION, RESULT)                           \
    {                                                          \
        const bool _condition_temp = (CONDITION);              \
        const ::hk::Result _result_temp = MAKE_RESULT(RESULT); \
        if (_condition_temp == false)                          \
            return _result_temp;                               \
    }

} // namespace hk
