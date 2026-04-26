#pragma once

#include "hk/prim/CellOperations.h"
#include "hk/util/TypeName.h"

namespace hk {

    namespace detail {

        template <typename T>
        struct CellStorage : util::CustomTypeName {
            union {
                T mValue;
            };
            bool mAlive = false;

            constexpr T* getStorage() { return &mValue; }
            constexpr const T* getStorageConst() const { return &mValue; }
            constexpr void setAlive(bool alive) { mAlive = alive; }
            constexpr bool isAlive() const { return mAlive; }

            static constexpr char cTypeName[] = "hk::Cell";
        };

    } // namespace detail

    template <typename T>
    class Cell : public CellOperations<T, detail::CellStorage<T>> {
        using Super = CellOperations<T, detail::CellStorage<T>>;

    public:
        using Super::Super;
    };

} // namespace hk
