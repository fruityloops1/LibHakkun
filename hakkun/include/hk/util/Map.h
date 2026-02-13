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
        using VecType = Vec<Pair, ReserveSize, Allocator>;
        using Hash = size;

        size binarySearch(const K& key, bool findBetween = false) const {
            return VecType::binarySearch([](const Pair& pair) -> Hash { return HashFunc::hash(pair.a); }, HashFunc::hash(key), findBetween);
        }

        void checkInsert(const K& key) {
            HK_ABORT_UNLESS(binarySearch(key) == -1, "hk::util::Map<%s, %s>::insert: key already exists", getTypeName<K>(), getTypeName<V>());
        }

    public:
        using VecType::capacity;
        using VecType::clear;
        using VecType::empty;
        using VecType::reserve;
        using VecType::size;
        using VecType::sort;

        using VecType::begin;
        using VecType::end;

        V* find(const K& key) {
            s32 foundIdx = binarySearch(key);
            if (foundIdx == -1)
                return nullptr;

            return &(*static_cast<VecType*>(this))[foundIdx].b;
        }

        const V* find(const K& key) const {
            s32 foundIdx = binarySearch(key);
            if (foundIdx == -1)
                return nullptr;

            return &(*static_cast<const VecType*>(this))[foundIdx].b;
        }

        Pair& insert(const K& key, const V& value) {
            checkInsert(key);
            s32 insertIdx = binarySearch(key, true);
            return VecType::insert({ key, value }, insertIdx);
        }

        Pair& insert(K&& key, V&& value) {
            checkInsert(key);
            s32 insertIdx = binarySearch(key, true);
            return VecType::insert({ move(key), move(value) }, insertIdx);
        }

        Pair& insert(const K& key, V&& value) {
            checkInsert(key);
            s32 insertIdx = binarySearch(key, true);
            return VecType::insert({ key, move(value) }, insertIdx);
        }

        Pair& insert(K&& key, const V& value) {
            checkInsert(key);
            s32 insertIdx = binarySearch(key, true);
            return VecType::insert({ move(key), value }, insertIdx);
        }

        V& at(const K& key) {
            V* value = find(key);
            HK_ABORT_UNLESS(value != nullptr, "hk::util::Map<%s, %s>::at(): key not found", getTypeName<K>(), getTypeName<V>());
            return *value;
        }

        const V& at(const K& key) const {
            V* value = find(key);
            HK_ABORT_UNLESS(value != nullptr, "hk::util::Map<%s, %s>::at(): key not found", getTypeName<K>(), getTypeName<V>());
            return *value;
        }

        ValueOrResult<V> remove(const K& key) {
            ::size index = binarySearch(key);
            HK_UNLESS(index != -1, ResultNoValue());

            return move(VecType::remove(index));
        }

        V& operator[](const K& key) {
            V* value = find(key);
            return value ? *value : insert(key, {}).b;
        }

        template <typename Callback>
        void forEach(Callback func) {
            VecType::forEach([func](Pair& pair) {
                func(pair.a, pair.b);
            });
        }

        template <typename Callback>
        void forEach(Callback func) const {
            VecType::forEach([func](const Pair& pair) {
                func(pair.a, pair.b);
            });
        }
    };

} // namespace hk::util
