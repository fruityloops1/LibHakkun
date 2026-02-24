#pragma once

#include "hk/Result.h"
#include "hk/diag/diag.h"
#include "hk/types.h"
#include "hk/util/FixedVec.h"
#include <type_traits>
#ifdef HK_ADDON_Sead
#include <sead/prim/seadSafeString.h>
#endif

namespace hk::util {

    /**
     * @brief Non-owning read/write buffer stream.
     *
     * @tparam BufferType Buffer type. If pointer to a const type, stream will be read-only
     */
    template <typename BufferType>
    class Stream {
        ptr mBuffer = 0;
        size mSize = -1;
        size mCursor = 0;

        bool hasSize() const {
            return mSize != -1;
        }

        constexpr static bool cIsReadOnly = std::is_const_v<BufferType>;

        void checkReadOnly() const {
            HK_ABORT_UNLESS(cIsReadOnly == false, "hk::util::Stream: stream is read only", 0);
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

        hk::Result tryWrite(const void* data, size dataSize) {
            checkReadOnly();

            if (hasSize() && mCursor + dataSize > mSize)
                return hk::ResultOutOfRange();

            memcpy(cast<void*>(mBuffer + mCursor), data, dataSize);
            mCursor += dataSize;

            return hk::ResultSuccess();
        }

        void write(const void* data, size dataSize) {
            checkReadOnly();

            if (hasSize())
                HK_ABORT_UNLESS(mCursor + dataSize <= mSize, "hk::util::Stream::write: out of range (%zu/%zu)", mCursor + dataSize, mSize);

            memcpy(cast<void*>(mBuffer + mCursor), data, dataSize);
            mCursor += dataSize;
        }

        template <typename T>
        hk::Result tryWrite(const T& value) {
            return tryWrite(&value, sizeof(T));
        }

        template <typename T>
        void write(const T& value) {
            write(&value, sizeof(T));
        }

        void writeString(const char* value, size length) {
            write(length);
            write(static_cast<const void*>(value), length);
        }

        void writeString(const char* value) {
            writeString(value, __builtin_strlen(value));
        }

        hk::Result tryWriteString(const char* value, size length) {
            HK_TRY(tryWrite(length));
            return tryWrite(static_cast<const void*>(value), length);
        }

        hk::Result tryWriteString(const char* value) {
            return tryWriteString(value, __builtin_strlen(value));
        }

        template <typename CT, typename T>
        hk::Result tryWriteIterator(const T& container) {
            for (CT ct : container)
                HK_TRY(tryWrite(ct));
            return hk::ResultSuccess();
        }

        template <typename CT, typename T>
        void writeIterator(const T& container) {
            for (CT ct : container)
                write(ct);
        }

        void align(size alignment, u8 c = 0) {
            size alignedCursor = alignUp(mCursor, alignment);
            size toZero = alignedCursor - mCursor;

            for (size i = 0; i < toZero; i++)
                write(c);
        }

        hk::Result tryRead(void* out, size dataSize) {
            if (hasSize() && mCursor + dataSize > mSize)
                return hk::ResultOutOfRange();

            memcpy(out, cast<const void*>(mBuffer + mCursor), dataSize);
            mCursor += dataSize;
            return hk::ResultSuccess();
        }

        void read(void* out, size dataSize) {
            if (hasSize())
                HK_ABORT_UNLESS(mCursor + dataSize <= mSize, "hk::util::Stream::write: out of range (%zu/%zu)", mCursor + dataSize, mSize);

            memcpy(out, cast<const void*>(mBuffer + mCursor), dataSize);
            mCursor += dataSize;
        }

        template <typename T>
        hk::ValueOrResult<T> read() {
            alignas(alignof(T)) u8 storage[sizeof(T)];
            HK_TRY(tryRead(storage, sizeof(T)));
            return { move(*cast<T*>(storage)) };
        }

        template <typename T, size Capacity>
        hk::ValueOrResult<FixedVec<T, Capacity>> readArray(size amount) {
            HK_UNLESS(amount <= Capacity, hk::ResultOutOfRange());
            FixedVec<T, Capacity> arr;

            for (size i = 0; i < amount; i++)
                arr.add(HK_TRY(read<T>()));

            return move(arr);
        }

        hk::Result readString(char* outBuffer, size maxLen = -1) {
            size length = HK_TRY(read<size>());
            HK_UNLESS(length <= maxLen, hk::ResultOutOfRange());
            return tryRead(static_cast<void*>(outBuffer), length);
        }

#ifdef HK_ADDON_Sead
        void writeString(const sead::SafeString& str) {
            writeString(str.cstr(), size(str.calcLength()));
        }

        hk::Result tryWriteString(const sead::SafeString& str) {
            return tryWriteString(str.cstr(), size(str.calcLength()));
        }

        hk::Result readString(sead::BufferedSafeString* out) {
            return readString(out->getBuffer(), size(out->getBufferSize()));
        }

        template <size Capacity>
        hk::ValueOrResult<sead::FixedSafeString<Capacity>> readString() {
            sead::FixedSafeString<Capacity> buffer;
            HK_TRY(readString(&buffer));
            return buffer;
        }
#endif

        static constexpr bool isReadOnly() { return cIsReadOnly; }
    };

} // namespace hk::util
