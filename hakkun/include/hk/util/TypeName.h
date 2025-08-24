#pragma once

#include "hk/types.h"
#include <array>

namespace hk::util {

    template <typename T>
    constexpr /* breaks when consteval is used? */ const char* getTypeName() {
        constexpr static const char* cPrettyFunctionData = __PRETTY_FUNCTION__;
        constexpr static size cPrettyFunctionDataLen = __builtin_strlen(cPrettyFunctionData);
        constexpr static auto dataArr = ([]() {
            std::array<char, cPrettyFunctionDataLen + 1> data { '\0' };

            const char* start = cPrettyFunctionData;
            while (__builtin_strncmp(++start, "T = ", 4) != 0)
                ;
            start += 4;

            const char* end = start;
            while (*++end != ']')
                ;

            size len = end - start;

            __builtin_memcpy(data.data(), start, len);
            return data;
        })();

        return dataArr.data();
    }

    static_assert(__builtin_strcmp("int", getTypeName<int>()) == 0);
    static_assert(__builtin_strcmp("const char *", getTypeName<const char*>()) == 0);
    static_assert(__builtin_strcmp("std::array<int, 4>", getTypeName<std::array<int, 4>>()) == 0);

} // namespace hk::util
