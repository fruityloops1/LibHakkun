#pragma once

#include "hk/types.h"

namespace hk {

    /**
     * @brief Value representing the result of an operation.
     *
     * When the value is ResultSuccess(), the operation was successful.
     * Otherwise, a module and description code can be retrieved to determine the error.
     */
    class Result {
        u32 mValue = 0;

        // BOOOOOOOOOOOORIIIING
        static constexpr u32 makeResult(int module, int description) {
            return (module & 0b0111111111) | ((description) & 0b01111111111111) << 9;
        }

    public:
        constexpr Result()
            : mValue(0) { }
        constexpr Result(u32 value)
            : mValue(value) { }
        constexpr Result(int module, int description)
            : mValue(makeResult(module, description)) { }

        constexpr u32 getValue() const { return mValue; }
        constexpr operator u32() const { return mValue; }
        constexpr operator bool() const { return failed(); }

        constexpr int getModule() const { return mValue & 0b0111111111; }
        constexpr int getDescription() const { return ((mValue) >> 9) & 0b01111111111111; }

        constexpr bool operator==(const Result& rhs) const { return rhs.mValue == mValue; }

        constexpr bool succeeded() const { return mValue == 0; }
        constexpr bool failed() const { return !succeeded(); }
    };

    template <int Module, int Description>
    class ResultV : public Result {
    public:
        constexpr ResultV()
            : Result(Module, Description) { }

        ResultV(u32 value) = delete;
        ResultV(int module, int description) = delete;
    };

    template <u32 Min, u32 Max>
    struct ResultRange {
        static constexpr bool includes(Result value) {
            u32 intval = value;
            return intval >= Min && intval <= Max;
        }
    };

    /**
     * @brief Successful Result.
     *
     */
    using ResultSuccess = ResultV<0, 0>;

#define HK_RESULT_MODULE(ID)            \
    namespace _hk_result_id_namespace { \
        constexpr int module = ID;      \
    }

/**
 * @brief Define a range for result types to be used in current module.
 *
 */
#define HK_DEFINE_RESULT_RANGE(NAME, MIN, MAX) using ResultRange##NAME = ::hk::ResultRange<::hk::ResultV<_hk_result_id_namespace::module, MIN>().getValue(), ::hk::ResultV<_hk_result_id_namespace::module, MAX>().getValue()>;

/**
 * @brief Define a result type for the current module.
 *
 */
#define HK_DEFINE_RESULT(NAME, DESCRIPTION) \
    using Result##NAME = ::hk::ResultV<_hk_result_id_namespace::module, DESCRIPTION>;

    template <typename ResultType>
    hk_alwaysinline bool isResult(Result value) {
        return value == ResultType();
    }

/**
 * @brief Return if Result within expression is unsuccessful.
 * Function must return Result.
 */
#define HK_TRY(RESULT)                            \
    {                                             \
        const ::hk::Result _result_temp = RESULT; \
        if (_result_temp.failed())                \
            return _result_temp;                  \
    }

/**
 * @brief Return a Result expression if CONDITION is false.
 * Function must return Result.
 */
#define HK_UNLESS(CONDITION, RESULT)              \
    {                                             \
        const bool _condition_temp = (CONDITION); \
        const ::hk::Result _result_temp = RESULT; \
        if (_condition_temp == false)             \
            return _result_temp;                  \
    }

} // namespace hk
