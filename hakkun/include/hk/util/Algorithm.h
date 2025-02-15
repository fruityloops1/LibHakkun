#pragma once

#include "hk/types.h"

namespace hk::util {

    template <typename T, typename GetFunc>
    s32 binarySearch(GetFunc get, fu32 low, fu32 high, T searchValue) {
        while (low <= high) {
            fu32 mid = (low + high) / 2;
            if (get(mid) == searchValue)
                return mid;

            if (get(mid) < searchValue)
                low = mid + 1;
            else
                high = mid - 1;
        }

        return -1;
    }

} // namespace hk::util
