#pragma once

#include "hk/types.h"
#include <memory>

using std::construct_at;

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
    constexpr size binarySearch(GetFunc get, fs32 low, fs32 high, T searchValue, bool findBetween = false) {
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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnontrivial-memcall"

    template <typename T>
    constexpr void copy(T* dest, const T* src, size amount) {
        if (!std::is_constant_evaluated() and std::is_trivially_move_constructible_v<T> and std::is_trivially_destructible_v<T>)
            __builtin_memcpy(dest, src, amount * sizeof(T));
        else {
            for (size i = 0; i < amount; i++) {
                dest[i].~T();
                construct_at(dest + i, src[i]);
            }
        }
    }

    template <typename T>
    constexpr void copyOverlapping(T* dest, const T* src, size amount) {
        if (!std::is_constant_evaluated() and std::is_trivially_move_constructible_v<T> and std::is_trivially_destructible_v<T>)
            __builtin_memmove(dest, src, amount * sizeof(T));
        else {
            if (dest < src)
                for (size i = 0; i < amount; i++) {
                    T* to = dest + i;
                    const T* from = src + i;
                    to->~T();
                    construct_at(to, *from);
                }
            else
                for (size i = amount; i != 0; i--) {
                    T* to = dest + i - 1;
                    const T* from = src + i - 1;
                    to->~T();
                    construct_at(to, *from);
                }
        }
    }

    using ::move;
    template <typename T>
    constexpr void move(T* dest, T* src, size amount) {
        if (!std::is_constant_evaluated() and std::is_trivially_move_constructible_v<T> and std::is_trivially_destructible_v<T>)
            __builtin_memcpy(dest, src, amount * sizeof(T));
        else
            for (size i = 0; i < amount; i++) {
                dest[i].~T();
                construct_at(dest + i, forward<T>(src[i]));
            }
    }

    template <typename T>
    constexpr void moveOverlapping(T* dest, T* src, size amount) {
        if (!std::is_constant_evaluated() and std::is_trivially_move_constructible_v<T> and std::is_trivially_destructible_v<T>)
            __builtin_memmove(dest, src, amount * sizeof(T));
        else {
            if (dest < src)
                for (size i = 0; i < amount; i++) {
                    T* to = dest + i;
                    T* from = src + i;
                    to->~T();
                    construct_at(to, forward<T>(*from));
                }
            else
                for (size i = amount; i != 0; i--) {
                    T* to = dest + i - 1;
                    T* from = src + i - 1;
                    to->~T();
                    construct_at(to, forward<T>(*from));
                }
        }
    }
#pragma clang diagnostic pop

    template <typename T>
    constexpr void fill(T* dest, size amount, const T& value = T()) {
        for (size i = 0; i < amount; i++) {
            dest[i].~T();
            construct_at(dest + i, value);
        }
    }

    namespace detail {

        template <typename T>
        constexpr void construct(T* dest, size amount, const T& value) {
            for (size i = 0; i < amount; i++)
                construct_at(dest + i, value);
        }

        template <typename T, typename... Args>
        constexpr void construct(T* dest, size amount, Args&&... args) {
            for (size i = 0; i < amount; i++)
                construct_at(dest + i, forward<Args>()...);
        }

    } // namespace detail

    template <class T, class... Args>
    constexpr void construct(T* dest, size amount, Args&&... args) {
        detail::construct(dest, amount, forward<Args>(args)...);
    }

    template <typename T>
    constexpr void destroy(T* dest, size amount) {
        for (size i = 0; i < amount; i++)
            dest[i].~T();
    }

} // namespace hk::util
