#pragma once

#include "hk/diag/diag.h"
#include "hk/types.h"
#include "hk/util/Algorithm.h"
#include "hk/util/Math.h"
#include "hk/util/TypeName.h"
#include <iterator>
#include <span>
#include <type_traits>

namespace hk::util {

    template <typename T, typename Storage>
    struct SpanOperations;
    template <typename T, typename Storage>
    struct SpanOperationsWithBufferPointer;

    namespace detail {
        template <typename T, typename Storage>
        struct SpanOperationsWithBufferPointerBase;

        template <typename T>
        struct SpanStorage;
    } // namespace detail

    template <typename T>
    struct Span;

    namespace detail {

        template <typename T, typename Storage>
        struct SpanOperationsBase : protected Storage {
            using Type = T;
            using MutableType = std::remove_const_t<T>;

            friend SpanOperationsWithBufferPointer<T, Storage>;
            friend SpanOperationsWithBufferPointerBase<T, Storage>;
            friend SpanOperations<T, Storage>;

            constexpr SpanOperationsBase() { setSize(0); }
            NON_COPYABLE(SpanOperationsBase);
            NON_MOVABLE(SpanOperationsBase);

            const hk_alwaysinline constexpr T* data() const { return getDataConst(); }
            hk_alwaysinline constexpr size size_bytes() const { return getSize() * sizeof(T); }
            hk_alwaysinline constexpr size size() const { return getSize(); }
            hk_alwaysinline constexpr bool empty() const { return getSize() == 0; }

            class ConstIterator {
            public:
                using iterator_category = std::forward_iterator_tag;
                using difference_type = ptrdiff;
                using value_type = T;
                using pointer = const T*;
                using reference = const T&;

            private:
                pointer mCur;

            public:
                constexpr ConstIterator(pointer cur)
                    : mCur(cur) { }

                constexpr ConstIterator& operator++() {
                    mCur++;
                    return *this;
                }
                constexpr ConstIterator operator++(s32) { ++*this; }

                constexpr bool operator==(const ConstIterator& other) const { return mCur == other.mCur; }
                constexpr bool operator!=(const ConstIterator& other) const { return !(*this == other); }

                constexpr reference operator*() const { return *mCur; }
                constexpr pointer operator->() const { return mCur; }
                constexpr operator pointer() const { return mCur; }
            };

            hk_alwaysinline constexpr ConstIterator begin() const { return { getDataConst() }; }
            hk_alwaysinline constexpr ConstIterator end() const { return { getDataConst() + getSize() }; }

            hk_alwaysinline constexpr Span<const T> slice(::size offset, ::size amount = 0) const {
                if (amount == 0)
                    amount = getSize() - offset;
                HK_ABORT_UNLESS(amount <= getSize() - offset, "hk::util::Span<%s>::slice(%zu, %zu): out of range (size: %zu)", getTypeName<T>(), offset, amount, getSize());
                return Span<const T>(getDataConst() + offset, amount);
            }

            hk_alwaysinline constexpr operator Span<const T>() const { return Span<const T>(getDataConst(), getSize()); }

            constexpr const T& operator[](::size index) const {
                HK_ABORT_UNLESS(index < getSize(), "hk::util::Span<%s>::operator[%zu]: out of range (size: %zu)", getTypeName<T>(), index, getSize());
                return getDataConst()[index];
            }

            constexpr const T& at(::size index) const {
                HK_ABORT_UNLESS(index < getSize(), "hk::util::Span<%s>::at(%zu): out of range (size: %zu)", getTypeName<T>(), index, getSize());
                return getDataConst()[index];
            }

            template <typename ST, typename GetFunc>
            constexpr ::size binarySearch(GetFunc getValue, ST searchValue, bool findBetween = false) const {
                return util::binarySearch([this, getValue](s32 index) {
                    return getValue(at(index));
                },
                    0, getSize() - 1, searchValue, findBetween);
            }

            constexpr const T& first() const {
                HK_ABORT_UNLESS(!empty(), "hk::util::Span<%s>::first(): empty", getTypeName<T>());
                return getDataConst()[0];
            }

            constexpr const T& last() const {
                HK_ABORT_UNLESS(!empty(), "hk::util::Span<%s>::last(): empty", getTypeName<T>());
                return getDataConst()[getSize() - 1];
            }

            template <typename Callback>
            constexpr void forEach(Callback func) const {
                for (::size i = 0; i < getSize(); i++)
                    func(at(i));
            }

            constexpr void copy(MutableType* out, ::size max = -1) const {
                copy(out, getDataConst(), hk::util::min(max, getSize()));
            }

            template <::size N>
            constexpr void copy(MutableType (&out)[N]) const {
                copy(out, N);
            }

        protected:
            using Storage::getData;
            using Storage::getDataConst;
            using Storage::getSize;
            using Storage::setSize;
        };

    } // namespace detail

    template <typename T, typename Storage>
    struct SpanOperations : detail::SpanOperationsBase<T, Storage> {
        using Super = detail::SpanOperationsBase<T, Storage>;

        using Super::Super;

        hk_alwaysinline constexpr operator Span<T>() { return Span<T>(getData(), getSize()); }

        using Super::operator[];
        constexpr T& operator[](::size index) {
            HK_ABORT_UNLESS(index < getSize(), "hk::util::Span<%s>::operator[%zu]: out of range (size: %zu)", getTypeName<T>(), index, getSize());
            return getData()[index];
        }

        using Super::at;
        constexpr T& at(::size index) {
            HK_ABORT_UNLESS(index < getSize(), "hk::util::Span<%s>::at(%zu): out of range (size: %zu)", getTypeName<T>(), index, getSize());
            return getData()[index];
        }

        constexpr void set(::size index, const T& value) {
            HK_ABORT_UNLESS(index < getSize(), "hk::util::Span<%s>::set(%zu): out of range (size: %zu)", getTypeName<T>(), index, getSize());
            getData()[index] = value;
        }

        constexpr void set(::size index, T&& value) {
            HK_ABORT_UNLESS(index < getSize(), "hk::util::Span<%s>::set(%zu): out of range (size: %zu)", getTypeName<T>(), index, getSize());
            new (&getData()[index]) T(value);
        }

        using Super::data;
        constexpr T* data() { return getData(); }

        class Iterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using difference_type = ptrdiff;
            using value_type = T;
            using pointer = T*;
            using reference = T&;

        private:
            pointer mCur;

        public:
            constexpr Iterator(pointer cur)
                : mCur(cur) { }

            constexpr Iterator& operator++() {
                mCur++;
                return *this;
            }

            constexpr bool operator==(const Iterator& other) const { return mCur == other.mCur; }
            constexpr bool operator!=(const Iterator& other) const { return !(*this == other); }

            constexpr reference operator*() const { return *mCur; }
            constexpr pointer operator->() const { return mCur; }
            constexpr operator pointer() const { return mCur; }
        };

        using Super::begin;
        using Super::end;
        constexpr Iterator begin() { return { getData() }; }
        constexpr Iterator end() { return { getData() + getSize() }; }

        hk_alwaysinline constexpr Span<T> slice(::size offset, ::size amount = 0) {
            if (amount == 0)
                amount = getSize() - offset;
            HK_ABORT_UNLESS(amount <= getSize() - offset, "hk::util::Span<%s>::slice(%zu, %zu): out of range (size: %zu)", getTypeName<T>(), offset, amount, getSize());
            return Span<T>(getData() + offset, amount);
        }

        using Super::empty;

        using Super::first;
        constexpr T& first() {
            HK_ABORT_UNLESS(!empty(), "hk::util::Span<%s>::first(): empty", getTypeName<T>());
            return at(0);
        }

        using Super::last;
        constexpr T& last() {
            HK_ABORT_UNLESS(!empty(), "hk::util::Span<%s>::last(): empty", getTypeName<T>());
            return at(getSize() - 1);
        }

        template <typename Callback>
        constexpr void forEach(Callback func) {
            for (::size i = 0; i < getSize(); i++)
                func(at(i));
        }

        constexpr void sort() {
            std::sort(getData(), getData() + getSize());
        }

        template <typename Compare>
        constexpr void sort(Compare comp) {
            std::sort(getData(), getData() + getSize(), comp);
        }

        constexpr void fill(size dstIdx, size amount, const T& value = T()) {
            if (amount == 0)
                return;
            HK_ABORT_UNLESS(dstIdx < getSize(), "hk::util::Span<%s>::fill(%zu, %zu): destination out of bounds (size: %zu)", getTypeName<T>(), dstIdx, amount, getSize());
            HK_ABORT_UNLESS(dstIdx + (amount - 1) < getSize(), "hk::util::Span<%s>::fill(%zu, %zu): destination out of bounds (size: %zu)", getTypeName<T>(), dstIdx, amount, getSize());

            util::fill(getData() + dstIdx, amount, value);
        }

        constexpr void fill(const T& value = T()) {
            fill(0, getSize(), value);
        }

        constexpr void move(size dstIdx, size srcIdx, size amount) {
            if (dstIdx == srcIdx or amount == 0)
                return;

            HK_ABORT_UNLESS(dstIdx < getSize(), "hk::util::Span<%s>::move(%zu, %zu, %zu): destination out of bounds (size: %zu)", getTypeName<T>(), dstIdx, srcIdx, amount, getSize());
            HK_ABORT_UNLESS(srcIdx < getSize(), "hk::util::Span<%s>::move(%zu, %zu, %zu): source out of bounds (size: %zu)", getTypeName<T>(), dstIdx, srcIdx, amount, getSize());
            HK_ABORT_UNLESS(dstIdx + (amount - 1) < getSize(), "hk::util::Span<%s>::move(%zu, %zu, %zu): destination out of bounds (size: %zu)", getTypeName<T>(), dstIdx, srcIdx, amount, getSize());
            HK_ABORT_UNLESS(srcIdx + (amount - 1) < getSize(), "hk::util::Span<%s>::move(%zu, %zu, %zu): source out of bounds (size: %zu)", getTypeName<T>(), dstIdx, srcIdx, amount, getSize());

            moveOverlapping(getData() + dstIdx, getData() + srcIdx, amount);
        }

        constexpr void copy(size dstIdx, size srcIdx, size amount) {
            if (dstIdx == srcIdx or amount == 0)
                return;

            HK_ABORT_UNLESS(dstIdx < getSize(), "hk::util::Span<%s>::copy(%zu, %zu, %zu): destination out of bounds (size: %zu)", getTypeName<T>(), dstIdx, srcIdx, amount, getSize());
            HK_ABORT_UNLESS(srcIdx < getSize(), "hk::util::Span<%s>::copy(%zu, %zu, %zu): source out of bounds (size: %zu)", getTypeName<T>(), dstIdx, srcIdx, amount, getSize());
            HK_ABORT_UNLESS(dstIdx + (amount - 1) < getSize(), "hk::util::Span<%s>::copy(%zu, %zu, %zu): destination out of bounds (size: %zu)", getTypeName<T>(), dstIdx, srcIdx, amount, getSize());
            HK_ABORT_UNLESS(srcIdx + (amount - 1) < getSize(), "hk::util::Span<%s>::copy(%zu, %zu, %zu): source out of bounds (size: %zu)", getTypeName<T>(), dstIdx, srcIdx, amount, getSize());

            copyOverlapping(getData() + dstIdx, getData() + srcIdx, amount);
        }

    protected:
        using Super::getData;
        using Super::getSize;
    };

    template <typename T, typename Storage>
    struct SpanOperations<const T, Storage> : detail::SpanOperationsBase<const T, Storage> {
        using Super = detail::SpanOperationsBase<const T, Storage>;

        using Super::Super;
    };

    namespace detail {

        template <typename T, typename Storage>
        struct SpanOperationsWithBufferPointerBase : SpanOperations<T, Storage> {
            using Super = SpanOperations<T, Storage>;

            using Super::Super;

            constexpr SpanOperationsWithBufferPointerBase()
                : Super() {
                setData(nullptr);
            }

            constexpr SpanOperationsWithBufferPointerBase(const SpanOperationsWithBufferPointerBase& other)
                : SpanOperationsWithBufferPointerBase(other.getData(), other.getSize()) { }
            constexpr SpanOperationsWithBufferPointerBase& operator=(const SpanOperationsWithBufferPointerBase& other) {
                set(other.data(), other.size());
                return *this;
            }

            constexpr SpanOperationsWithBufferPointerBase(SpanOperationsWithBufferPointerBase&& other)
                : SpanOperationsWithBufferPointerBase(other.data(), other.size()) { }
            constexpr SpanOperationsWithBufferPointerBase& operator=(SpanOperationsWithBufferPointerBase&& other) {
                set(other.data(), other.size());
                return *this;
            }

            constexpr SpanOperationsWithBufferPointerBase(T* data, size size)
                : Super() {
                set(data, size);
            }

            template <size N>
            constexpr SpanOperationsWithBufferPointerBase(T (&data)[N])
                : SpanOperationsWithBufferPointerBase(data, N) { }

#define HK_UTIL_DETAIL_SPANOPERATIONSWITHBUFFERPOINTERBASE_DEDUCTION_GUIDE(TYPE, DESIRED) \
    template <typename T>                                                                 \
    TYPE(T*, size)->DESIRED;                                                              \
    template <typename T, size N>                                                         \
    TYPE(T(&)[N])->DESIRED

            constexpr void set(T* data, size size) {
                setData(data);
                setSize(size);
            }

        protected:
            using Super::getData;
            using Super::setSize;

            using Super::setData;
        };

    } // namespace detail

    template <typename T, typename Storage>
    struct SpanOperationsWithBufferPointer : detail::SpanOperationsWithBufferPointerBase<T, Storage> {
        using Super = detail::SpanOperationsWithBufferPointerBase<T, Storage>;

        using Super::Super;

        constexpr SpanOperationsWithBufferPointer(const detail::SpanOperationsWithBufferPointerBase<const T, Storage>& other) = delete;
        constexpr SpanOperationsWithBufferPointer(const Super& other)
            : Super(other.getData(), other.getSize()) { }

        constexpr SpanOperationsWithBufferPointer(const std::span<const T>& other) = delete;
        constexpr SpanOperationsWithBufferPointer(const std::span<T>& other)
            : Super(other.data(), other.size()) { }
    };

    template <typename T, typename Storage>
    struct SpanOperationsWithBufferPointer<const T, Storage> : detail::SpanOperationsWithBufferPointerBase<const T, Storage> {
        using Super = detail::SpanOperationsWithBufferPointerBase<const T, Storage>;

        using Super::Super;

        constexpr SpanOperationsWithBufferPointer(detail::SpanOperationsWithBufferPointerBase<T, Storage> other)
            : Super(other.data(), other.size()) { }
        constexpr SpanOperationsWithBufferPointer(const Super& other)
            : Super(other.getDataConst(), other.getSize()) { }

        constexpr SpanOperationsWithBufferPointer(const std::span<const T>& other)
            : Super(other.data(), other.size()) { }
        constexpr SpanOperationsWithBufferPointer(const std::span<T>& other)
            : Super(other.data(), other.size()) { }
    };

#define HK_UTIL_SPANOPERATIONSWITHBUFFERPOINTER_DEDUCTION_GUIDE(TYPE, DESIRED)         \
    HK_UTIL_DETAIL_SPANOPERATIONSWITHBUFFERPOINTERBASE_DEDUCTION_GUIDE(TYPE, DESIRED); \
    template <typename T>                                                              \
    TYPE(const ::std::span<T>&)->DESIRED;                                              \
    template <typename T>                                                              \
    TYPE(const ::std::span<const T>&)->DESIRED

    namespace detail {

        template <typename T>
        class SpanStorage {
            T* mData;
            size mSize;

        public:
            hk_alwaysinline constexpr void setData(T* data) { mData = data; }
            hk_alwaysinline constexpr T* getData() const { return mData; }
            const hk_alwaysinline constexpr T* getDataConst() const { return mData; }
            hk_alwaysinline constexpr void setSize(::size size) { mSize = size; }
            hk_alwaysinline constexpr ::size getSize() const { return mSize; }
        };

    } // namespace detail

    template <typename T>
    class Span : public SpanOperationsWithBufferPointer<T, detail::SpanStorage<T>> {

    public:
        using Super = SpanOperationsWithBufferPointer<T, detail::SpanStorage<T>>;

        using Super::Super;

        friend Super;
        friend Super::Super;
        friend Super::Super::Super;
        friend Super::Super::Super::Super;
    };

    HK_UTIL_SPANOPERATIONSWITHBUFFERPOINTER_DEDUCTION_GUIDE(Span, Span<T>);

    template <>
    class Span<void> : public Span<u8> {
    public:
        constexpr Span() = default;
        Span(void* data, ::size size)
            : Span<u8>(cast<u8*>(data), size) { }
    };

    template <>
    class Span<const void> : public Span<const u8> {
    public:
        constexpr Span() = default;
        Span(const void* data, ::size size)
            : Span<const u8>(cast<const u8*>(data), size) { }
    };

    static_assert(([]() consteval -> bool {
        u8 arr[128] { 4, 3, 2, 1 };

        std::span<u8> stl(arr);
        Span<u8> mut(arr);
        { Span<const u8> view(arr); }
        { Span<u8> view(stl); }
        { Span<const u8> view(stl); }

        mut.fill(64, 64, 42);
        mut.sort();

        HK_ASSERT(mut.first() == 0 && mut.last() == 42);

        Span<const u8> view(mut);
        mut.fill(132);

        return view[42] == 132;
    })());

} // namespace hk::util

template <typename To, typename From>
hk_alwaysinline hk::util::Span<To> cast(hk::util::Span<From> value) {
    return { cast<To*>(value.data()), value.size_bytes() / sizeof(To) };
}
