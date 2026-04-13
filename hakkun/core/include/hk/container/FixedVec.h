#pragma once

#include "hk/prim/VecOperations.h"

namespace hk {

    namespace detail {

        template <typename T, size Capacity>
        class FixedVecStorage : util::CustomTypeName {
            union {
                T mData[Capacity];
                T* mDataPtrForConsteval;
            };
            size mSize = 0;

        public:
            constexpr FixedVecStorage() {
                if consteval {
                    mDataPtrForConsteval = nullptr;
                }
            }

            constexpr ~FixedVecStorage() {
                if consteval {
                    if (mDataPtrForConsteval != nullptr)
                        std::allocator<T>().deallocate(mDataPtrForConsteval, Capacity);
                }
            }

            hk_alwaysinline constexpr T* getData() {
                if consteval {
                    if (mDataPtrForConsteval == nullptr) {
                        mDataPtrForConsteval = std::allocator<T>().allocate(Capacity);
                    }
                    return mDataPtrForConsteval;
                } else {
                    return mData;
                }
            }
            const hk_alwaysinline constexpr T* getDataConst() const {
                if consteval {
                    if (mDataPtrForConsteval == nullptr)
                        mDataPtrForConsteval = std::allocator<T>().allocate(Capacity);
                    return mDataPtrForConsteval;
                } else {
                    return mData;
                }
            }
            hk_alwaysinline constexpr void setSize(::size size) { mSize = size; }
            hk_alwaysinline constexpr size getSize() const { return mSize; }
            constexpr static size getCapacity() { return Capacity; }

            static constexpr const char cTypeName[] = "hk::FixedVec";
        };

    } // namespace detail

    /**
     * @brief Heapless vector.
     *
     * @tparam T
     * @tparam Capacity
     */
    template <typename T, size Capacity>
    class FixedVec : public VecOperations<T, detail::FixedVecStorage<T, Capacity>> {

    public:
        using Super = VecOperations<T, detail::FixedVecStorage<T, Capacity>>;

        using Super::Super;

        friend Super;
        friend Super::Super;
        friend Super::Super::Super;
        friend Super::Super::Super::Super;
        friend Super::Super::Super::Super::Super;
    };

    template <typename T, size N>
    FixedVec(T (&data)[N]) -> FixedVec<T, N>;
    template <typename T, size N>
    FixedVec(const T (&data)[N]) -> FixedVec<T, N>;

} // namespace hk
