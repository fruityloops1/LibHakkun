#pragma once

#include "hk/diag/diag.h"
#include "hk/util/Algorithm.h"
#include "hk/util/Math.h"
#include "hk/util/TypeName.h"
#include <span>

namespace hk {

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
            friend SpanOperations<T, Storage>;

            using Storage::cTypeName;

            constexpr SpanOperationsBase() = default;
            NON_COPYABLE(SpanOperationsBase);
            NON_MOVABLE(SpanOperationsBase);

            const hk_alwaysinline constexpr T* data() const { return getDataConst(); }
            hk_alwaysinline constexpr size size_bytes() const { return getSize() * sizeof(T); }
            hk_alwaysinline constexpr size size() const { return getSize(); }
            hk_alwaysinline constexpr ::size length() const { return getSize(); }
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
                HK_ABORT_UNLESS(amount <= getSize() - offset, "%s<%s>::slice(%zu, %zu): out of range (size: %zu)", util::getTypeName<Storage>(), util::getTypeName<T>(), offset, amount, getSize());
                return Span<const T>(getDataConst() + offset, amount);
            }

            hk_alwaysinline constexpr Span<const T> operator[](::size offset, ::size amount) const {
                return slice(offset, amount);
            }

            hk_alwaysinline constexpr operator Span<const T>() const { return Span<const T>(getDataConst(), getSize()); }

            constexpr const T& operator[](::size index) const {
                if not consteval {
                    HK_ABORT_UNLESS(index < getSize(), "%s<%s>::operator[%zu]: out of range (size: %zu)", util::getTypeName<Storage>(), util::getTypeName<T>(), index, getSize());
                }
                return getDataConst()[index];
            }

            constexpr const T& at(::size index) const {
                if not consteval {
                    HK_ABORT_UNLESS(index < getSize(), "%s<%s>::at(%zu): out of range (size: %zu)", util::getTypeName<Storage>(), util::getTypeName<T>(), index, getSize());
                }
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
                HK_ABORT_UNLESS(!empty(), "%s<%s>::first(): empty", util::getTypeName<Storage>(), util::getTypeName<T>());
                return getDataConst()[0];
            }

            constexpr const T& last() const {
                HK_ABORT_UNLESS(!empty(), "%s<%s>::last(): empty", util::getTypeName<Storage>(), util::getTypeName<T>());
                return getDataConst()[getSize() - 1];
            }

            template <typename Callback>
            constexpr void forEach(Callback func) const {
                for (::size i = 0; i < getSize(); i++)
                    func(at(i));
            }

            constexpr void copy(MutableType* out, ::size max = -1) const {
                util::copy(out, getDataConst(), util::min(max, getSize()));
            }

            template <::size N>
            constexpr void copy(MutableType (&out)[N]) const {
                copy(out, N);
            }

            constexpr bool includes(const T& value) const {
                for (const T& cmp : *this)
                    if (value == cmp)
                        return true;
                return false;
            }

        protected:
            using Storage::getData;
            using Storage::getDataConst;
            using Storage::getSize;
        };

    } // namespace detail

    template <typename T, typename Storage>
    struct SpanOperations : detail::SpanOperationsBase<T, Storage> {
        using Super = detail::SpanOperationsBase<T, Storage>;

        using Super::Super;

        hk_alwaysinline constexpr operator Span<T>() { return Span<T>(getData(), getSize()); }

        using Super::operator[];
        constexpr T& operator[](::size index) {
            HK_ABORT_UNLESS(index < getSize(), "%s<%s>::operator[%zu]: out of range (size: %zu)", util::getTypeName<Storage>(), util::getTypeName<T>(), index, getSize());
            return getData()[index];
        }

        using Super::at;
        constexpr T& at(::size index) {
            HK_ABORT_UNLESS(index < getSize(), "%s<%s>::at(%zu): out of range (size: %zu)", util::getTypeName<Storage>(), util::getTypeName<T>(), index, getSize());
            return getData()[index];
        }

        constexpr T& set(::size index, const T& value) {
            HK_ABORT_UNLESS(index < getSize(), "%s<%s>::set(%zu): out of range (size: %zu)", util::getTypeName<Storage>(), util::getTypeName<T>(), index, getSize());
            T* dest = getData() + index;
            dest->~T();
            return *construct_at(dest, value);
        }

        constexpr T& set(::size index, T&& value) {
            HK_ABORT_UNLESS(index < getSize(), "%s<%s>::set(%zu): out of range (size: %zu)", util::getTypeName<Storage>(), util::getTypeName<T>(), index, getSize());
            T* dest = getData() + index;
            dest->~T();
            return *construct_at(dest, forward<T>(value));
        }

        constexpr void setCopy(Span<const T> data) {
            size amount = util::min(getSize(), data.size());
            util::copy(getData(), data.data(), amount);
        }

        constexpr void setMove(Span<T> data) {
            size amount = util::min(getSize(), data.size());
            util::move(getData(), data.data(), amount);
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

        using Super::slice;
        hk_alwaysinline constexpr Span<T> slice(::size offset, ::size amount = 0) {
            if (amount == 0)
                amount = getSize() - offset;
            HK_ABORT_UNLESS(amount <= getSize() - offset, "%s<%s>::slice(%zu, %zu): out of range (size: %zu)", util::getTypeName<Storage>(), util::getTypeName<T>(), offset, amount, getSize());
            return Span<T>(getData() + offset, amount);
        }

        hk_alwaysinline constexpr Span<T> operator[](::size offset, ::size amount) {
            return slice(offset, amount);
        }

        using Super::empty;

        using Super::first;
        constexpr T& first() {
            HK_ABORT_UNLESS(!empty(), "%s<%s>::first(): empty", util::getTypeName<Storage>(), util::getTypeName<T>());
            return at(0);
        }

        using Super::last;
        constexpr T& last() {
            HK_ABORT_UNLESS(!empty(), "%s<%s>::last(): empty", util::getTypeName<Storage>(), util::getTypeName<T>());
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

        constexpr void reverse() {
            util::reverseMove(getData(), getSize());
        }

        constexpr void fill(size dstIdx, size amount, const T& value = T()) {
            if (amount == 0)
                return;
            HK_ABORT_UNLESS(dstIdx < getSize(), "%s<%s>::fill(%zu, %zu): destination out of bounds (size: %zu)", util::getTypeName<Storage>(), util::getTypeName<T>(), dstIdx, amount, getSize());
            HK_ABORT_UNLESS(dstIdx + (amount - 1) < getSize(), "%s<%s>::fill(%zu, %zu): destination out of bounds (size: %zu)", util::getTypeName<Storage>(), util::getTypeName<T>(), dstIdx, amount, getSize());

            util::fill(getData() + dstIdx, amount, value);
        }

        constexpr void fill(const T& value = T()) {
            fill(0, getSize(), value);
        }

        using Super::copy;
        constexpr void copy(size dstIdx, size srcIdx, size amount) {
            if (dstIdx == srcIdx or amount == 0)
                return;

            HK_ABORT_UNLESS(dstIdx < getSize(), "%s<%s>::copy(%zu, %zu, %zu): destination out of bounds (size: %zu)", util::getTypeName<Storage>(), util::getTypeName<T>(), dstIdx, srcIdx, amount, getSize());
            HK_ABORT_UNLESS(srcIdx < getSize(), "%s<%s>::copy(%zu, %zu, %zu): source out of bounds (size: %zu)", util::getTypeName<Storage>(), util::getTypeName<T>(), dstIdx, srcIdx, amount, getSize());
            HK_ABORT_UNLESS(dstIdx + (amount - 1) < getSize(), "%s<%s>::copy(%zu, %zu, %zu): destination out of bounds (size: %zu)", util::getTypeName<Storage>(), util::getTypeName<T>(), dstIdx, srcIdx, amount, getSize());
            HK_ABORT_UNLESS(srcIdx + (amount - 1) < getSize(), "%s<%s>::copy(%zu, %zu, %zu): source out of bounds (size: %zu)", util::getTypeName<Storage>(), util::getTypeName<T>(), dstIdx, srcIdx, amount, getSize());

            util::copyOverlapping(getData() + dstIdx, getData() + srcIdx, amount);
        }

    protected:
        using Super::getData;
        using Super::getSize;

        constexpr void move(size dstIdx, size srcIdx, size amount) {
            if (dstIdx == srcIdx or amount == 0)
                return;

            HK_ABORT_UNLESS(dstIdx < getSize(), "%s<%s>::move(%zu, %zu, %zu): destination out of bounds (size: %zu)", util::getTypeName<Storage>(), util::getTypeName<T>(), dstIdx, srcIdx, amount, getSize());
            HK_ABORT_UNLESS(srcIdx < getSize(), "%s<%s>::move(%zu, %zu, %zu): source out of bounds (size: %zu)", util::getTypeName<Storage>(), util::getTypeName<T>(), dstIdx, srcIdx, amount, getSize());
            HK_ABORT_UNLESS(dstIdx + (amount - 1) < getSize(), "%s<%s>::move(%zu, %zu, %zu): destination out of bounds (size: %zu)", util::getTypeName<Storage>(), util::getTypeName<T>(), dstIdx, srcIdx, amount, getSize());
            HK_ABORT_UNLESS(srcIdx + (amount - 1) < getSize(), "%s<%s>::move(%zu, %zu, %zu): source out of bounds (size: %zu)", util::getTypeName<Storage>(), util::getTypeName<T>(), dstIdx, srcIdx, amount, getSize());

            util::constructMoveOverlapping<true>(getData() + dstIdx, getData() + srcIdx, amount);
        }
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
                set(other.getData(), other.getSize());
                return *this;
            }

            NON_MOVABLE(SpanOperationsWithBufferPointerBase);

            constexpr SpanOperationsWithBufferPointerBase(T* data, size size)
                : Super() {
                set(data, size);
            }

            template <size N>
            constexpr SpanOperationsWithBufferPointerBase(T (&data)[N])
                : SpanOperationsWithBufferPointerBase(data, N) { }

#define HK_DETAIL_SPANOPERATIONSWITHBUFFERPOINTERBASE_DEDUCTION_GUIDE(TYPE, DESIRED) \
    template <typename T>                                                            \
    TYPE(T*, ::size)->DESIRED;                                                       \
    template <typename T, ::size N>                                                  \
    TYPE(T(&)[N])->DESIRED

            constexpr void set(T* data, size size) {
                setData(data);
                setSize(size);
            }

            constexpr void set(Span<T> data) {
                setData(data.data());
                setSize(data.size());
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

        constexpr SpanOperationsWithBufferPointer(std::initializer_list<T> data)
            : Super(data.begin(), data.size()) { }
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

        constexpr SpanOperationsWithBufferPointer(std::initializer_list<T> data)
            : Super(data.begin(), data.size()) { }
    };

#define HK_SPANOPERATIONSWITHBUFFERPOINTER_DEDUCTION_GUIDE(TYPE, DESIRED)         \
    HK_DETAIL_SPANOPERATIONSWITHBUFFERPOINTERBASE_DEDUCTION_GUIDE(TYPE, DESIRED); \
    template <typename T>                                                         \
    TYPE(const ::std::span<T>&)->DESIRED;                                         \
    template <typename T>                                                         \
    TYPE(const ::std::span<const T>&)->DESIRED;                                   \
    template <typename T>                                                         \
    TYPE(std::initializer_list<T>)->DESIRED

    namespace detail {

        template <typename T, typename Storage, bool HasBufferPointer>
        struct SpanOperationsConditionalBufferPointer;

        template <typename T, typename Storage>
        struct SpanOperationsConditionalBufferPointer<T, Storage, true> : SpanOperationsWithBufferPointer<T, Storage> {
            using Super = SpanOperationsWithBufferPointer<T, Storage>;

            using Super::Super;
        };

        template <typename T, typename Storage>
        struct SpanOperationsConditionalBufferPointer<T, Storage, false> : SpanOperations<T, Storage> {
            using Super = SpanOperations<T, Storage>;

            using Super::Super;
        };

    } // namespace detail

    template <typename T, typename Storage>
        requires(not std::is_const_v<T>)
    struct SpanOperationsCopyable : SpanOperations<T, Storage> {
        using Super = SpanOperations<T, Storage>;

        using Super::Super;

        constexpr SpanOperationsCopyable(const T* data, size size) {
            util::constructCopy(getData(), data, util::min(size, getSize()));
        }

        constexpr SpanOperationsCopyable(T* data, size size)
            : SpanOperationsCopyable(data, size) {
        }

        template <size N>
        constexpr SpanOperationsCopyable(const T (&data)[N])
            : SpanOperationsCopyable(data, N) { }

        template <size N>
        constexpr SpanOperationsCopyable(T (&data)[N])
            : SpanOperationsCopyable(data, N) { }

        constexpr SpanOperationsCopyable(std::initializer_list<T> data)
            : SpanOperationsCopyable(data.begin(), data.size()) { }

        constexpr SpanOperationsCopyable(const std::span<const T>& other)
            : SpanOperationsCopyable(other.data(), other.size()) { }
        constexpr SpanOperationsCopyable(const std::span<T>& other)
            : SpanOperationsCopyable(other.data(), other.size()) { }

        constexpr SpanOperationsCopyable(Span<const T> other)
            : SpanOperationsCopyable(other.data(), other.size()) { }
        constexpr SpanOperationsCopyable(Span<T> other)
            : SpanOperationsCopyable(other.data(), other.size()) { }

        constexpr SpanOperationsCopyable(const SpanOperationsCopyable& other)
            : SpanOperationsCopyable(other.getDataConst(), other.getSize()) { }

        constexpr SpanOperationsCopyable& operator=(const SpanOperationsCopyable& other) {
            setCopy(other);
            return *this;
        }

        constexpr SpanOperationsCopyable(SpanOperationsCopyable&& other) {
            util::constructMove(getData(), other.getData(), util::min(other.getSize(), getSize()));
        }

        constexpr SpanOperationsCopyable& operator=(SpanOperationsCopyable&& other) {
            setMove(other);
            return *this;
        }

        using Super::setCopy;
        using Super::setMove;

    protected:
        using Super::getData;
        using Super::getSize;
    };

#define HK_SPANOPERATIONSCOPYABLE_DEDUCTION_GUIDE(TYPE, DESIRED) \
    template <typename T>                                        \
    TYPE(const T*, ::size)->DESIRED;                             \
    template <typename T>                                        \
    TYPE(T*, ::size)->DESIRED;                                   \
    template <typename T, ::size N>                              \
    TYPE(const T(&data)[N])->DESIRED;                            \
    template <typename T, ::size N>                              \
    TYPE(T(&)[N])->DESIRED;                                      \
    template <typename T>                                        \
    TYPE(::hk::Span<T>)->DESIRED;                                \
    template <typename T>                                        \
    TYPE(::hk::Span<const T>)->DESIRED;                          \
    template <typename T>                                        \
    TYPE(const ::std::span<T>&)->DESIRED;                        \
    template <typename T>                                        \
    TYPE(const ::std::span<const T>&)->DESIRED;                  \
    template <typename T>                                        \
    TYPE(::std::initializer_list<T>)->DESIRED;

} // namespace hk
