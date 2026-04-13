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
        requires std::is_copy_constructible_v<T>
        and std::is_destructible_v<T>
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
        requires std::is_copy_constructible_v<T>
        and std::is_destructible_v<T>
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
        requires std::is_move_constructible_v<T>
        and std::is_destructible_v<T>
    constexpr void move(T* dest, T* src, size amount) {
        if (!std::is_constant_evaluated() and std::is_trivially_move_constructible_v<T> and std::is_trivially_destructible_v<T>)
            __builtin_memcpy(dest, src, amount * sizeof(T));
        else
            for (size i = 0; i < amount; i++) {
                dest[i].~T();
                construct_at(dest + i, move(src[i]));
            }
    }

    template <bool DestroyMovedFrom = false, typename T>
        requires std::is_move_constructible_v<T>
    constexpr void constructMove(T* dest, T* src, size amount) {
        if (!std::is_constant_evaluated() and std::is_trivially_move_constructible_v<T>)
            __builtin_memcpy(dest, src, amount * sizeof(T));
        else
            for (size i = 0; i < amount; i++) {
                T* to = dest + i;
                T* from = src + i;
                construct_at(dest + i, move(*from));
                if constexpr (DestroyMovedFrom)
                    from->~T();
            }
    }

    template <bool DestroyMovedFrom = false, typename T>
        requires std::is_move_constructible_v<T>
        and std::is_destructible_v<T>
    constexpr void constructMoveOverlapping(T* dest, T* src, size amount) {
        if (!std::is_constant_evaluated() and std::is_trivially_move_constructible_v<T> and std::is_trivially_destructible_v<T>)
            __builtin_memmove(dest, src, amount * sizeof(T));
        else {
            if (dest < src)
                for (size i = 0; i < amount; i++) {
                    T* to = dest + i;
                    T* from = src + i;
                    construct_at(to, move(*from));
                    if constexpr (DestroyMovedFrom)
                        from->~T();
                }
            else
                for (size i = amount; i != 0; i--) {
                    T* to = dest + i - 1;
                    T* from = src + i - 1;
                    construct_at(to, move(*from));
                    if constexpr (DestroyMovedFrom)
                        from->~T();
                }
        }
    }

    template <typename T>
        requires std::is_move_constructible_v<T>
    constexpr void constructCopy(T* dest, const T* src, size amount) {
        if (!std::is_constant_evaluated() and std::is_trivially_copy_constructible_v<T>)
            __builtin_memcpy(dest, src, amount * sizeof(T));
        else
            for (size i = 0; i < amount; i++)
                construct_at(dest + i, src[i]);
    }

    template <typename T>
        requires std::is_copy_constructible_v<T>
        and std::is_destructible_v<T>
    constexpr void reverseCopy(T* dest, size amount) {
        for (size i = 0; i < amount / 2; i++) {
            T temp = dest[i];

            dest[i].~T();
            construct_at(dest + i, dest[amount - i - 1]);
            dest[amount - i - 1].~T();
            construct_at(dest + amount - i - 1, temp);
        }
    }

    template <typename T>
        requires std::is_move_constructible_v<T>
        and std::is_destructible_v<T>
    constexpr void reverseMove(T* dest, size amount) {
        for (size i = 0; i < amount / 2; i++) {
            T temp = T(forward<T>(dest[i]));

            dest[i].~T();
            construct_at(dest + i, forward<T>(dest[amount - i - 1]));
            dest[amount - i - 1].~T();
            construct_at(dest + forward<T>(amount - i - 1, temp));
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

    template <typename T>
    constexpr void construct(T* dest, size amount, const T& data) {
        for (size i = 0; i < amount; i++)
            construct_at(dest + i, data);
    }

    template <typename T, typename... Args>
    constexpr void construct(T* dest, size amount, Args&&... args) {
        for (size i = 0; i < amount; i++)
            construct_at(dest + i, forward<Args>()...);
    }

    template <typename T>
        requires std::is_destructible_v<T>
    constexpr void destroy(T* dest, size amount) {
        for (size i = 0; i < amount; i++)
            dest[i].~T();
    }

    template <typename T>
    constexpr bool isEqual(const T* a, const T* b, size amount) {
        for (size i = 0; i < amount; i++)
            if (*a++ != *b++)
                return false;
        return true;
    }

} // namespace hk::util
