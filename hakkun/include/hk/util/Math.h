#pragma once

#include "hk/types.h"
#include <cmath>

namespace hk::util {

    // Misc

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

    // Vectors

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

    template <typename T>
    struct Vector3 {
        T x = 0, y = 0, z = 0;

        Vector3() = default;
        Vector3(T x, T y, T z)
            : x(x)
            , y(y)
            , z(z) { }

        Vector3 operator+(const Vector3& rhs) const {
            return { x + rhs.x, y + rhs.y, z + rhs.z };
        }
        Vector3 operator-(const Vector3& rhs) const {
            return { x - rhs.x, y - rhs.y, z - rhs.z };
        }
        Vector3 operator*(const Vector3& rhs) const {
            return { x * rhs.x, y * rhs.y, z * rhs.z };
        }
        Vector3 operator/(const Vector3& rhs) const {
            return { x / rhs.x, y / rhs.y, z / rhs.z };
        }

        Vector3& operator+=(const Vector3& rhs) {
            x += rhs.x;
            y += rhs.y;
            z += rhs.z;
            return *this;
        }
        Vector3& operator-=(const Vector3& rhs) {
            x -= rhs.x;
            y -= rhs.y;
            z -= rhs.z;
            return *this;
        }
        Vector3& operator*=(const Vector3& rhs) {
            x *= rhs.x;
            y *= rhs.y;
            z *= rhs.z;
            return *this;
        }
        Vector3& operator/=(const Vector3& rhs) {
            x /= rhs.x;
            y /= rhs.y;
            z /= rhs.z;
            return *this;
        }

        Vector3 operator+(T v) {
            return { x + v, y + v, z + v };
        }
        Vector3 operator-(T v) {
            return { x - v, y - v, z - v };
        }
        Vector3 operator*(T v) {
            return { x * v, y * v, z * v };
        }
        Vector3 operator/(T v) {
            return { x / v, y / v, z / v };
        }

        Vector3& operator+=(T v) {
            x += v;
            y += v;
            z += v;
            return *this;
        }
        Vector3& operator-=(T v) {
            x -= v;
            y -= v;
            z -= v;
            return *this;
        }
        Vector3& operator*=(T v) {
            x *= v;
            y *= v;
            z *= v;
            return *this;
        }
        Vector3& operator/=(T v) {
            x /= v;
            y /= v;
            z /= v;
        }

        T length() {
            return std::sqrt(x * x + y * y + z * z);
        }

        Vector3& normalize() {
            const T len = length();
            if (len > 0) {
                const T invLen = 1 / len;
                *this *= invLen;
            }

            return *this;
        }

        operator Vector3<f32>() const {
            return Vector3<f32>(f32(x), f32(y), f32(z));
        }
        operator Vector3<f64>() const {
            return Vector3<f64>(f64(x), f64(y), f64(z));
        }
        operator Vector3<int>() const {
            return Vector3<int>(int(x), int(y), int(z));
        }
    };

    using Vector3f = Vector3<f32>;
    using Vector3f64 = Vector3<f64>;
    using Vector3i = Vector3<int>;

    // Bit math

    template <typename T>
    class IntBuilder {
        T mValue = 0;

    public:
        constexpr IntBuilder& set(int upperIdx, int lowerIdx, T value) {
            mValue |= (value << lowerIdx) & ((1ull << (upperIdx + 1)) - (1ull << lowerIdx));
            return *this;
        }

        constexpr operator T() const { return mValue; }
    };

    template <typename T>
    constexpr int countSetBits(T n) {
        int count = 0;
        while (n) {
            count += n & 1;
            n >>= 1;
        }
        return count;
    }

    template <typename T>
    constexpr bool isRepeatingBitPattern(T value, int elemSize) {
        if (elemSize == sizeof(T) * 8)
            return true;

        T pattern = value & ((T(1) << elemSize) - 1);
        for (int i = elemSize; i < sizeof(T) * 8; i += elemSize) {
            if ((value & ((T(1) << elemSize) - 1)) != pattern) {
                return false;
            }
            value >>= elemSize;
        }
        return true;
    }

    template <typename T>
    constexpr int countTrailingOnes(T value) {
        int count = 0;
        while ((value & 1) && count < sizeof(T) * 8) {
            count++;
            value >>= 1;
        }
        return count;
    }

    template <typename T>
    constexpr int countLeadingZeros(T value, int width) {
        int count = 0;
        for (int i = width - 1; i >= 0; i--) {
            if ((value & (T(1) << i)) == 0) {
                count++;
            } else {
                break;
            }
        }
        return count;
    }

    template <typename T>
    constexpr int countLeadingZeros(T value) {
        return __builtin_clz(value);
    }

    template <typename T>
    constexpr int calcHighestSetBit(T value) {
        for (int i = sizeof(T) * 8 - 1; i >= 0; i--) {
            if (value & (T(1) << i)) {
                return i;
            }
        }
        return -1;
    }

    template <typename T>
    constexpr T ror(T value, int shift, int width) {
        shift = shift % width;
        if (shift == 0)
            return value;

        T mask = (T(1) << width) - 1;
        value &= mask;
        return ((value >> shift) | (value << (width - shift))) & mask;
    }

    template <size Width, size NumShift>
    constexpr bool isRepresentable(u64 value) {
        for (size i = 0; i < NumShift; i++) {
            if ((value & (bits(Width) << (i * Width))) == value) {
                return true;
            }
        }
        return false;
    }

} // namespace hk::util
