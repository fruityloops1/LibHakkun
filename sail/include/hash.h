#pragma once

#include "types.h"

namespace sail {
    namespace detail {

        template <typename T>
        struct ReadDefault {
            static constexpr T read(const T* data, ptr offset, void*) {
                return data[offset];
            }
        };

        template <typename T, class Read>
        constexpr u32 getBlock(const T* p, fu32 i, void* userData) {
            static_assert(sizeof(T) == 1);
            constexpr auto read = Read::read;

            const u32 a = read(p, 0 + i * 4, userData);
            const u32 b = read(p, 1 + i * 4, userData);
            const u32 c = read(p, 2 + i * 4, userData);
            const u32 d = read(p, 3 + i * 4, userData);

            const u32 block = a << 0 | b << 8 | c << 16 | d << 24;
            return block;
        }

        constexpr u32 rotateLeft32(u32 x, u32 r) {
            return (x << r) | (x >> (32 - r));
        }

        constexpr u32 finalMix(u32 h) {
            h ^= h >> 16;
            h *= 0x85ebca6b;
            h ^= h >> 13;
            h *= 0xc2b2ae35;
            h ^= h >> 16;
            return h;
        }

        template <typename T, class Read>
        constexpr u32 hashMurmurImpl(const T* data, const fu32 len, const u32 seed = 0, void* userData = nullptr) {
            static_assert(sizeof(T) == 1);
            constexpr auto read = Read::read;

            const fu32 nblocks = len / 4;

            u32 h1 = seed;

            constexpr u32 c1 = 0xcc9e2d51;
            constexpr u32 c2 = 0x1b873593;

            constexpr u32 c3 = 0xe6546b64;

            for (fu32 i = 0; i < nblocks; i++) {
                u32 k1 = detail::getBlock<T, Read>(data, i, userData);

                k1 *= c1;
                k1 = detail::rotateLeft32(k1, 15);
                k1 *= c2;

                h1 ^= k1;
                h1 = detail::rotateLeft32(h1, 13);
                h1 = h1 * 5 + c3;
            }

            u32 k1 = 0;

            const u32 tail = len - (len % 4);

            switch (len & 3) {
            case 3:
                k1 ^= read(data, tail + 2, userData) << 16;
            case 2:
                k1 ^= read(data, tail + 1, userData) << 8;
            case 1:
                k1 ^= read(data, tail + 0, userData);
                k1 *= c1;
                k1 = detail::rotateLeft32(k1, 15);
                k1 *= c2;
                h1 ^= k1;
            }

            h1 ^= len;

            return detail::finalMix(h1);
        }

    } // namespace detail

    constexpr u32 hashMurmur(const char* str, u32 seed = 0) {
        return detail::hashMurmurImpl<char, detail::ReadDefault<char>>(str, __builtin_strlen(str), seed);
    }

    template <typename T>
    constexpr u32 hashMurmur(const T* data, const fu32 len, const u32 seed = 0) {
        static_assert(sizeof(T) == 1);
        return detail::hashMurmurImpl<T, detail::ReadDefault<T>>(data, len, seed);
    }

    constexpr u64 rtldElfHash(const char* name) {
        u64 h = 0;
        u64 g;

        while (*name) {
            h = (h << 4) + *name++;
            if ((g = h & 0xf0000000))
                h ^= g >> 24;
            h &= ~g;
        }
        return h;
    }

    constexpr u32 djb2Hash(const char* name) {
        u32 h = 5381;
        for (const char* p = name; *p; p++)
            h = h * 33 + static_cast<uint8_t>(*p);
        return h;
    }

} // namespace sail
