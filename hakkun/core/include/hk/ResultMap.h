#pragma once

#include "hk/Result.h"
#include "hk/ValueOrResult.h"
#include "hk/util/Tuple.h"

namespace hk {

    template <typename T, Tuple<T, Result>... Map>
    struct ResultMap : public Result {
        constexpr ResultMap(const T& value)
            : Result(toResult(value)) {
        }

        static constexpr Result toResult(const T& value) {
            for (const Tuple<T, Result>& pair : { Map... })
                if (pair.a == value)
                    return pair.b;
            return ResultUnknown();
        }

        static constexpr ValueOrResult<T> toValue(Result result) {
            for (const Tuple<T, Result>& pair : { Map... })
                if (pair.b == result)
                    return pair.a;
            return ResultUnknown();
        }
    };

} // namespace hk
