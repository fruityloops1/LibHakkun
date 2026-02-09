#pragma once

#include "hk/types.h"

namespace hk::util {

    /**
     * @brief Binary search algorithm.
     *
     * @tparam T
     * @tparam GetFunc Function to get value from given array
     * @tparam FindBetween whether to return -1 or in between index with no exact match
     * @param get
     * @param low
     * @param high
     * @param searchValue
     * @return s32
     */
    template <bool FindBetweenIdx = false, typename T, typename GetFunc>
    s32 binarySearch(GetFunc get, fs32 low, fs32 high, T searchValue) {
        while (low <= high) {
            fs32 mid = (low + high) / 2;
            if (get(mid) == searchValue)
                return mid;

            if (get(mid) < searchValue)
                low = mid + 1;
            else
                high = mid - 1;
        }

        return FindBetweenIdx ? low : -1;
    }

} // namespace hk::util
