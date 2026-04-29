#pragma once

#include "hk/prim/CellOperations.h"
#include "hk/util/TypeName.h"

namespace hk {

    namespace detail {

        template <typename T>
        class CellStorage : util::CustomTypeName {
        protected:
            union {
                T mValue;
            };
            bool mAlive = false;

        public:
            constexpr T* getStorage() { return &mValue; }
            constexpr const T* getStorageConst() const { return &mValue; }
            constexpr void setAlive(bool alive) { mAlive = alive; }
            constexpr bool isAlive() const { return mAlive; }
            constexpr static bool isAliveMutable() { return true; }

            static constexpr char cTypeName[] = "hk::Cell";
        };

    } // namespace detail

    template <typename T>
    class Cell : public CellOperations<T, detail::CellStorage<T>> {
        using Super = CellOperations<T, detail::CellStorage<T>>;

    public:
        using Super::Super;
    };

    namespace detail {

        template <typename T>
        class HeapCellStorage : util::CustomTypeName {
            T* mInstance = nullptr;

        public:
            constexpr void setStorage(T* instance) { mInstance = instance; }
            constexpr T* getStorage() { return mInstance; }
            constexpr const T* getStorageConst() const { return mInstance; }
            constexpr void setAlive(bool alive) { HK_ABORT(""); }
            constexpr bool isAlive() const { return mInstance != nullptr; }
            constexpr static bool isAliveMutable() { return false; }

            static constexpr char cTypeName[] = "hk::HeapCell";
        };

    } // namespace detail

    template <typename T, typename Allocator = util::DefaultAllocator>
    class HeapCell : public CellOperationsOnHeap<T, detail::HeapCellStorage<T>, Allocator> {
        using Super = CellOperationsOnHeap<T, detail::HeapCellStorage<T>, Allocator>;

    public:
        using Super::Super;
    };

} // namespace hk
