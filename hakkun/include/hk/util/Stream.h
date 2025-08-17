#pragma once

#include "hk/diag/diag.h"
#include "hk/types.h"
#include "hk/util/FixedCapacityArray.h"
#include <type_traits>

namespace hk::util {

    template <typename BufferType>
    class Stream {
        ptr mBuffer = 0;
        size mSize = -1;
        size mCursor = 0;

        bool hasSize() const {
            return mSize != -1;
        }

        constexpr static bool cIsReadOnly = std::is_const_v<BufferType>;

        void checkReadonly() const {
            HK_ABORT_UNLESS(cIsReadOnly == false, "hk::util::Stream: stream is readonly", 0);
        }

    public:
        Stream(BufferType* buffer, size maxSize = -1)
            : mBuffer(ptr(buffer))
            , mSize(maxSize) { }

        void seek(size cursor) {
            if (hasSize())
                HK_ABORT_UNLESS(cursor <= mSize, "hk::util::Stream::seek: out of range (%zu/%zu)", cursor, mSize);

            mCursor = cursor;
        }

        size tell() const { return mCursor; }

        bool tryWrite(const void* data, size dataSize) {
            checkReadonly();

            if (hasSize() && mCursor + dataSize > mSize)
                return false;

            memcpy(cast<void*>(mBuffer + mCursor), data, dataSize);
            mCursor += dataSize;

            return true;
        }

        void write(const void* data, size dataSize) {
            checkReadonly();

            if (hasSize())
                HK_ABORT_UNLESS(mCursor + dataSize <= mSize, "hk::util::Stream::write: out of range (%zu/%zu)", mCursor + dataSize, mSize);

            memcpy(cast<void*>(mBuffer + mCursor), data, dataSize);
            mCursor += dataSize;
        }

        template <typename T>
        void write(const T& value) {
            write(&value, sizeof(T));
        }

        template <typename T>
        bool tryWrite(const T& value) {
            return tryWrite(&value, sizeof(T));
        }

        template <typename CT, typename T>
        void writeIterator(const T& container) {
            for (CT ct : container)
                write(ct);
        }

        template <typename CT, typename T>
        bool tryWriteIterator(const T& container) {
            for (CT ct : container)
                if (tryWrite(ct) == false)
                    return false;
            return true;
        }

        void align(size alignment, u8 c = 0) {
            size alignedCursor = alignUp(mCursor, alignment);
            size toZero = alignedCursor - mCursor;

            for (size i = 0; i < toZero; i++)
                write(c);
        }

        bool tryRead(void* out, size dataSize) {
            if (hasSize() && mCursor + dataSize > mSize)
                return false;

            memcpy(out, cast<const void*>(mBuffer + dataSize), dataSize);
            return true;
        }

        void read(void* out, size dataSize) {
            if (hasSize())
                HK_ABORT_UNLESS(mCursor + dataSize <= mSize, "hk::util::Stream::write: out of range (%zu/%zu)", mCursor + dataSize, mSize);

            memcpy(out, cast<const void*>(mBuffer + dataSize), dataSize);
        }

        template <typename T>
        T read() {
            T value;
            read(&value, sizeof(T));
        }

        template <typename T>
        bool tryRead(T* out) {
            return tryRead(out, sizeof(T));
        }

        template <typename T, size Capacity>
        FixedCapacityArray<T, Capacity> readArray(size amount) {
            HK_ABORT_UNLESS(amount <= Capacity, "hk::util::Stream::readArray: size exceeds capacity (%zu/%zu)", amount, Capacity);
            FixedCapacityArray<T, Capacity> arr;

            for (size i = 0; i < amount; i++)
                arr.add(read<T>());

            return arr;
        }

        static constexpr bool isReadOnly() { return cIsReadOnly; }
    };

} // namespace hk::util
