#pragma once

#include "hk/prim/traits/Integer.h"
#include <memory>

using std::construct_at;
using std::forward;
using std::move;

namespace hk::util {

    template <typename T>
    constexpr T min(T first) {
        return first;
    }

    template <typename T, typename... Args>
    constexpr T min(T first, Args... args) {
        T minOfRest = min(args...);
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

    template <typename T, size N>
    constexpr size arraySize(T (&array)[N]) {
        return N;
    }

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
     * @return size
     */
    template <typename T, LambdaType GetFunc>
    constexpr size binarySearch(GetFunc get, s32 low, s32 high, T searchValue, bool findBetween = false) {
        while (low <= high) {
            int_fast32_t mid = (low + high) / 2;
            if (get(mid) == searchValue)
                return mid;

            if (get(mid) < searchValue)
                low = mid + 1;
            else
                high = mid - 1;
        }

        return findBetween ? low : -1;
    }

    namespace detail {

        template <typename T>
        using DefaultCompare = decltype([](const T& a, const T& b) -> bool { return a < b; });

    } // namespace detail

    template <typename T, LambdaType Compare = detail::DefaultCompare<T>>
    constexpr void insertionSort(T* values, size numValues, Compare cmp = Compare()) {
        for (size rhsIdx = 1; rhsIdx < numValues; rhsIdx++) {
            T rhs = T(forward<T>(values[rhsIdx]));
            ssize lhsIdx = rhsIdx - 1;

            while (lhsIdx >= 0 && !cmp(values[lhsIdx], rhs)) {
                values[lhsIdx + 1].~T();
                construct_at(values + 1 + lhsIdx, forward<T>(values[lhsIdx]));

                lhsIdx--;
            }

            values[lhsIdx + 1].~T();
            construct_at(values + 1 + lhsIdx, forward<T>(rhs));
        }
    }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnontrivial-memcall"

    template <typename T>
        requires util::ctIsCopyConstructible<T>
        and util::ctIsDestructible<T>
    constexpr void copy(T* dest, const T* src, size amount) {
        if (!std::is_constant_evaluated() and util::ctIsTriviallyCopyConstructible<T> and util::ctIsTriviallyDestructible<T>)
            __builtin_memcpy(dest, src, amount * sizeof(T));
        else {
            for (size i = 0; i < amount; i++) {
                dest[i].~T();
                construct_at(dest + i, src[i]);
            }
        }
    }

    template <typename T>
        requires util::ctIsCopyConstructible<T>
        and util::ctIsDestructible<T>
    constexpr void copyOverlapping(T* dest, const T* src, size amount) {
        if (!std::is_constant_evaluated() and util::ctIsTriviallyCopyConstructible<T> and util::ctIsTriviallyDestructible<T>)
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
        requires util::ctIsMoveConstructible<T>
        and util::ctIsDestructible<T>
    constexpr void move(T* dest, T* src, size amount) {
        if (!std::is_constant_evaluated() and util::ctIsTriviallyMoveConstructible<T> and util::ctIsTriviallyDestructible<T>)
            __builtin_memcpy(dest, src, amount * sizeof(T));
        else
            for (size i = 0; i < amount; i++) {
                dest[i].~T();
                construct_at(dest + i, move(src[i]));
            }
    }

    template <bool DestroyMovedFrom = false, typename T>
        requires util::ctIsMoveConstructible<T>
    constexpr void constructMove(T* dest, T* src, size amount) {
        if (!std::is_constant_evaluated() and util::ctIsTriviallyMoveConstructible<T>)
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
        requires util::ctIsMoveConstructible<T>
        and util::ctIsDestructible<T>
    constexpr void constructMoveOverlapping(T* dest, T* src, size amount) {
        if (!std::is_constant_evaluated() and util::ctIsTriviallyMoveConstructible<T> and util::ctIsTriviallyDestructible<T>)
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
        requires util::ctIsCopyConstructible<T>
    constexpr void constructCopy(T* dest, const T* src, size amount) {
        if (!std::is_constant_evaluated() and util::ctIsTriviallyCopyConstructible<T>)
            __builtin_memcpy(dest, src, amount * sizeof(T));
        else
            for (size i = 0; i < amount; i++)
                construct_at(dest + i, src[i]);
    }

    template <typename T>
        requires util::ctIsCopyConstructible<T>
        and util::ctIsDestructible<T>
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
        requires util::ctIsMoveConstructible<T>
        and util::ctIsDestructible<T>
    constexpr void reverseMove(T* dest, size amount) {
        for (size i = 0; i < amount / 2; i++) {
            T temp = T(forward<T>(dest[i]));

            dest[i].~T();
            construct_at(dest + i, forward<T>(dest[amount - i - 1]));
            dest[amount - i - 1].~T();
            construct_at(dest + amount - i - 1, forward<T>(temp));
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
    constexpr void copyFromNullTerminated(T* dest, const T* src, size max = -1) {
        size i = 0;
        for (; i < max && src[i] != T(0); i++)
            dest[i] = src[i];
        if (i == max - 1)
            dest[i] = T(0);
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
        requires util::ctIsDestructible<T>
    constexpr void destroy(T* dest, size amount) {
        for (size i = 0; i < amount; i++)
            dest[i].~T();
    }

    template <typename T>
        requires util::ctIsDestructible<T>
    constexpr void destroy(T* dest) {
        dest->~T();
    }

    template <typename T>
    constexpr bool isEqual(const T* a, const T* b, size amount) {
        for (size i = 0; i < amount; i++)
            if (*a++ != *b++)
                return false;
        return true;
    }

    template <typename T>
        requires util::ctIsCopyConstructible<T>
    constexpr void swap(T& a, T& b) {
        T tmp = a;
        a = b;
        b = tmp;
    }

    template <typename T, typename L>
        requires util::ctIsMoveConstructible<T>
    constexpr void transform(T* dest, size amount, L&& func) {
        for (size i = 0; i < amount; i++) {
            T tmp = move(func(dest[i]));
            dest[i].~T();
            construct_at(dest + i, move(tmp));
        }
    }

} // namespace hk::util
