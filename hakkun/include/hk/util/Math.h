#pragma once

#include <cmath>
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

    template <typename T, size N>
    constexpr size arraySize(T (&array)[N]) {
        return N;
    }

    template <typename T>
    struct Vector2 {
        T x = 0, y = 0;

        Vector2() = default;
        Vector2(T x, T y)
            : x(x)
            , y(y) { }

        Vector2 operator+(const Vector2& rhs) const {
            return { x + rhs.x, y + rhs.y };
        }
        Vector2 operator-(const Vector2& rhs) const {
            return { x - rhs.x, y - rhs.y };
        }
        Vector2 operator*(const Vector2& rhs) const {
            return { x * rhs.x, y * rhs.y };
        }
        Vector2 operator/(const Vector2& rhs) const {
            return { x / rhs.x, y / rhs.y };
        }

        Vector2& operator+=(const Vector2& rhs) {
            x += rhs.x;
            y += rhs.y;
            return *this;
        }
        Vector2& operator-=(const Vector2& rhs) {
            x -= rhs.x;
            y -= rhs.y;
            return *this;
        }
        Vector2& operator*=(const Vector2& rhs) {
            x *= rhs.x;
            y *= rhs.y;
            return *this;
        }
        Vector2& operator/=(const Vector2& rhs) {
            x /= rhs.x;
            y /= rhs.y;
            return *this;
        }

        Vector2 operator+(T v) {
            return { x + v, y + v };
        }
        Vector2 operator-(T v) {
            return { x - v, y - v };
        }
        Vector2 operator*(T v) {
            return { x * v, y * v };
        }
        Vector2 operator/(T v) {
            return { x / v, y / v };
        }

        Vector2& operator+=(T v) {
            x += v;
            y += v;
            return *this;
        }
        Vector2& operator-=(T v) {
            x -= v;
            y -= v;
            return *this;
        }
        Vector2& operator*=(T v) {
            x *= v;
            y *= v;
            return *this;
        }
        Vector2& operator/=(T v) {
            x /= v;
            y /= v;
        }

        T length() {
            return std::sqrt(x * x + y * y);
        }

        Vector2& normalize() {
            const T len = length();
            if (len > 0) {
                const T invLen = 1 / len;
                *this *= invLen;
            }

            return *this;
        }

        operator Vector2<f32>() const {
            return Vector2<f32>(f32(x), f32(y));
        }
        operator Vector2<f64>() const {
            return Vector2<f64>(f64(x), f64(y));
        }
        operator Vector2<int>() const {
            return Vector2<int>(int(x), int(y));
        }
    };

    using Vector2f = Vector2<f32>;
    using Vector2f64 = Vector2<f64>;
    using Vector2i = Vector2<int>;

} // namespace hk::util
