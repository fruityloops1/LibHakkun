#pragma once

#include "hk/types.h"

namespace hk::util {

    /**
     * @brief Binary search algorithm.
     *
     * @tparam T
     * @tparam GetFunc Function to get value from given array
     * @param get
     * @param low
     * @param high
     * @param searchValue
     * @param findBetween whether to return -1 or in between index with no exact match
     * @return s32
     */
    template <typename T, typename GetFunc>
    size binarySearch(GetFunc get, fs32 low, fs32 high, T searchValue, bool findBetween = false) {
        while (low <= high) {
            fs32 mid = (low + high) / 2;
            if (get(mid) == searchValue)
                return mid;

            if (get(mid) < searchValue)
                low = mid + 1;
            else
                high = mid - 1;
        }

        return findBetween ? low : -1;
    }

} // namespace hk::util
