#pragma once

#include "hk/container/Vec.h"
#include "hk/util/Algorithm.h"
#include "hk/util/Allocator.h"
#include "hk/util/Tuple.h"
#include "hk/util/hash.h"

namespace hk {

    /**
     * @brief (unordered) map of keys to values.
     *
     * @tparam K Key
     * @tparam V Value
     * @tparam HashFunc Class containing hash function for key; HashMurmur by default, must be supplied with a custom function for
     * @tparam ReserveSize Amount of elements to reserve by default or when pushing elements past the current capacity
     */
    template <typename K, typename V, typename HashFunc = util::MurmurHash3<K>, size ReserveSize = 16, util::AllocatorType Allocator = util::DefaultAllocator>
    class Map : Vec<Tuple<K, V>, ReserveSize, Allocator> {
        using Pair = Tuple<K, V>;
        using VecType = Vec<Pair, ReserveSize, Allocator>;
        using Hash = size;

        constexpr size binarySearch(const K& key, bool findBetween = false) const {
            return VecType::binarySearch([](const Pair& pair) -> Hash { return HashFunc::hash(pair.a); }, HashFunc::hash(key), findBetween);
        }

        constexpr void checkInsert(const K& key) {
            HK_ABORT_UNLESS(binarySearch(key) == -1, "hk::Map<%s, %s>::insert: key already exists", util::getTypeName<K>(), util::getTypeName<V>());
        }

        using VecType::at;

    public:
        using VecType::capacity;
        using VecType::clear;
        using VecType::empty;
        using VecType::reserve;
        using VecType::size;
        using VecType::sort;

        using VecType::begin;
        using VecType::end;

        constexpr V* find(const K& key) {
            s32 foundIdx = binarySearch(key);
            if (foundIdx == -1)
                return nullptr;

            return &VecType::at(foundIdx).b;
        }

        constexpr const V* find(const K& key) const {
            s32 foundIdx = binarySearch(key);
            if (foundIdx == -1)
                return nullptr;

            return &VecType::at(foundIdx).b;
        }

        constexpr Pair& insert(const K& key, const V& value) {
            checkInsert(key);
            s32 insertIdx = binarySearch(key, true);
            return VecType::insert({ key, value }, insertIdx);
        }

        constexpr Pair& insert(K&& key, V&& value) {
            checkInsert(key);
            s32 insertIdx = binarySearch(key, true);
            return VecType::insert({ forward<K>(key), forward<V>(value) }, insertIdx);
        }

        constexpr Pair& insert(const K& key, V&& value) {
            checkInsert(key);
            s32 insertIdx = binarySearch(key, true);
            return VecType::insert({ key, forward<V>(value) }, insertIdx);
        }

        constexpr Pair& insert(K&& key, const V& value) {
            checkInsert(key);
            s32 insertIdx = binarySearch(key, true);
            return VecType::insert({ forward<K>(key), value }, insertIdx);
        }

        constexpr V& at(const K& key) {
            V* value = find(key);
            HK_ABORT_UNLESS(value != nullptr, "hk::Map<%s, %s>::at(): key not found", util::getTypeName<K>(), util::getTypeName<V>());
            return *value;
        }

        constexpr const V& at(const K& key) const {
            const V* value = find(key);
            HK_ABORT_UNLESS(value != nullptr, "hk::Map<%s, %s>::at(): key not found", util::getTypeName<K>(), util::getTypeName<V>());
            return *value;
        }

        constexpr ValueOrResult<Pair> remove(const K& key) {
            ::size index = binarySearch(key);
            HK_UNLESS(index != -1, ResultNoValue());

            return move(VecType::remove(index));
        }

        constexpr V& operator[](const K& key) {
            V* value = find(key);
            return value ? *value : insert(key, V()).b;
        }

        template <typename Callback>
        constexpr void forEach(Callback func) {
            VecType::forEach([func](Pair& pair) {
                func(pair.a, pair.b);
            });
        }

        template <typename Callback>
        constexpr void forEach(Callback func) const {
            VecType::forEach([func](const Pair& pair) {
                func(pair.a, pair.b);
            });
        }
    };

} // namespace hk
