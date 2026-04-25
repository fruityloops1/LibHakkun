#pragma once

#include "hk/container/Vec.h"
#include "hk/util/Algorithm.h"
#include "hk/util/Allocator.h"
#include "hk/util/Tuple.h"
#include "hk/util/hash.h"

namespace hk {

    /**
     * @brief (unordered) set.
     *
     * @tparam K Key
     * @tparam V Value
     * @tparam HashFunc Class containing hash function for key; HashMurmur by default, must be supplied with a custom function for
     * @tparam ReserveSize Amount of elements to reserve by default or when pushing elements past the current capacity
     */
    template <typename T, typename HashFunc = util::MurmurHash3<T>, size ReserveSize = 16, util::AllocatorType Allocator = util::DefaultAllocator>
    class Set : Vec<Tuple<size, T>, ReserveSize, Allocator> {
        using Pair = Tuple<size, T>;
        using VecType = Vec<Pair, ReserveSize, Allocator>;
        using Hash = size;

        constexpr size binarySearch(const T& key, bool findBetween = false) const {
            return VecType::binarySearch([](const Pair& pair) -> Hash { return pair.a; }, HashFunc::hash(key), findBetween);
        }

        constexpr void checkInsert(const T& key) {
            HK_ABORT_UNLESS(binarySearch(key) == -1, "hk::Set<%s>::insert: value already exists", util::getTypeName<T>());
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

        constexpr T* find(const T& key) {
            s32 foundIdx = binarySearch(key);
            if (foundIdx == -1)
                return nullptr;

            return &VecType::at(foundIdx).b;
        }

        constexpr const T* find(const T& key) const {
            s32 foundIdx = binarySearch(key);
            if (foundIdx == -1)
                return nullptr;

            return &VecType::at(foundIdx).b;
        }

        constexpr T& insert(const T& value) {
            checkInsert(value);
            s32 insertIdx = binarySearch(value, true);
            return VecType::insert({ HashFunc::hash(value), value }, insertIdx).b;
        }

        constexpr Pair& insert(T&& value) {
            checkInsert(value);
            s32 insertIdx = binarySearch(value, true);
            return VecType::insert({ HashFunc::hash(static_cast<const T&>(value)), forward<T>(value) }, insertIdx).b;
        }

        constexpr T& at(const T& key) {
            T* value = find(key);
            HK_ABORT_UNLESS(value != nullptr, "hk::Set<%s>::at(): key not found", util::getTypeName<T>());
            return *value;
        }

        constexpr const T& at(const T& key) const {
            const T* value = find(key);
            HK_ABORT_UNLESS(value != nullptr, "hk::Set<%s>::at(): key not found", util::getTypeName<T>());
            return *value;
        }

        constexpr ValueOrResult<T> remove(const T& key) {
            ::size index = binarySearch(key);
            HK_UNLESS(index != -1, ResultNoValue());

            return move(VecType::remove(index).b);
        }

        constexpr T& operator[](const T& key) {
            T* value = find(key);
            return value ? *value : insert(key, T()).b;
        }

        template <typename Callback>
        constexpr void forEach(Callback func) {
            VecType::forEach([func](Pair& pair) {
                func(pair.b);
            });
        }

        template <typename Callback>
        constexpr void forEach(Callback func) const {
            VecType::forEach([func](const Pair& pair) {
                func(pair.b);
            });
        }
    };

} // namespace hk
