#pragma once

#include "hk/prim/traits/Type.h"
#include <cstdint>

constexpr std::size_t operator""_B(unsigned long long val) { return val; }
constexpr std::size_t operator""_KB(unsigned long long val) { return val * 1024; }
constexpr std::size_t operator""_MB(unsigned long long val) { return val * 1024 * 1024; }
constexpr std::size_t operator""_GB(unsigned long long val) { return val * 1024 * 1024 * 1024; }

constexpr std::size_t operator""_ns(unsigned long long val) { return val; }
constexpr std::size_t operator""_µs(unsigned long long val) { return val * 1e3; }
constexpr std::size_t operator""_ms(unsigned long long val) { return val * 1e6; }
constexpr std::size_t operator""_sec(unsigned long long val) { return val * 1e9; }

namespace hk {

    template <typename T>
    concept IntegerType = util::ctIsIntegral<T>;

    constexpr int cSystemRegisterWidth = sizeof(void*) * 8;
    constexpr int cMaxRepresentableWidth = cSystemRegisterWidth;

    constexpr std::size_t bit(int n) { return std::size_t(1) << n; }

    constexpr std::size_t bits(int n) {
        if (n == cSystemRegisterWidth)
            return std::size_t(-1);
        return bit(n) - 1;
    }

    namespace util {

        template <int Width>
        struct SignedIntegerTraitsByWidth;
        template <int Width>
        struct UnsignedIntegerTraitsByWidth;

        template <IntegerType T>
        struct IntegerTraits : private TypeTraits<T> {
            using Type = T;

            static constexpr int cSize = sizeof(T);
            static constexpr int cWidth = util::ctIsSame<T, bool> ? 1 : cSize * 8;
            static constexpr bool cIsSigned = T(-1) < T(0);
            static constexpr bool cIsUnsigned = !cIsSigned;
            static constexpr std::size_t cBits = bits(cWidth);

            static constexpr T cMin = cIsUnsigned ? T(0) : -T((cBits - 1) / 2) - T(1);
            static constexpr T cMax = cIsUnsigned ? T(cBits) : T(cBits / 2);

            using UnsignedTraits = UnsignedIntegerTraitsByWidth<cWidth>;
            using SignedTraits = SignedIntegerTraitsByWidth<cWidth>;
        };

        template <>
        struct SignedIntegerTraitsByWidth<IntegerTraits<int8_t>::cWidth> : IntegerTraits<int8_t> { };
        template <>
        struct SignedIntegerTraitsByWidth<IntegerTraits<int16_t>::cWidth> : IntegerTraits<int16_t> { };
        template <>
        struct SignedIntegerTraitsByWidth<IntegerTraits<int32_t>::cWidth> : IntegerTraits<int32_t> { };
        template <>
        struct SignedIntegerTraitsByWidth<IntegerTraits<int64_t>::cWidth> : IntegerTraits<int64_t> { };

        template <>
        struct UnsignedIntegerTraitsByWidth<IntegerTraits<bool>::cWidth> : IntegerTraits<bool> { };
        template <>
        struct UnsignedIntegerTraitsByWidth<IntegerTraits<uint8_t>::cWidth> : IntegerTraits<uint8_t> { };
        template <>
        struct UnsignedIntegerTraitsByWidth<IntegerTraits<uint16_t>::cWidth> : IntegerTraits<uint16_t> { };
        template <>
        struct UnsignedIntegerTraitsByWidth<IntegerTraits<uint32_t>::cWidth> : IntegerTraits<uint32_t> { };
        template <>
        struct UnsignedIntegerTraitsByWidth<IntegerTraits<uint64_t>::cWidth> : IntegerTraits<uint64_t> { };

    } // namespace util

    template <int Width>
    using SignedInteger = util::SignedIntegerTraitsByWidth<Width>::Type;
    template <int Width>
    using UnsignedInteger = util::UnsignedIntegerTraitsByWidth<Width>::Type;

    template <typename T>
    concept UnsignedIntegerType = util::IntegerTraits<T>::cIsUnsigned;
    template <typename T>
    concept SignedIntegerType = util::IntegerTraits<T>::cIsSigned;

    using SizeType = UnsignedInteger<cSystemRegisterWidth>;

    static_assert(util::ctIsSame<SizeType, std::size_t>);

    static constexpr SizeType cPageSize =
#ifdef NNSDK
        4_KB
#else
        4_KB // default
#endif
        ;

    namespace util {

        constexpr bool isRepresentable(SizeType width, SizeType value, SizeType numShift = 1) {
            for (SizeType i = 0; i < numShift; i++) {
                if ((value & (bits(width) << (i * width))) == value) {
                    return true;
                }
            }
            return false;
        }

        constexpr int calcSmallestWidth(SizeType max) {
            if (max <= 1)
                return 1;
            int curWidth = 8;
            while (curWidth != cMaxRepresentableWidth) {
                if (isRepresentable(curWidth, max))
                    return curWidth;
                curWidth *= 2;
            }
            return -1;
        }

        template <SizeType MaxValue>
        struct SmallestUnsignedIntegerTraits : UnsignedIntegerTraitsByWidth<calcSmallestWidth(MaxValue)> { };

    } // namespace util

    template <SizeType MaxValue>
    using SmallestUnsignedInteger = util::SmallestUnsignedIntegerTraits<MaxValue>::Type;

    template <IntegerType T>
    constexpr T alignUp(T from, SizeType alignment) { return T((typename util::IntegerTraits<T>::UnsignedTraits::Type(from) + (alignment - 1)) & ~(alignment - 1)); }
    template <IntegerType T>
    constexpr T alignDown(T from, SizeType alignment) { return T(typename util::IntegerTraits<T>::UnsignedTraits::Type(from) & ~(alignment - 1)); }

    template <IntegerType T>
    constexpr T alignUpPage(T from) { return alignUp(from, cPageSize); }
    template <IntegerType T>
    constexpr T alignDownPage(T from) { return alignDown(from, cPageSize); }

    template <IntegerType T>
    constexpr bool isAligned(T from, T alignment) { return alignDown(from, alignment) == from; }
    template <IntegerType T>
    constexpr bool isAlignedPage(T from) { return alignDown(from, cPageSize) == from; }

} // namespace hk

using s8 = hk::SignedInteger<8>;
using s16 = hk::SignedInteger<16>;
using s32 = hk::SignedInteger<32>;
using s64 = hk::SignedInteger<64>;

using u8 = hk::UnsignedInteger<8>;
using u16 = hk::UnsignedInteger<16>;
using u32 = hk::UnsignedInteger<32>;
using u64 = hk::UnsignedInteger<64>;

using size = hk::UnsignedInteger<hk::cSystemRegisterWidth>;

namespace hk::util::detail {

    template <>
    struct PrintfFormatVerbose<bool> {
        static constexpr const char cValue[] = "bool: %d";
    };
    template <>
    struct PrintfFormatVerbose<char> {
        static constexpr const char cValue[] = "char: %c";
    };
    template <>
    struct PrintfFormatVerbose<s8> {
        static constexpr const char cValue[] = "s8: %hhd";
    };
    template <>
    struct PrintfFormatVerbose<u8> {
        static constexpr const char cValue[] = "u8: %hhu";
    };
    template <>
    struct PrintfFormatVerbose<s16> {
        static constexpr const char cValue[] = "s16: %hd";
    };
    template <>
    struct PrintfFormatVerbose<u16> {
        static constexpr const char cValue[] = "u16: %hu";
    };
    template <>
    struct PrintfFormatVerbose<s32> {
        static constexpr const char cValue[] = "s32: %d";
    };
    template <>
    struct PrintfFormatVerbose<u32> {
        static constexpr const char cValue[] = "u32: %u";
    };
    template <>
    struct PrintfFormatVerbose<s64> {
        static constexpr const char cValue[] = "s64: %lld";
    };
    template <>
    struct PrintfFormatVerbose<size> {
        static constexpr const char cValue[] = "size: %zu";
    };

} // namespace hk::util::detail
