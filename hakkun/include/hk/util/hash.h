#pragma once

#include "hk/diag/diag.h"
#include "hk/types.h"
#include <type_traits>

#ifdef HK_ADDON_Sead
#include <sead/prim/seadSafeString.h>
#endif

namespace hk::util {

    /* MurmurHash3 */

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

            const u32 block = a << 0
                | b << 8
                | c << 16
                | d << 24;
            return block;
        }

        template <typename T, class Read>
        constexpr u64 getBlock64(const T* p, fu64 i, void* userData) {
            static_assert(sizeof(T) == 1);
            constexpr auto read = Read::read;

            const u64 a = read(p, 0 + i * 8, userData);
            const u64 b = read(p, 1 + i * 8, userData);
            const u64 c = read(p, 2 + i * 8, userData);
            const u64 d = read(p, 3 + i * 8, userData);
            const u64 e = read(p, 4 + i * 8, userData);
            const u64 f = read(p, 5 + i * 8, userData);
            const u64 g = read(p, 6 + i * 8, userData);
            const u64 h = read(p, 7 + i * 8, userData);

            const u64 block = a << 0
                | b << 8
                | c << 16
                | d << 24
                | e << 32
                | f << 40
                | g << 48
                | h << 56;
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

        /**
         * @brief HashMurmur3 32-bit implementation. Allows partial and full feeding.
         *
         * @tparam T
         * @tparam Read
         */
        template <typename T, class Read>
        class HashMurmur32Impl {
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
            constexpr HashMurmur32Impl(const T* data, const fu32 len, const u32 seed = 0, void* userData = nullptr)
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

            template <typename V>
            void feed(const V& value) {
                feed(cast<const T*>(&value), sizeof(V));
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

            constexpr void calcWithCallback() {
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
        using HashMurmurImpl = HashMurmur32Impl<T, Read>;
        using HashMurmur = HashMurmur32Impl<char, ReadDefault<char>>;

        /**
         * @brief HashMurmur3 64-bit implementation. Allows partial and full feeding.
         *
         * @tparam T
         * @tparam Read
         */
        template <typename T, class Read>
        class HashMurmur64Impl {
            static_assert(sizeof(T) == 1);

            const T* const mData;
            const fu64 mLen;
            void* const mUserData;
            u64 mHashValue = 0;

            constexpr static fu64 nblocks(fu64 len) { return len / 8; }
            constexpr fu64 nblocks() const { return nblocks(mLen); }

            constexpr static u64 cMurmur64Constant = 0xc6a4a7935bd1e995;
            constexpr static u64 cMurmur64Seed = 0xa9f63e6017234875;
            constexpr static u64 cShift = 47;

            constexpr static u64 finalMix64(u64 h) {
                h = (h ^ (h >> cShift)) * cMurmur64Constant;
                return (h ^ (h >> cShift));
            }

        public:
            constexpr HashMurmur64Impl(const T* data, const fu64 len, void* userData = nullptr)
                : mData(data)
                , mLen(len)
                , mUserData(userData) {
                mHashValue = (len * cMurmur64Constant) ^ cMurmur64Seed;
            }

            constexpr __attribute__((noinline)) void feed(const T* fedData, const fu64 fedLen) {
                for (fu64 i = 0; i < nblocks(fedLen); i++) {
                    u64 value = getBlock64<T, ReadDefault<T>>(fedData, i, nullptr);
                    mHashValue = (((((value * cMurmur64Constant) ^ ((value * cMurmur64Constant) >> cShift)) * cMurmur64Constant)) ^ mHashValue) * cMurmur64Constant;
                }
            }

            template <typename V>
            void feed(const V& value) {
                feed(cast<const T*>(&value), sizeof(V));
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

            constexpr void calcWithCallback() {
                for (fu64 i = 0; i < nblocks(); i++) {
                    u64 value = getBlock64<T, Read>(mData, i, mUserData);
                    mHashValue = (((((value * cMurmur64Constant) ^ ((value * cMurmur64Constant) >> cShift)) * cMurmur64Constant)) ^ mHashValue) * cMurmur64Constant;
                }
            }

            constexpr u64 finalize() const {
                constexpr auto read = Read::read;

                u64 h1 = mHashValue;

                const u64 tail = mLen - (mLen % 8);

                switch (mLen & 7) {
                case 7:
                    h1 ^= u64(read(mData, tail + 6, mUserData)) << 48;
                case 6:
                    h1 ^= u64(read(mData, tail + 5, mUserData)) << 40;
                case 5:
                    h1 ^= u64(read(mData, tail + 4, mUserData)) << 32;
                case 4:
                    h1 ^= u64(read(mData, tail + 3, mUserData)) << 24;
                case 3:
                    h1 ^= u64(read(mData, tail + 2, mUserData)) << 16;
                case 2:
                    h1 ^= u64(read(mData, tail + 1, mUserData)) << 8;
                case 1:
                    h1 ^= read(mData, tail + 0, mUserData);
                    h1 *= cMurmur64Constant;
                }

                return finalMix64(h1);
            }
        };

        using HashMurmur64 = HashMurmur64Impl<char, ReadDefault<char>>;

        template <typename T, class Read>
        constexpr u32 hashMurmurImpl(const T* data, const fu32 len, const u32 seed = 0, void* userData = nullptr) {
            HashMurmur32Impl<T, Read> hash(data, len, seed, userData);
            hash.calcWithCallback();
            return hash.finalize();
        }

        template <typename T, class Read>
        constexpr u64 hashMurmur64Impl(const T* data, const fu64 len, void* userData = nullptr) {
            HashMurmur64Impl<T, Read> hash(data, len, userData);
            hash.calcWithCallback();
            return hash.finalize();
        }

    } // namespace detail

    constexpr u32 hashMurmur(const char* str, u32 seed = 0) {
        return detail::hashMurmurImpl<char, detail::ReadDefault<char>>(str, __builtin_strlen(str), seed);
    }

    constexpr u64 hashMurmur64(const char* str) {
        return detail::hashMurmur64Impl<char, detail::ReadDefault<char>>(str, __builtin_strlen(str));
    }

    template <typename T>
    constexpr u32 hashMurmur(const T* data, const fu32 len, const u32 seed = 0) {
        static_assert(sizeof(T) == 1);
        return detail::hashMurmurImpl<T, detail::ReadDefault<T>>(data, len, seed);
    }

    template <typename T>
    constexpr u64 hashMurmur64(const T* data, const fu64 len) {
        static_assert(sizeof(T) == 1);
        return detail::hashMurmur64Impl<T, detail::ReadDefault<T>>(data, len);
    }

    // Not sure how to make constexpr type-punning work
    template <typename T>
    hk_alwaysinline u32 hashMurmurT(const T& value, const u32 seed = 0) {
        struct {
            u8 data[sizeof(T)];
        } data = pun<typeof(data)>(value);

        return hashMurmur(data.data, sizeof(T), seed);
    }

    template <typename T>
    hk_alwaysinline u32 hashMurmur64T(const T& value) {
        struct {
            u8 data[sizeof(T)];
        } data = pun<typeof(data)>(value);

        return hashMurmur64(data.data, sizeof(T));
    }

    template <typename T>
    struct MurmurHash3 {
        static size hash(const T& value) {
            if constexpr (is64Bit())
                return hashMurmur64T(value);
            else
                return hashMurmurT(value);
        }
    };

    template <>
    struct MurmurHash3<const char*> {
        static size hash(const char* str) {
            if constexpr (is64Bit())
                return hashMurmur64(str);
            else
                return hashMurmur(str);
        }
    };

#ifdef HK_ADDON_Sead
    template <Derived<sead::SafeString> T>
    struct MurmurHash3<T> {
        static size hash(const T& str) {
            return MurmurHash3<const char*>::hash(str.cstr());
        }
    };
#endif

    template <size N>
    hk_alwaysinline bool isEqualStringHash(const char* str, const char (&literal)[N], u32 seed = 0) {
        constexpr u32 literalHash = hashMurmur(literal, seed);
        return hashMurmur(str, seed) == literalHash;
    }

    /* ELF */

    constexpr u64 hashElfBucket(const char* name) {
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

    /* GNU ELF */

    constexpr u32 hashDjb2(const char* name) {
        u32 h = 5381;
        for (const char* p = name; *p; p++)
            h = h * 33 + static_cast<u8>(*p);
        return h;
    }

    /* IEEE 802.3 CRC32 */

    template <typename T>
    constexpr u32 hashCrc32(const T* data, const fu64 len, u32 seed = 0) {
        static_assert(sizeof(T) == 1);

        constexpr u32 cLut[256] {
#include "hash_crc32data.h"
        };

        u32 h = seed ^ 0xFFFFFFFF;
        for (fu64 i = 0; i < len; i++)
            h = cLut[*data++ ^ (h & 0xFF)] ^ (h >> 8);

        return h ^ 0xFFFFFFFF;
    }

    constexpr u32 hashCrc32(const char* str, u32 seed = 0) {
        return hashCrc32<char>(str, __builtin_strlen(str), seed);
    }

    static_assert(hashMurmur("meow meow meow") == 0x1a1888b6);
    static_assert(hashMurmur("Haiiiiiiiiiiii") == 0x6726fccb);
    static_assert(hashMurmur(":333333333", 0xB00B1E5) == 0x4f39bed5);
    static_assert(hashMurmur("lkdjtgljkwerlkgver#g#ää5r+#ä#23ü4#2ü3420395904e3r8i9", 0xB00B1E6) == 0xcaafb947);
    static_assert(hashCrc32("meow") == 0x8a106afe);
    static_assert(hashMurmur64("engine__actor__ActorParam") == 0x3c520ab863c2a552);

} // namespace hk::util
