#pragma once

#include "hk/container/Span.h"
#include "hk/prim/VecOperations.h"
#include "hk/util/Allocator.h"

namespace hk {

    namespace detail {

        template <typename T, util::AllocatorType Allocator>
        class VecStorage {
            T* mData = nullptr;
            size mSize = 0;
            size mCapacity = 0;

        public:
            constexpr static Span<T> allocate(size size, ::size capacity) {
                capacity = util::max(size, capacity);
                T* data;
                if consteval {
                    data = std::allocator<T>().allocate(capacity);
                } else {
                    data = cast<T*>(Allocator::allocate(capacity * sizeof(T), alignof(T)));
                }

                HK_ABORT_UNLESS(data != nullptr, "%s<%s>::allocate(): allocation failed (Capacity = %zu)", cTypeName, util::getTypeName<T>(), capacity);

                return Span<T>(data, size);
            }

            constexpr static void free(Span<T> data, ::size capacity) {
                if consteval {
                    std::allocator<T>().deallocate(data.data(), capacity);
                } else {
                    Allocator::free(data.data());
                }
            }

            hk_alwaysinline constexpr T* getData() const { return mData; }
            hk_alwaysinline constexpr void setData(T* data) { mData = data; }
            const hk_alwaysinline constexpr T* getDataConst() const { return mData; }
            hk_alwaysinline constexpr void setSize(::size size) { mSize = size; }
            hk_alwaysinline constexpr void setCapacity(::size capacity) { mCapacity = capacity; }
            hk_alwaysinline constexpr size getSize() const { return mSize; }
            hk_alwaysinline constexpr size getCapacity() const { return mCapacity; }

            static constexpr const char cTypeName[] = "hk::Vec";
        };

    } // namespace detail

    /**
     * @brief Vector.
     *
     * @tparam T
     * @tparam ReserveSize Amount of elements to reserve by default or when pushing elements past the current capacity
     */
    template <typename T, size ReserveSize = 16, util::AllocatorType Allocator = util::DefaultAllocator>
    class Vec : public VecOperationsOnHeap<T, detail::VecStorage<T, Allocator>, ReserveSize, Allocator> {

    public:
        using Super = VecOperationsOnHeap<T, detail::VecStorage<T, Allocator>, ReserveSize, Allocator>;

        using Super::Super;

        friend Super;
        friend Super::Super;
        friend Super::Super::Super;
        friend Super::Super::Super::Super;
        friend Super::Super::Super::Super::Super;
    };

} // namespace hk
