#pragma once

#include "hk/diag/diag.h"
#include "hk/types.h"

namespace hk::util {

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
        class HashMurmurImpl {
            static_assert(sizeof(T) == 1);

            const T* const mData;
            const fu32 mLen;
            const u32 mSeed;
            void* const mUserData;
            u32 mHashValue = 0;

            constexpr static fu32 nblocks(fu32 len) { return len / 4; }
            constexpr fu32 nblocks() const { return nblocks(mLen); }

            constexpr static u32 c1 = 0xcc9e2d51;
            constexpr static u32 c2 = 0x1b873593;

            constexpr static u32 c3 = 0xe6546b64;

        public:
            constexpr HashMurmurImpl(const T* data, const fu32 len, const u32 seed = 0, void* userData = nullptr)
                : mData(data)
                , mLen(len)
                , mSeed(seed)
                , mUserData(userData) {
                mHashValue = mSeed;
            }

            constexpr __attribute__((noinline)) void feed(const T* fedData, const fu32 fedLen) {
                for (fu32 i = 0; i < nblocks(fedLen); i++) {
                    u32 k1 = getBlock<T, ReadDefault<T>>(fedData, i, nullptr);

                    k1 *= c1;
                    k1 = rotateLeft32(k1, 15);
                    k1 *= c2;

                    mHashValue ^= k1;
                    mHashValue = rotateLeft32(mHashValue, 13);
                    mHashValue = mHashValue * 5 + c3;
                }
            }

            constexpr void feedNullTerminated(const T* str) {
                size len = 0;
                {
                    const T* _str = str;
                    while (*_str) {
                        _str++;
                        len++;
                    }
                }

                feed(str, len);

                const fu32 tail = len & 3;
                if (tail > 0) {
                    T tailData[sizeof(u32)] = {};
                    for (size i = 0; i < tail; i++)
                        tailData[i] = str[len - tail + i];
                    feed(tailData, sizeof(tailData));
                }
            }

            constexpr void calculateWithCallback() {
                for (fu32 i = 0; i < nblocks(); i++) {
                    u32 k1 = getBlock<T, Read>(mData, i, mUserData);

                    k1 *= c1;
                    k1 = rotateLeft32(k1, 15);
                    k1 *= c2;

                    mHashValue ^= k1;
                    mHashValue = rotateLeft32(mHashValue, 13);
                    mHashValue = mHashValue * 5 + c3;
                }
            }

            constexpr u32 finalize() const {
                constexpr auto read = Read::read;

                u32 h1 = mHashValue;
                u32 k1 = 0;

                const u32 tail = mLen - (mLen % 4);

                switch (mLen & 3) {
                case 3:
                    k1 ^= read(mData, tail + 2, mUserData) << 16;
                case 2:
                    k1 ^= read(mData, tail + 1, mUserData) << 8;
                case 1:
                    k1 ^= read(mData, tail + 0, mUserData);
                    k1 *= c1;
                    k1 = rotateLeft32(k1, 15);
                    k1 *= c2;
                    h1 ^= k1;
                }

                h1 ^= mLen;

                return finalMix(h1);
            }
        };

        template <typename T, class Read>
        constexpr u32 hashMurmurImpl(const T* data, const fu32 len, const u32 seed = 0, void* userData = nullptr) {
            HashMurmurImpl<T, Read> hash(data, len, seed, userData);
            hash.calculateWithCallback();
            return hash.finalize();
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

    // Not sure how to make constexpr type-punning work
    template <typename T>
    hk_alwaysinline u32 hashMurmurT(const T& value, const u32 seed = 0) {
        struct {
            u8 data[sizeof(T)];
        } data = pun<typeof(data)>(value);

        return hashMurmur(data.data, sizeof(T), seed);
    }

    template <size N>
    hk_alwaysinline bool isEqualStringHash(const char* str, const char (&literal)[N], u32 seed = 0) {
        constexpr u32 literalHash = hashMurmur(literal, seed);
        return hashMurmur(str, seed) == literalHash;
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

    static_assert(hashMurmur("meow meow meow") == 0x1a1888b6);
    static_assert(hashMurmur("Haiiiiiiiiiiii") == 0x6726fccb);
    static_assert(hashMurmur(":333333333", 0xB00B1E5) == 0x4f39bed5);
    static_assert(hashMurmur("lkdjtgljkwerlkgver#g#ää5r+#ä#23ü4#2ü3420395904e3r8i9", 0xB00B1E6) == 0xcaafb947);

} // namespace hk::util
