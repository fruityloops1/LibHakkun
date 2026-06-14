#pragma once

#include "hk/types.h"
#include <cmath>

namespace hk::util {

    // Vectors

    template <typename T>
    struct Vector2 {
        T x = 0, y = 0;

        constexpr Vector2() = default;
        constexpr Vector2(T x, T y)
            : x(x)
            , y(y) { }

        constexpr bool operator==(const Vector2& rhs) const {
            return x == rhs.x && y == rhs.y;
        }

        constexpr Vector2 operator+(const Vector2& rhs) const {
            return { x + rhs.x, y + rhs.y };
        }
        constexpr Vector2 operator-(const Vector2& rhs) const {
            return { x - rhs.x, y - rhs.y };
        }
        constexpr Vector2 operator*(const Vector2& rhs) const {
            return { x * rhs.x, y * rhs.y };
        }
        constexpr Vector2 operator/(const Vector2& rhs) const {
            return { x / rhs.x, y / rhs.y };
        }

        constexpr Vector2& operator+=(const Vector2& rhs) {
            x += rhs.x;
            y += rhs.y;
            return *this;
        }
        constexpr Vector2& operator-=(const Vector2& rhs) {
            x -= rhs.x;
            y -= rhs.y;
            return *this;
        }
        constexpr Vector2& operator*=(const Vector2& rhs) {
            x *= rhs.x;
            y *= rhs.y;
            return *this;
        }
        constexpr Vector2& operator/=(const Vector2& rhs) {
            x /= rhs.x;
            y /= rhs.y;
            return *this;
        }

        constexpr Vector2 operator+(T v) {
            return { x + v, y + v };
        }
        constexpr Vector2 operator-(T v) {
            return { x - v, y - v };
        }
        constexpr Vector2 operator*(T v) {
            return { x * v, y * v };
        }
        constexpr Vector2 operator/(T v) {
            return { x / v, y / v };
        }

        constexpr Vector2& operator+=(T v) {
            x += v;
            y += v;
            return *this;
        }
        constexpr Vector2& operator-=(T v) {
            x -= v;
            y -= v;
            return *this;
        }
        constexpr Vector2& operator*=(T v) {
            x *= v;
            y *= v;
            return *this;
        }
        constexpr Vector2& operator/=(T v) {
            x /= v;
            y /= v;
        }

        constexpr T length() {
            return std::sqrt(x * x + y * y);
        }

        constexpr Vector2& normalize() {
            const T len = length();
            if (len > 0) {
                const T invLen = 1 / len;
                *this *= invLen;
            }

            return *this;
        }

        constexpr operator Vector2<f32>() const {
            return Vector2<f32>(f32(x), f32(y));
        }
        constexpr operator Vector2<f64>() const {
            return Vector2<f64>(f64(x), f64(y));
        }
        constexpr operator Vector2<int>() const {
            return Vector2<int>(int(x), int(y));
        }
    };

    using Vector2f = Vector2<f32>;
    using Vector2f64 = Vector2<f64>;
    using Vector2i = Vector2<int>;

    template <typename T>
    struct Vector3 {
        T x = 0, y = 0, z = 0;

        constexpr Vector3() = default;
        constexpr Vector3(T x, T y, T z)
            : x(x)
            , y(y)
            , z(z) { }

        constexpr bool operator==(const Vector3& rhs) const {
            return x == rhs.x && y == rhs.y && z == rhs.z;
        }

        constexpr Vector3 operator+(const Vector3& rhs) const {
            return { x + rhs.x, y + rhs.y, z + rhs.z };
        }
        constexpr Vector3 operator-(const Vector3& rhs) const {
            return { x - rhs.x, y - rhs.y, z - rhs.z };
        }
        constexpr Vector3 operator*(const Vector3& rhs) const {
            return { x * rhs.x, y * rhs.y, z * rhs.z };
        }
        constexpr Vector3 operator/(const Vector3& rhs) const {
            return { x / rhs.x, y / rhs.y, z / rhs.z };
        }

        constexpr Vector3& operator+=(const Vector3& rhs) {
            x += rhs.x;
            y += rhs.y;
            z += rhs.z;
            return *this;
        }
        constexpr Vector3& operator-=(const Vector3& rhs) {
            x -= rhs.x;
            y -= rhs.y;
            z -= rhs.z;
            return *this;
        }
        constexpr Vector3& operator*=(const Vector3& rhs) {
            x *= rhs.x;
            y *= rhs.y;
            z *= rhs.z;
            return *this;
        }
        constexpr Vector3& operator/=(const Vector3& rhs) {
            x /= rhs.x;
            y /= rhs.y;
            z /= rhs.z;
            return *this;
        }

        constexpr Vector3 operator+(T v) {
            return { x + v, y + v, z + v };
        }
        constexpr Vector3 operator-(T v) {
            return { x - v, y - v, z - v };
        }
        constexpr Vector3 operator*(T v) {
            return { x * v, y * v, z * v };
        }
        constexpr Vector3 operator/(T v) {
            return { x / v, y / v, z / v };
        }

        constexpr Vector3& operator+=(T v) {
            x += v;
            y += v;
            z += v;
            return *this;
        }
        constexpr Vector3& operator-=(T v) {
            x -= v;
            y -= v;
            z -= v;
            return *this;
        }
        constexpr Vector3& operator*=(T v) {
            x *= v;
            y *= v;
            z *= v;
            return *this;
        }
        constexpr Vector3& operator/=(T v) {
            x /= v;
            y /= v;
            z /= v;
        }

        constexpr T length() {
            return std::sqrt(x * x + y * y + z * z);
        }

        constexpr Vector3& normalize() {
            const T len = length();
            if (len > 0) {
                const T invLen = 1 / len;
                *this *= invLen;
            }

            return *this;
        }

        constexpr operator Vector3<f32>() const {
            return Vector3<f32>(f32(x), f32(y), f32(z));
        }
        constexpr operator Vector3<f64>() const {
            return Vector3<f64>(f64(x), f64(y), f64(z));
        }
        constexpr operator Vector3<int>() const {
            return Vector3<int>(int(x), int(y), int(z));
        }
    };

    using Vector3f = Vector3<f32>;
    using Vector3f64 = Vector3<f64>;
    using Vector3i = Vector3<int>;

    template <typename T>
    struct Vector4 {
        T x = 0, y = 0, z = 0, w = 0;

        constexpr Vector4() = default;
        constexpr Vector4(T x, T y, T z, T w)
            : x(x)
            , y(y)
            , z(z)
            , w(w) { }

        constexpr bool operator==(const Vector4& rhs) const {
            return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
        }

        constexpr Vector4 operator+(const Vector4& rhs) const {
            return { x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w };
        }
        constexpr Vector4 operator-(const Vector4& rhs) const {
            return { x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w };
        }
        constexpr Vector4 operator*(const Vector4& rhs) const {
            return { x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w };
        }
        constexpr Vector4 operator/(const Vector4& rhs) const {
            return { x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w };
        }

        constexpr Vector4& operator+=(const Vector4& rhs) {
            x += rhs.x;
            y += rhs.y;
            z += rhs.z;
            w += rhs.w;
            return *this;
        }
        constexpr Vector4& operator-=(const Vector4& rhs) {
            x -= rhs.x;
            y -= rhs.y;
            z -= rhs.z;
            w -= rhs.w;
            return *this;
        }
        constexpr Vector4& operator*=(const Vector4& rhs) {
            x *= rhs.x;
            y *= rhs.y;
            z *= rhs.z;
            w *= rhs.w;
            return *this;
        }
        constexpr Vector4& operator/=(const Vector4& rhs) {
            x /= rhs.x;
            y /= rhs.y;
            z /= rhs.z;
            w /= rhs.w;
            return *this;
        }

        constexpr Vector4 operator+(T v) {
            return { x + v, y + v, z + v, w + v };
        }
        constexpr Vector4 operator-(T v) {
            return { x - v, y - v, z - v, w - v };
        }
        constexpr Vector4 operator*(T v) {
            return { x * v, y * v, z * v, w * v };
        }
        constexpr Vector4 operator/(T v) {
            return { x / v, y / v, z / v, w / v };
        }

        constexpr Vector4& operator+=(T v) {
            x += v;
            y += v;
            z += v;
            w += v;
            return *this;
        }
        constexpr Vector4& operator-=(T v) {
            x -= v;
            y -= v;
            z -= v;
            w -= v;
            return *this;
        }
        constexpr Vector4& operator*=(T v) {
            x *= v;
            y *= v;
            z *= v;
            w *= v;
            return *this;
        }
        constexpr Vector4& operator/=(T v) {
            x /= v;
            y /= v;
            z /= v;
            w /= v;
        }

        constexpr T length() {
            return std::sqrt(x * x + y * y + z * z + w * w);
        }

        constexpr Vector4& normalize() {
            const T len = length();
            if (len > 0) {
                const T invLen = 1 / len;
                *this *= invLen;
            }

            return *this;
        }

        constexpr operator Vector4<f32>() const {
            return Vector4<f32>(f32(x), f32(y), f32(z), f32(w));
        }
        constexpr operator Vector4<f64>() const {
            return Vector4<f64>(f64(x), f64(y), f64(z), f64(w));
        }
        constexpr operator Vector4<int>() const {
            return Vector4<int>(int(x), int(y), int(z), int(w));
        }
    };

    using Vector4f = Vector4<f32>;
    using Vector4f64 = Vector4<f64>;
    using Vector4i = Vector4<int>;

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

    template <UnsignedIntegerType T>
    constexpr typename IntegerTraits<T>::SignedTraits::Type signExtend(T value, int width) {
        using Signed = typename IntegerTraits<T>::SignedTraits::Type;
        Signed mask = Signed(1) << (width - 1);
        return (value ^ mask) - mask;
    }

    template <UnsignedIntegerType T, size NumValues = 1>
    struct WildcardBits {
        T mask;
        T bits[NumValues] { 0 };

        using Traits = IntegerTraits<T>;

        constexpr bool test(T value) const {
            T testValue = value & mask;
            for (int i = 0; i < NumValues; i++)
                if (testValue == bits[i])
                    return true;
            return false;
        }

        template <size N>
        constexpr WildcardBits(T mask, T (&bits)[N])
            : mask(mask) {
            copy(this->bits, bits, N);
        }

        template <size N>
            requires(NumValues == 1)
        consteval WildcardBits(const char (&expr)[N])
            : mask(hk::bits(N - 1)) {
            if (N - 1 > Traits::cWidth)
                throwErr("too big");

            T& bits = this->bits[0];

            for (int i = 0; i < N - 1; i++) {
                const size idx = N - i - 2;
                const char curBit = expr[idx];

                if (curBit == '?')
                    mask &= ~bit(i);
                else if (curBit == '1')
                    bits |= bit(i);
                else if (curBit == '0')
                    ;
                else
                    throwErr("Invalid bit: ", curBit);
            }
        }

        template <size RhsNumValues>
        constexpr WildcardBits<T, NumValues + RhsNumValues> operator or(const WildcardBits<T, RhsNumValues>& rhs) const {
            T values[NumValues + RhsNumValues];
            copy(values, this->bits, NumValues);
            copy(values + NumValues, rhs.bits, RhsNumValues);
            return { mask & rhs.mask, values };
        }

    private:
        template <typename... Args>
        constexpr void throwErr(Args...) {
            char v[0];
            (void)v[1];
        }
    };

} // namespace hk::util
