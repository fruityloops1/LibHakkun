#pragma once

#include "hk/diag/diag.h"
#include "hk/types.h"

namespace hk::util {

    class StreamWriter {
        ptr mBuffer = 0;
        size mSize = 0;
        size mCursor = 0;

        bool hasSize() const { return mSize != -1; }

    public:
        StreamWriter(void* buffer, size maxSize = -1)
            : mBuffer(ptr(buffer))
            , mSize(maxSize) { }

        void seek(size cursor) {
            if (hasSize())
                HK_ABORT_UNLESS(cursor <= mSize, "hk::util::StreamWriter::seek: out of range (%zu/%zu)", cursor, mSize);

            mCursor = cursor;
        }

        size tell() const { return mSize; }

        bool tryWrite(void* data, size dataSize) {
            if (mCursor + dataSize > mSize)
                return false;

            memcpy(cast<void*>(mBuffer + mCursor), data, dataSize);
            mCursor += dataSize;

            return true;
        }

        void write(void* data, size dataSize) {
            HK_ABORT_UNLESS(mCursor + dataSize <= mSize, "hk::util::StreamWriter::write: out of range (%zu/%zu)", mCursor + dataSize, mSize);

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
    };

} // namespace hk::util
