#pragma once

#include "hk/prim/traits/Integer.h"
#include "hk/prim/traits/Type.h"
#include "hk/util/Algorithm.h"
#include <array>

namespace hk::util {

    struct CustomTypeName { };

    namespace detail {

        template <typename T>
        struct TypeNameExtractor {
            constexpr static /* breaks when consteval is used? */ const auto& getTypeNameData() {
                constexpr static const char* cPrettyFunctionData = __PRETTY_FUNCTION__;
                constexpr static size cPrettyFunctionDataLen = __builtin_strlen(cPrettyFunctionData);
                constexpr static auto dataArr = ([]() {
                    std::array<char, cPrettyFunctionDataLen + 1> data { '\0' };

                    const char* start = cPrettyFunctionData;
                    while (!isEqualString(++start, "T = ", 4))
                        ;
                    start += 4;

                    const char* end = start;
                    while (*++end != ']')
                        ;

                    size len = end - start;

                    util::copy(data.data(), start, len);
                    return data;
                })();

                return dataArr;
            }
        };

        template <typename T>
            requires ctIsBaseOf<CustomTypeName, T>
        struct TypeNameExtractor<T> {
            constexpr static const auto& getTypeNameData() {
                constexpr static std::array data { T::cTypeName };
                return data;
            }
        };

    } // namespace detail

    /**
     * @brief Return name of type T as std::array<char>.
     *
     * @tparam T Type
     */
    template <typename T>
    constexpr const auto& getTypeNameData() {
        return detail::TypeNameExtractor<T>::getTypeNameData();
    }

    /**
     * @brief Return name of type T.
     *
     * @tparam T Type
     */
    template <typename T>
    constexpr /* breaks when consteval is used? */ const char* getTypeName() {
        const char* value = (const char*)(getTypeNameData<T>().data());
        return value;
    }

    static_assert(isEqualString("int", getTypeName<int>()));
    static_assert(isEqualString("std::array<int, 4>", getTypeName<std::array<int, 4>>()));
#ifdef __clang__
    static_assert(isEqualString("const char *", getTypeName<const char*>()));
#else
    static_assert(isEqualString("const char*", getTypeName<const char*>()));
#endif

} // namespace hk::util
