#include "hk/ValueOrResult.h"
#include "hk/sf/hipc.h"
#include "hk/svc/api.h"
#include "hk/types.h"
#include "hk/util/FixedVec.h"

namespace hk::tipc {
    using Buffer = sf::hipc::Buffer;
    using BufferMode = sf::hipc::BufferMode;
    class Request {
        struct {
            bool mPrintRequest : 1 = false;
            bool mAbortAfterRequest : 1 = false;
            bool mPrintResponse : 1 = false;
            bool mSendPid : 1 = false;
        };
        u32 mCommandId = 0;
        util::FixedVec<u32, 8> mObjects;
        util::FixedVec<Handle, 8> mHipcCopyHandles;
        util::FixedVec<Buffer, 8> mHipcSendBuffers;
        util::FixedVec<Buffer, 8> mHipcReceiveBuffers;
        util::FixedVec<Buffer, 8> mHipcExchangeBuffers;
        util::Span<const u8> mData = {};

        void writeToTls();

    public:
        Request(u32 command)
            : mCommandId(command) { }
        template <typename T>
        Request(u32 command, const T* data)
            : mCommandId(command)
            , mData(cast<const u8*>(data), sizeof(T)) { }
        template <typename T>
        Request(u32 command, const std::span<T> data)
            : mCommandId(command)
            , mData(cast<const u8*>(data.data()), data.size_bytes()) { }

        constexpr void enableDebug(bool before = true, bool after = true) {
            mPrintRequest = before;
            mPrintResponse = after;
        }

        constexpr void debugAbortAfterRequest() {
            mPrintRequest = true;
            mAbortAfterRequest = true;
        }

        void setSendPid() {
            mSendPid = true;
        }

        void addCopyHandle(Handle handle) {
            mHipcCopyHandles.add(handle);
        }

        void addInBuffer(const void* data, u64 size, BufferMode mode = BufferMode::Normal) {
            mHipcSendBuffers.add(Buffer(
                mode,
                u64(data),
                size));
        }

        template <typename T>
        void addInBuffer(std::span<const T> span, BufferMode mode = BufferMode::Normal) {
            addInBuffer(span.data(), span.size_bytes(), mode);
        }

        void addOutBuffer(void* data, u64 size, BufferMode mode = BufferMode::Normal) {
            mHipcReceiveBuffers.add(Buffer(
                mode,
                u64(data),
                size));
        }

        template <typename T>
        void addOutBuffer(std::span<T> span, BufferMode mode = BufferMode::Normal) {
            addOutBuffer(span.data(), span.size_bytes(), mode);
        }

        void addExchangeBuffer(void* data, u64 size, BufferMode mode = BufferMode::Normal) {
            mHipcExchangeBuffers.add(Buffer(
                mode,
                u64(data),
                size));
        }

        template <typename T>
        void addExchangeBuffer(std::span<T> span, BufferMode mode = BufferMode::Normal) {
            addExchangeBuffer(span.data(), span.size_bytes(), mode);
        }

        ValueOrResult<class Response> invoke(Handle handle);
    };

    class Response {
        friend class Request;
        util::FixedVec<Handle, 8> mHipcHandles;
        util::Span<u8> mData;

        static ValueOrResult<Response> readFromTls(bool printResponse);

    public:
        Handle takeNextHandle() {
            return mHipcHandles.remove(0);
        }

        util::Span<u8> getData() const { return mData; }
    };

    inline ValueOrResult<class Response> Request::invoke(Handle handle) {
        writeToTls();
        HK_TRY(svc::SendSyncRequest(handle));
        return Response::readFromTls(mPrintResponse);
    }

}
