#pragma once

#include "hk/types.h"
#include <array>
#include <type_traits>

namespace hk::util {

    struct CustomTypeName { };

    namespace detail {

        template <typename T>
        struct TypeNameExtractor {
            constexpr static /* breaks when consteval is used? */ const auto& getTypeNameData() {
#ifdef __clang__
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

                return dataArr;
#endif
            }
        };

        template <typename T>
            requires std::is_base_of_v<CustomTypeName, T>
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

    static_assert(__builtin_strcmp("int", getTypeName<int>()) == 0);
    static_assert(__builtin_strcmp("const char *", getTypeName<const char*>()) == 0);
    static_assert(__builtin_strcmp("std::array<int, 4>", getTypeName<std::array<int, 4>>()) == 0);

} // namespace hk::util
