#pragma once

#include "hk/util/Algorithm.h"
#include "hk/util/Tuple.h"
#include "hk/util/Vec.h"
#include "hk/util/hash.h"

namespace hk::util {

    /**
     * @brief (unordered) map of keys to values.
     *
     * @tparam K Key
     * @tparam V Value
     * @tparam HashFunc Class containing hash function for key; HashMurmur by default, must be supplied with a custom function for
     * @tparam ReserveSize Amount of elements to reserve by default or when pushing elements past the current capacity
     */
    template <typename K, typename V, typename HashFunc = MurmurHash3<K>, size ReserveSize = 16, AllocatorType Allocator = MallocAllocator>
    class Map : Vec<Tuple<K, V>, ReserveSize, Allocator> {
        using Pair = Tuple<K, V>;
        using VecType = Vec<Tuple<K, V>, ReserveSize, Allocator>;
        using Hash = size;

        template <bool FindBetweenIdx = false>
        s32 binarySearch(const K& key) const {
            return VecType::binarySearch([](const Pair& pair) -> Hash { return HashFunc::hash(pair.a); }, HashFunc::hash(key));
        }

        V* findValue(const K& key) {
            s32 foundIdx = binarySearch(key);
            if (foundIdx == -1)
                return -1;

            return &(*static_cast<VecType*>(this))[foundIdx].b;
        }

        const V* findValue(const K& key) const {
            s32 foundIdx = binarySearch(key);
            if (foundIdx == -1)
                return -1;

            return &(*static_cast<const VecType*>(this))[foundIdx].b;
        }

        void checkInsert(const K& key) {
            HK_ABORT_UNLESS(binarySearch(key) == -1, "hk::util::Map<%s, %s>::insert: key already exists", getTypeName<K>(), getTypeName<V>());
        }

    public:
        using VecType::capacity;
        using VecType::clear;
        using VecType::empty;
        using VecType::forEach;
        using VecType::reserve;
        using VecType::size;
        using VecType::sort;

        using VecType::begin;
        using VecType::end;

        Pair& insert(const K& key, const V& value) {
            checkInsert(key);
            s32 insertIdx = binarySearch<true>(key);
            return VecType::insert({ key, value }, insertIdx);
        }

        Pair& insert(K&& key, V&& value) {
            checkInsert(key);
            s32 insertIdx = binarySearch<true>(key);
            return VecType::insert({ move(key), move(value) }, insertIdx);
        }

        Pair& insert(const K& key, V&& value) {
            checkInsert(key);
            s32 insertIdx = binarySearch<true>(key);
            return VecType::insert({ key, move(value) }, insertIdx);
        }

        Pair& insert(K&& key, const V& value) {
            checkInsert(key);
            s32 insertIdx = binarySearch<true>(key);
            return VecType::insert({ move(key), value }, insertIdx);
        }

        V& at(const K& key) {
            V* value = findValue(key);
            HK_ABORT_UNLESS(value != nullptr, "hk::util::Map<%s, %s>::at(): key not found", getTypeName<K>(), getTypeName<V>());
            return *value;
        }

        const V& at(const K& key) const {
            V* value = findValue(key);
            HK_ABORT_UNLESS(value != nullptr, "hk::util::Map<%s, %s>::at(): key not found", getTypeName<K>(), getTypeName<V>());
            return *value;
        }

        V& operator[](const K& key) {
            V* value = findValue(key);
            return value ? *value : insert(key, {}).b;
        }
    };

} // namespace hk::util
