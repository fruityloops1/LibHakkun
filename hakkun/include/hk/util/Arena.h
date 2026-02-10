#pragma once

#include "hk/util/Allocator.h"
#include "hk/util/Tuple.h"
#include "hk/util/TypeName.h"
#include "hk/util/Vec.h"

namespace hk::util {

    /**
     * @brief Generational arena.
     *
     * @tparam T
     * @tparam Allocator
     */
    template <typename T, AllocatorType Allocator = MallocAllocator>
    class Arena {
        using Key = u32;
        using Entry = Tuple<Key, T>;

        Vec<Entry, 16, Allocator> mEntries;
        Key mCurrentKey = 0;

    public:
        Tuple<Key, T&> add(T&& value) {
            Key key = mCurrentKey++;
            mEntries.add({ key, std::move(value) });
            return { key, mEntries.last().b };
        }

        Tuple<Key, T&> add(const T& value) {
            Key key = mCurrentKey++;
            mEntries.add({ key, value });
            return { key, mEntries.last().b };
        }

        T& get(Key key) {
            for (Entry& entry : mEntries)
                if (entry.a == key)
                    return entry.b;
            HK_ABORT("hk::util::Arena<%s>::get(): key (%u) not found", getTypeName<T>(), key);
        }

        const T& get(Key key) const {
            for (const Entry& entry : mEntries)
                if (entry.a == key)
                    return entry.b;
            HK_ABORT("hk::util::Arena<%s>::get(): key (%u) not found", getTypeName<T>(), key);
        }

        bool has(Key key) const {
            for (const Entry& entry : mEntries)
                if (entry.a == key)
                    return true;
            return false;
        }

        void drop(Key key) {
            for (size i = 0; i < mEntries.size(); i++)
                if (mEntries[i].a == key) {
                    mEntries.remove(i);
                    break;
                }
        }

        T& operator[](Key key) { return get(key); }
        const T& operator[](Key key) const { return get(key); }
    };

} // namespace hk::util
