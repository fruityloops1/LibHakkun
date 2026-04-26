#pragma once

#include "hk/prim/SpanOperations.h"
#include "hk/util/Algorithm.h"
#include "hk/util/Allocator.h"

namespace hk {

    namespace detail {

        template <typename T, typename Storage, bool HasBufferPointer>
            requires(not std::is_const_v<T>)
        struct VecOperationsBase : detail::SpanOperationsConditionalBufferPointer<T, Storage, HasBufferPointer> {
            using Super = detail::SpanOperationsConditionalBufferPointer<T, Storage, HasBufferPointer>;

            using Super::findIndex;
            using Super::Super;

            constexpr VecOperationsBase(size numElements) {
                resize(numElements);
            }

            constexpr ~VecOperationsBase() {
                clear();
            }

            constexpr T& add(const T& value) {
                HK_ABORT_UNLESS(getSize() < getCapacity(), "%s<%s>::add: Full (capacity: %zu)", util::getTypeName<Storage>(), util::getTypeName<T>(), getCapacity());
                setSize(getSize() + 1);
                return *construct_at(getData() + getSize() - 1, value);
            }

            constexpr T& add(T&& value) {
                HK_ABORT_UNLESS(getSize() < getCapacity(), "%s<%s>::add: Full (capacity: %zu)", util::getTypeName<Storage>(), util::getTypeName<T>(), getCapacity());

                setSize(getSize() + 1);
                return *construct_at(getData() + getSize() - 1, forward<T>(value));
            }

            constexpr T& pushBack(const T& value) { return add(value); }
            constexpr T& pushBack(T&& value) { return add(forward<T>(value)); }

            constexpr T& pushFront(const T& value) { return insert(value, 0); }
            constexpr T& pushFront(T&& value) { return insert(forward<T>(value), 0); }

            using Super::move;

            constexpr T& insert(const T& value, size index = 0) {
                HK_ABORT_UNLESS(getSize() < getCapacity(), "%s<%s>::add: Full (capacity: %zu)", util::getTypeName<Storage>(), util::getTypeName<T>(), getCapacity());
                HK_ABORT_UNLESS(index <= getSize(), "%s<%s>::insert(index: %zu): out of range (size: %zu)", util::getTypeName<Storage>(), util::getTypeName<T>(), index, getSize());
                setSize(getSize() + 1);
                move(index + 1, index, getSize() - 1 - index);
                return *construct_at(getData() + index, value);
            }

            constexpr T& insert(T&& value, size index = 0) {
                HK_ABORT_UNLESS(getSize() < getCapacity(), "%s<%s>::add: Full (capacity: %zu)", util::getTypeName<Storage>(), util::getTypeName<T>(), getCapacity());
                HK_ABORT_UNLESS(index <= getSize(), "%s<%s>::insert(index: %zu): out of range (size: %zu)", util::getTypeName<Storage>(), util::getTypeName<T>(), index, getSize());
                setSize(getSize() + 1);
                move(index + 1, index, getSize() - 1 - index);
                return *construct_at(getData() + index, forward<T>(value));
            }

            constexpr T remove(size index) {
                HK_ABORT_UNLESS(index < getSize(), "%s<%s>::remove(%zu): out of range (size: %zu)", util::getTypeName<Storage>(), util::getTypeName<T>(), index, getSize());

                T removedValue = T(::move(getData()[index]));
                getData()[index].~T();

                if (index < getSize() - 1)
                    move(index, index + 1, getSize() - index - 1);

                setSize(getSize() - 1);

                return T(::move(removedValue));
            }

            constexpr T removeByValue(const T& value) {
                size index = findIndex(value);
                HK_ABORT_UNLESS(index != -1, "%s<%s>::remove(const T&): value not found (size: %zu)", util::getTypeName<Storage>(), util::getTypeName<T>(), getSize());

                return T(::move(remove(index)));
            }

            constexpr bool tryRemoveByValue(const T& value) {
                size index = findIndex(value);
                if (index == -1)
                    return false;

                return T(::move(remove(index)));
            }

            constexpr T popBack() {
                HK_ABORT_UNLESS(getSize() > 0, "%s<%s>::popBack(): empty", util::getTypeName<Storage>(), util::getTypeName<T>());
                return T(::move(remove(getSize() - 1)));
            }

            constexpr T popFront() {
                HK_ABORT_UNLESS(getSize() > 0, "%s<%s>::popFront(): empty", util::getTypeName<Storage>(), util::getTypeName<T>());
                return T(::move(remove(0)));
            }

            constexpr void extend(size newSize, const T& extendValue = T()) {
                HK_ABORT_UNLESS(newSize >= getSize() && newSize <= getCapacity(), "%s<%s>::extend(%zu): out of range (size: %zu, capacity: %zu)", util::getTypeName<Storage>(), util::getTypeName<T>(), newSize, getSize(), getCapacity());
                util::fill(getData() + getSize(), newSize - getSize(), extendValue);

                setSize(newSize);
            }

            constexpr void truncate(size newSize) {
                HK_ABORT_UNLESS(newSize <= getSize(), "%s<%s>::truncate(%zu): newSize larger than size (size: %zu)", util::getTypeName<Storage>(), util::getTypeName<T>(), newSize, getSize());
                util::destroy(getData() + newSize, getSize() - newSize);

                setSize(newSize);
            }

            constexpr void resize(size newSize, const T& extendValue = T()) {
                if (getSize() == newSize)
                    return;

                if (getSize() > newSize)
                    truncate(newSize);
                else
                    extend(newSize, extendValue);
            }

            constexpr void clear() {
                util::destroy(getData(), getSize());
                setSize(0);
            }

            constexpr size capacity() const { return getCapacity(); }

        protected:
            using Storage::getCapacity;
            using Storage::getData;
            using Storage::getSize;
            using Storage::setSize;
        };

        template <typename T, typename Storage, bool HasBufferPointer>
        struct VecOperations;

        template <typename T, typename Storage>
        struct VecOperations<T, Storage, true> : VecOperationsBase<T, Storage, true> {
            using Super = VecOperationsBase<T, Storage, true>;

            using Super::Super;

            constexpr VecOperations(const T* data, size size) = delete;
            constexpr VecOperations(T* data, size size) = delete;
            constexpr VecOperations(std::initializer_list<T> data) = delete;
            template <size N>
            constexpr VecOperations(const T (&data)[N]) = delete;
            template <size N>
            constexpr VecOperations(T (&data)[N]) = delete;
            constexpr VecOperations(const std::span<const T>& other) = delete;
            constexpr VecOperations(const std::span<T>& other) = delete;
        };

        template <typename T, typename Storage>
        struct VecOperations<T, Storage, false> : VecOperationsBase<T, Storage, false> {
            using Super = VecOperationsBase<T, Storage, false>;

            using Super::Super;

            constexpr VecOperations(const T* data, size size) {
                setSize(util::min(size, getCapacity()));
                util::constructCopy(getData(), data, getSize());
            }

            constexpr VecOperations(T* data, size size)
                : VecOperations(data, size) {
            }

            template <size N>
            constexpr VecOperations(const T (&data)[N])
                : VecOperations(data, N) { }

            constexpr VecOperations(std::initializer_list<T> data)
                : VecOperations(data.begin(), data.size()) { }

            constexpr VecOperations(const std::span<const T>& other)
                : VecOperations(other.data(), other.size()) { }
            constexpr VecOperations(const std::span<T>& other)
                : VecOperations(other.data(), other.size()) { }

            constexpr VecOperations(const hk::Span<const T>& other)
                : VecOperations(other.data(), other.size()) { }
            constexpr VecOperations(const hk::Span<T>& other)
                : VecOperations(other.data(), other.size()) { }

            constexpr VecOperations(const VecOperations& other)
                : VecOperations(other.getDataConst(), other.getSize()) { }

            constexpr VecOperations& operator=(const VecOperations& other) {
                setCopy(other);
                return *this;
            }

            constexpr VecOperations(VecOperations&& other) {
                setSize(util::min(other.getSize(), getCapacity()));
                util::constructMove(getData(), other.getData(), getSize());
            }

            constexpr VecOperations& operator=(VecOperations&& other) {
                setMove(other);
                return *this;
            }

            constexpr void setCopy(Span<const T> data) {
                setSize(util::min(data.size(), getCapacity()));
                util::copy(getData(), data.data(), getSize());
            }

            constexpr void setMove(Span<T> data) {
                setSize(util::min(data.size(), getCapacity()));
                util::move(getData(), data.data(), getSize());
            }

            static constexpr size cCapacity = Storage::getCapacity();

        protected:
            using Super::getCapacity;
            using Super::getData;
            using Super::getSize;
            using Super::setSize;

        private:
            using Super::setCopy;
            using Super::setMove;
        };

#define HK_VECOPERATIONS_DEDUCTION_GUIDE(TYPE, DESIRED)                \
    HK_SPANOPERATIONSWITHBUFFERPOINTER_DEDUCTION_GUIDE(TYPE, DESIRED); \
    template <typename T, ::size N>                                    \
    TYPE(const T(&data)[N])->DESIRED;                                  \
    template <typename T>                                              \
    TYPE(::std::initializer_list<T>)->DESIRED;                         \
    template <typename T>                                              \
    TYPE(::hk::Span<const T>)->DESIRED;                                \
    template <typename T>                                              \
    TYPE(::hk::Span<T>)->DESIRED

    } // namespace detail

    template <typename T, typename Storage>
    using VecOperations = detail::VecOperations<T, Storage, false>;

    template <typename T, typename Storage>
    using VecOperationsWithBufferPointer = detail::VecOperations<T, Storage, true>;

    template <typename T, typename Storage, size ReserveSize = 16, typename Allocator = util::DefaultAllocator>
    struct VecOperationsOnHeap : VecOperationsWithBufferPointer<T, Storage> {
        using Super = VecOperationsWithBufferPointer<T, Storage>;

        using Super::clear;

        constexpr VecOperationsOnHeap(size numElements = 0) {
            set(allocate(numElements, ReserveSize));
            setCapacity(util::max(numElements, ReserveSize));
            util::construct(getData(), numElements);
        }

        constexpr ~VecOperationsOnHeap() {
            clear();
            free(*this, getCapacity());
        }

        constexpr VecOperationsOnHeap(const VecOperationsOnHeap& other) {
            setCopy(other);
        }

        constexpr VecOperationsOnHeap& operator=(const VecOperationsOnHeap& other) {
            setCopy(other);
            return *this;
        }

        constexpr VecOperationsOnHeap(VecOperationsOnHeap&& other) {
            setMove(other);
        }

        constexpr VecOperationsOnHeap& operator=(VecOperationsOnHeap&& other) {
            setMove(other);
            return *this;
        }

        constexpr VecOperationsOnHeap(const T* data, size size) {
            set(allocate(size, size));
            util::constructCopy(getData(), data, size);
        }

        constexpr VecOperationsOnHeap(T* data, size size)
            : VecOperationsOnHeap(data, size) { }

        template <size N>
        constexpr VecOperationsOnHeap(const T (&data)[N])
            : VecOperationsOnHeap(data, N) { }

        template <size N>
        constexpr VecOperationsOnHeap(T (&data)[N])
            : VecOperationsOnHeap(data, N) { }

        constexpr VecOperationsOnHeap(Span<T> data)
            : VecOperationsOnHeap(data.data(), data.size()) { }

        constexpr VecOperationsOnHeap(Span<const T> data)
            : VecOperationsOnHeap(data.data(), data.size()) { }

        constexpr VecOperationsOnHeap(std::span<T> data)
            : VecOperationsOnHeap(data.data(), data.size()) { }

        constexpr VecOperationsOnHeap(std::span<const T> data)
            : VecOperationsOnHeap(data.data(), data.size()) { }

        constexpr VecOperationsOnHeap(std::initializer_list<T> data)
            : VecOperationsOnHeap(data.begin(), data.size()) { }

        constexpr T& set(size index, const T& value) { return set(index, value); }
        constexpr T& set(size index, T&& value) { return set(index, forward<T>(value)); }

        constexpr void setCopy(Span<const T> data, size capacity = -1) {
            if (capacity == -1)
                capacity = data.size();

            clear();
            set(allocate(data.size(), capacity));
            setCapacity(capacity);
            util::constructCopy(getData(), data.data(), data.size());
        }

        constexpr void setCopy(const VecOperationsOnHeap& other) {
            setCopy(other, other.getCapacity());
        }

        constexpr void setMove(Span<T> data, size capacity = -1) {
            if (capacity == -1)
                capacity = data.size();

            clear();
            set(allocate(data.size(), capacity));
            setCapacity(capacity);
            util::constructMove(getData(), data.data(), data.size());
        }

        constexpr void setMove(VecOperationsOnHeap&& other) {
            setMove(other, other.getCapacity());
            other.clear();
        }

        constexpr void reserve(size capacity) {
            if (getCapacity() >= capacity)
                return;

            Span<T> newData = allocate(getSize(), capacity);
            util::constructMove(newData.data(), getData(), getSize());
            util::destroy(getData(), getSize());
            free(*this, getCapacity());
            set(newData);
            setCapacity(capacity);
        }

        constexpr void extend(size newSize, const T& extendValue = T()) {
            reserve(newSize);
            Super::extend(newSize, extendValue);
        }

        constexpr void resize(size newSize, const T& extendValue = T()) {
            reserve(newSize);
            Super::resize(newSize, extendValue);
        }

        constexpr T& insert(const T& value, size index = 0) {
            reserve(getSize() + 1);
            return Super::insert(value, index);
        }

        constexpr T& insert(T&& value, size index = 0) {
            reserve(getSize() + 1);
            return Super::insert(forward<T>(value), index);
        }

        constexpr T& add(const T& valueAt) {
            reserve(getSize() + 1);
            return Super::add(valueAt);
        }

        constexpr T& add(T&& valueAt) {
            reserve(getSize() + 1);
            return Super::add(forward<T>(valueAt));
        }

        constexpr T& pushBack(const T& value) { return add(value); }
        constexpr T& pushBack(T&& value) { return add(forward<T>(value)); }

        constexpr T& pushFront(const T& value) { return insert(value, 0); }
        constexpr T& pushFront(T&& value) { return insert(forward<T>(value), 0); }

    protected:
        using Super::getCapacity;
        using Super::getData;
        using Super::getSize;
        using Super::setCapacity;

    private:
        using Super::add;
        using Super::extend;
        using Super::insert;
        using Super::resize;

        using Storage::allocate;
        using Storage::free;

        using Super::setCopy;
        using Super::setMove;

        using Super::set;
    };

} // namespace hk
