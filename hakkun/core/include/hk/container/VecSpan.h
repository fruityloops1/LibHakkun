#pragma once

#include "hk/prim/VecOperations.h"

namespace hk {

    namespace detail {

        template <typename T>
        class VecSpanStorage : util::CustomTypeName {
            T* mData = nullptr;
            size mSize = 0;
            size mCapacity = 0;

        public:
            hk_alwaysinline constexpr void setData(T* data) { mData = data; }
            hk_alwaysinline constexpr T* getData() const { return mData; }
            const hk_alwaysinline constexpr T* getDataConst() const { return mData; }
            hk_alwaysinline constexpr void setSize(::size size) { mSize = size; }
            hk_alwaysinline constexpr size getSize() const { return mSize; }
            hk_alwaysinline constexpr size getCapacity() const { return mCapacity; }
            hk_alwaysinline constexpr void setCapacity(size capacity) { mCapacity = capacity; }

            static constexpr const char cTypeName[] = "hk::VecSpan";
        };

    } // namespace detail

    /**
     * @brief Fixed vector over a memory region.
     *
     * @tparam T
     * @tparam Capacity
     */
    template <typename T>
    class VecSpan : public VecOperationsWithBufferPointer<T, detail::VecSpanStorage<T>, false> {

    public:
        using Super = VecOperationsWithBufferPointer<T, detail::VecSpanStorage<T>, false>;

        NON_MOVABLE(VecSpan);

        constexpr void set(T* data, size size) {
            setData(data);
            setSize(0);
            setCapacity(size);
        }

        constexpr void set(T* data, size size, ::size capacity) {
            setData(data);
            setSize(size);
            setCapacity(capacity);
        }

        constexpr VecSpan() { set(nullptr, 0, 0); }

        constexpr VecSpan(const VecSpan& other)
            : VecSpan(other.getData(), other.getSize(), other.getCapacity()) { }

        constexpr VecSpan& operator=(const VecSpan& other) {
            setData(other.getData());
            setSize(other.getSize());
            setCapacity(other.getCapacity());

            return *this;
        }

        constexpr VecSpan(T* data, size size, ::size capacity) { set(data, size, capacity); }

        constexpr VecSpan(T* data, size size)
            : VecSpan(data, 0, size) { }

        template <size N>
        constexpr VecSpan(T (&data)[N])
            : VecSpan(data, 0, N) { }

        template <size N>
        constexpr VecSpan(T (&data)[N], size capacity)
            : VecSpan(data, N, capacity) { }

        constexpr VecSpan(std::initializer_list<T> data)
            : VecSpan(data.begin(), data.size()) { }

        constexpr VecSpan(std::initializer_list<T> data, size capacity)
            : VecSpan(data.begin(), data.size(), capacity) { }

        constexpr VecSpan(const std::span<T>& other)
            : Super(other.data(), other.size()) { }

        constexpr VecSpan(const std::span<T>& other, size capacity)
            : Super(other.data(), other.size(), capacity) { }

        constexpr VecSpan(const Span<T>& other)
            : Super(other.data(), other.size()) { }

        constexpr VecSpan(const Span<T>& other, size capacity)
            : Super(other.data(), other.size(), capacity) { }

        friend Super;
        friend Super::Super;
        friend Super::Super::Super;
        friend Super::Super::Super::Super;
        friend Super::Super::Super::Super::Super;

    private:
        using Storage = detail::VecSpanStorage<T>;

        using Storage::setCapacity;
        using Super::setData;
        using Super::setSize;
        using Super::Super;

        using Super::setCopy;
        using Super::setMove;
    };

} // namespace hk
