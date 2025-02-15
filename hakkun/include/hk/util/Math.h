#pragma once

#include "hk/types.h"

namespace hk::util {

    template <typename T>
    constexpr T min(T first) {
        return first;
    }

    template <typename T, typename... Args>
    constexpr T min(T first, Args... args) {
        T minOfRest = max(args...);
        return (first < minOfRest) ? first : minOfRest;
    }

    template <typename T>
    constexpr T max(T first) {
        return first;
    }

    template <typename T, typename... Args>
    constexpr T max(T first, Args... args) {
        T maxOfRest = max(args...);
        return (first > maxOfRest) ? first : maxOfRest;
    }

} // namespace hk::util
