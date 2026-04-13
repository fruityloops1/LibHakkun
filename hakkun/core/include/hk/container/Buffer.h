#pragma once

#include "hk/Result.h"
#include "hk/container/Span.h"
#include "hk/types.h"
#include "hk/util/Allocator.h"

namespace hk {

    template <typename T, typename Allocator = util::DefaultAllocator>
    struct Buffer : public Span<T> {
        constexpr Buffer() = default;

        constexpr Buffer(const Buffer& other) {
            setCopy(other);
        }

        constexpr Buffer(Buffer&& old) {
            Span<T>::set(old.data(), old.size());
            old.Span<T>::set(nullptr, 0);
        }

        constexpr Buffer(const Span<T>& buffer) {
            setCopy(buffer);
        }

        constexpr Buffer(const Span<const T>& buffer) {
            setCopy(buffer);
        }

        constexpr ~Buffer() {
            freeBuffer();
        }

        constexpr Result allocBuffer(size size, const T& value = T(), ::size align = alignof(T)) {
            T* data;
            if consteval {
                data = std::allocator<T>().allocate(size);
            } else {
                data = cast<T*>(Allocator::allocate(size * sizeof(T), align));
            }
            HK_UNLESS(data != nullptr, ResultOutOfResource());

            set(data, size);

            util::construct(getData(), getSize(), value);

            return ResultSuccess();
        }

        constexpr Result freeBuffer() {
            if (getData() != nullptr) {
                util::destroy(getData(), getSize());

                if consteval {
                    std::allocator<T>().deallocate(getData(), getSize());
                } else {
                    Allocator::free(getData());
                }
                set(nullptr, 0);
                return ResultSuccess();
            }
            return ResultNoValue();
        }

        constexpr Result setCopy(const Span<const T>& buffer) {
            return setCopy(buffer.data(), buffer.size());
        }

        constexpr Result setCopy(const T* ptr, size size) {
            T* newPtr;
            if consteval {
                newPtr = std::allocator<T>().allocate(size);
            } else {
                newPtr = cast<T*>(Allocator::allocate(size * sizeof(T), alignof(T)));
            }
            HK_UNLESS(newPtr != nullptr, ResultOutOfResource());
            for (::size i = 0; i < size; i++)
                newPtr[i] = ptr[i];

            set(newPtr, size);
            return ResultSuccess();
        }

    protected:
        using Span<T>::getData;
        using Span<T>::getSize;

    private:
        using Span<T>::set;
    };

} // namespace hk
