#pragma once

#include "hk/Result.h"
#include "hk/ValueOrResult.h"
#include "hk/container/FixedVec.h"
#include "hk/diag/diag.h"
#include "hk/prim/traits/Function.h"
#include "hk/sf/cmif.h"
#include "hk/sf/hipc.h"
#include "hk/svc/api.h"
#include "hk/svc/types.h"
#include "hk/types.h"
#include "hk/util/Stream.h"
#include <cstddef>
#include <cstring>
#include <optional>
#include <span>

/*
Untested:
- recv statics
*/

namespace hk::sf {

    class Request;
    struct Response;

    constexpr size cTlsBufferSize = sizeof(svc::ThreadLocalRegion::ipcMessageBuffer);

    class Service {
        friend class Request;
        friend struct Response;
        Handle mSession;
        std::optional<u32> mObject;
        // Whether the session handle is owned by the current service.
        // Domain subservices don't own their own handles.
        bool mOwnedHandle;
        // uninitialized when u16 max
        u16 mPointerBufferSize = std::numeric_limits<u16>::max();

        template <typename ResponseExtractor>
        inline ValueOrResult<typename util::FunctionTraits<ResponseExtractor>::ReturnType> invoke(cmif::MessageTag tag, Request&& request, ResponseExtractor extractor);
        inline Result invoke(cmif::MessageTag tag, Request&& request);
        template <typename ResponseExtractor>
        inline ValueOrResult<typename util::FunctionTraits<ResponseExtractor>::ReturnType> invokeControl(Request&& request, ResponseExtractor extractor) {
            return invoke(cmif::MessageTag::Control, std::forward<Request>(request), extractor);
        }

        static Service domainSubservice(Service* parent, u32 object) {
            return Service(parent->mSession, object, false, parent->mPointerBufferSize);
        }

        Result release();

        Service(Handle session, std::optional<u32> object, bool isRoot, u16 pointerBufferSize)
            : mSession(session)
            , mObject(object)
            , mOwnedHandle(isRoot)
            , mPointerBufferSize(pointerBufferSize) { }

    public:
        Service(Service&& old)
            : mSession(old.mSession)
            , mObject(old.mObject)
            , mOwnedHandle(old.mOwnedHandle)
            , mPointerBufferSize(old.mPointerBufferSize) {
            old.mSession = 0;
            old.mOwnedHandle = false;
            old.mObject = std::nullopt;
        }

        Service(Handle handle)
            : mSession(handle)
            , mObject(std::nullopt)
            , mOwnedHandle(true) { }

        static Service fromHandle(Handle session) {
            return Service(session, std::nullopt, false, std::numeric_limits<u16>::max());
        }

        ~Service() {
            if (mOwnedHandle) {
                svc::CloseHandle(mSession);
            } else if (mSession != 0) {
                HK_ABORT_UNLESS_R(release());
            }
        }

        Handle handle() const {
            return mSession;
        }

        std::optional<u32> objectId() const {
            return mObject;
        }

        bool isDomain() const {
            return mObject.has_value();
        }

        bool isDomainSubservice() const {
            return !mOwnedHandle;
        }

        u16 pointerBufferSize();

        // A service might have many interfaces, and the kernel has a limit on active session handles for a process.
        // Domain objects allow a session to multiplex accesses to interfaces, saving on session handles.
        Result convertToDomain();

        template <typename ResponseExtractor>
        inline ValueOrResult<typename util::FunctionTraits<ResponseExtractor>::ReturnType> invokeRequest(Request&& request, ResponseExtractor extractor) {
            return invoke(cmif::MessageTag::Request, std::forward<Request>(request), extractor);
        }
        inline Result invokeRequest(Request&& request) {
            return invoke(cmif::MessageTag::Request, std::forward<Request>(request));
        }
    };

    class Request {
        struct {
            bool mPrintRequest : 1 = false;
            bool mAbortAfterRequest : 1 = false;
            bool mPrintResponse : 1 = false;
            bool mSendPid : 1 = false;
        };
        u8 mHipcStaticIdx = 0;
        u16 mServerPointerSize = 0;
        u32 mCommandId = 0;
        u32 mToken = 0;
        u32 mHardcodedDataSize = 0;
        cmif::DomainTag mDomainTag = cmif::DomainTag::Request;
        FixedVec<u32, 8> mObjects;
        FixedVec<Handle, 8> mHipcCopyHandles;
        FixedVec<Handle, 8> mHipcMoveHandles;
        FixedVec<hipc::Static, 8> mHipcSendStatics;
        FixedVec<hipc::Buffer, 8> mHipcSendBuffers;
        FixedVec<hipc::Buffer, 8> mHipcReceiveBuffers;
        FixedVec<hipc::Buffer, 8> mHipcExchangeBuffers;
        FixedVec<hipc::ReceiveStatic, 8> mHipcReceiveStatics;
        FixedVec<u16, 8> mHipcOutPointerSizes;
        Span<const u8> mData = { };

        friend class Service;
        // dedicated constructor for cmif::QueryPointerSize
        Request(std::nullptr_t, Service* service, u32 command)
            : mCommandId(command) { }

    public:
        Request(Service* service, u32 command)
            : mCommandId(command)
            , mServerPointerSize(service->pointerBufferSize()) { }
        template <typename T>
        Request(Service* service, u32 command, const T* data)
            : mCommandId(command)
            , mServerPointerSize(service->pointerBufferSize())
            , mData(cast<const u8*>(data), sizeof(T)) { }
        template <typename T>
        Request(Service* service, u32 command, const Span<T>& data)
            : mCommandId(command)
            , mServerPointerSize(service->pointerBufferSize())
            , mData(cast<const u8*>(data.data()), data.size_bytes()) { }

        constexpr void setDomainClose() {
            mDomainTag = cmif::DomainTag::Close;
        }

        constexpr void enableDebug(bool before = true, bool after = true) {
            mPrintRequest = before;
            mPrintResponse = after;
        }

        constexpr void debugAbortAfterRequest() {
            mPrintRequest = true;
            mAbortAfterRequest = true;
        }

        constexpr void setToken(u32 token) {
            mToken = token;
        }

        constexpr void forceDataSize(u32 byteCount) {
            mHardcodedDataSize = byteCount;
        }

        constexpr void setSendPid() {
            mSendPid = true;
        }

        constexpr void addCopyHandle(Handle handle) {
            mHipcCopyHandles.add(handle);
        }

        constexpr void addMoveHandle(Handle handle) {
            mHipcMoveHandles.add(handle);
        }

        void addInPointer(const void* data, u16 size, hipc::BufferMode mode = hipc::BufferMode::Normal) {
            mHipcSendStatics.add(hipc::Static(
                mHipcStaticIdx++,
                u64(data),
                size));
            mServerPointerSize -= size;
        }

        template <typename T>
        void addInPointer(Span<const T> span, hipc::BufferMode mode = hipc::BufferMode::Normal) {
            addInPointer(span.data(), span.size_bytes(), mode);
        }

        void addOutFixedSizePointer(void* data, u16 size) {
            mHipcReceiveStatics.add(hipc::ReceiveStatic(
                u64(data),
                size));
            mServerPointerSize -= size;
        }

        template <typename T>
        void addOutFixedSizePointer(Span<const T> span) {
            addOutFixedSizePointer(span.data(), span.size_bytes());
        }

        void addOutPointer(void* data, u64 size, hipc::BufferMode mode = hipc::BufferMode::Normal) {
            addOutFixedSizePointer(data, size);
            mHipcOutPointerSizes.add(size);
        }

        template <typename T>
        void addOutPointer(Span<T> span, hipc::BufferMode mode = hipc::BufferMode::Normal) {
            addOutPointer(span.data(), span.size_bytes(), mode);
        }

        void addInAutoselect(const void* data, u64 size, hipc::BufferMode mode = hipc::BufferMode::Normal) {
            if (mServerPointerSize > 0 && size <= mServerPointerSize) {
                addInPointer(data, size);
                mHipcSendBuffers.add(hipc::Buffer(mode, u64(nullptr), 0));
            } else {
                addInPointer(nullptr, 0);
                mHipcSendBuffers.add(hipc::Buffer(mode, u64(data), size));
            }
        }

        template <typename T>
        void addInAutoselect(Span<const T> span, hipc::BufferMode mode = hipc::BufferMode::Normal) {
            addInAutoselect(span.data(), span.size_bytes(), mode);
        }

        void addOutAutoselect(void* data, u64 size, hipc::BufferMode mode = hipc::BufferMode::Normal) {
            if (mServerPointerSize > 0 && size <= mServerPointerSize) {
                mHipcReceiveStatics.add(hipc::ReceiveStatic(u64(data), size));
                mHipcReceiveBuffers.add(hipc::Buffer(
                    mode,
                    u64(nullptr),
                    0));
            } else {
                mHipcReceiveStatics.add(hipc::ReceiveStatic());
                mHipcReceiveBuffers.add(hipc::Buffer(
                    mode,
                    u64(data),
                    size));
            }
        }

        template <typename T>
        void addOutAutoselect(Span<T> span, hipc::BufferMode mode = hipc::BufferMode::Normal) {
            addOutAutoselect(span.data(), span.size_bytes(), mode);
        }

        void addInMapAlias(const void* data, u64 size, hipc::BufferMode mode = hipc::BufferMode::Normal) {
            mHipcSendBuffers.add(hipc::Buffer(
                mode,
                u64(data),
                size));
        }

        template <typename T>
        void addInMapAlias(Span<const T> span, hipc::BufferMode mode = hipc::BufferMode::Normal) {
            addInMapAlias(span.data(), span.size_bytes(), mode);
        }

        void addOutMapAlias(void* data, u64 size, hipc::BufferMode mode = hipc::BufferMode::Normal) {
            mHipcReceiveBuffers.add(hipc::Buffer(
                mode,
                u64(data),
                size));
        }

        template <typename T>
        void addOutMapAlias(Span<T> span, hipc::BufferMode mode = hipc::BufferMode::Normal) {
            addOutMapAlias(span.data(), span.size_bytes(), mode);
        }

        void addInOutMapAlias(void* data, u64 size, hipc::BufferMode mode = hipc::BufferMode::Normal) {
            mHipcExchangeBuffers.add(hipc::Buffer(
                mode,
                u64(data),
                size));
        }

        template <typename T>
        void addInOutMapAlias(Span<T> span, hipc::BufferMode mode = hipc::BufferMode::Normal) {
            addInOutMapAlias(span.data(), span.size_bytes(), mode);
        }

    private:
        friend class Service;
        void writeToTls(Service* service, cmif::MessageTag tag) {
            std::memset(svc::getTLS()->ipcMessageBuffer, 0, cTlsBufferSize);
            util::Stream writer(svc::getTLS()->ipcMessageBuffer, cTlsBufferSize);
            bool hasSpecialHeader = mSendPid || !mHipcCopyHandles.empty() || !mHipcMoveHandles.empty();

            struct Sizes {
                u16 hipcDataSize;
                u16 cmifDataSize;
            } sizes = [this, service]() {
                u16 hipcDataSize = 16;
                u16 cmifDataSize = sizeof(cmif::InHeader) + mData.size_bytes();

                if (service->isDomain()) {
                    hipcDataSize += sizeof(cmif::DomainInHeader);
                    hipcDataSize += sizeof(u32) * mObjects.size();
                }

                hipcDataSize += sizeof(u16) * mHipcOutPointerSizes.size();
                hipcDataSize = alignUp(hipcDataSize, sizeof(u16));

                hipcDataSize += cmifDataSize;

                return Sizes {
                    .hipcDataSize = hipcDataSize,
                    .cmifDataSize = cmifDataSize
                };
            }();

            writer.write(hipc::Header {
                .tag = u16(tag),
                .sendStaticCount = u8(mHipcSendStatics.size()),
                .sendBufferCount = u8(mHipcSendBuffers.size()),
                .recvBufferCount = u8(mHipcReceiveBuffers.size()),
                .exchBufferCount = u8(mHipcExchangeBuffers.size()),
                .dataWords = u16(alignUp(sizes.hipcDataSize, 4) / 4),
                .recv_static_mode = 2u + u8(mHipcReceiveStatics.size()),
                .hasSpecialHeader = hasSpecialHeader,
            });

            if (hasSpecialHeader) {
                writer.write(hipc::SpecialHeader {
                    .sendPid = mSendPid,
                    .copyHandleCount = u8(mHipcCopyHandles.size()),
                    .moveHandleCount = u8(mHipcMoveHandles.size()),
                });

                if (mSendPid)
                    writer.write<u64>(0);

                writer.writeIterator<Handle>(mHipcCopyHandles);
                writer.writeIterator<Handle>(mHipcMoveHandles);
            }

            writer.writeIterator<hipc::Static>(mHipcSendStatics);
            writer.writeIterator<hipc::Buffer>(mHipcSendBuffers);
            writer.writeIterator<hipc::Buffer>(mHipcReceiveBuffers);
            writer.writeIterator<hipc::Buffer>(mHipcExchangeBuffers);
            writer.seek(alignUp(writer.tell(), 16));

            if (service->isDomain())
                writer.write(cmif::DomainInHeader {
                    .tag = mDomainTag,
                    .objectCount = u8(mObjects.size()),
                    .dataSize = sizes.cmifDataSize,
                    .objectId = *service->mObject,
                    .token = mToken,
                });

            writer.write(cmif::InHeader {
                .magic = cmif::cInHeaderMagic,
                .version = 0,
                .command = mCommandId,
                .token = service->mObject ? 0 : mToken,
            });

            writer.writeIterator<u8>(mData);
            writer.seek(alignUp(writer.tell(), 4));

            if (service->isDomain())
                writer.writeIterator<u32>(mObjects);

            writer.seek(alignUp(writer.tell(), 16));
            writer.writeIterator<u16>(mHipcOutPointerSizes);

#if !defined(HK_RELEASE)
            if (mPrintRequest) {
                u8 buf[256] = { };
                memcpy(buf, svc::getTLS()->ipcMessageBuffer, cTlsBufferSize);
                diag::logLine("");
                for (int i = 0; i < writer.tell(); i += 16)
                    diag::logLine("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                        buf[i + 0], buf[i + 1], buf[i + 2], buf[i + 3],
                        buf[i + 4], buf[i + 5], buf[i + 6], buf[i + 7],
                        buf[i + 8], buf[i + 9], buf[i + 10], buf[i + 11],
                        buf[i + 12], buf[i + 13], buf[i + 14], buf[i + 15]);
                memcpy(svc::getTLS()->ipcMessageBuffer, buf, cTlsBufferSize);
            }

            if (mAbortAfterRequest)
                HK_ABORT("Aborted after response (debug)");
#endif
        }
    };

    struct Response {
        Result result;
        FixedVec<u32, 8> objects;
        FixedVec<Handle, 8> hipcCopyHandles;
        FixedVec<Handle, 8> hipcMoveHandles;
        FixedVec<hipc::Static, 8> hipcSendStatics;
        std::optional<u64> pid;
        Span<u8> data;

        Service nextSubservice(Service* service) {
            if (service->isDomain())
                return Service::domainSubservice(service, objects.remove(0));
            else
                return Service::fromHandle(hipcMoveHandles.remove(0));
        }
        Handle nextCopyHandle() {
            return hipcCopyHandles.remove(0);
        }
        Handle nextMoveHandle() {
            return hipcMoveHandles.remove(0);
        }

    private:
        friend class Service;
        static Response readFromTls(Service* service, u32 hardcodedDataBytes) {
            util::Stream reader(svc::getTLS()->ipcMessageBuffer, cTlsBufferSize);

            auto header = HK_UNWRAP(reader.read<hipc::Header>());

            Response response;
            if (header.hasSpecialHeader) {
                auto specialHeader = HK_UNWRAP(reader.read<hipc::SpecialHeader>());

                if (specialHeader.sendPid)
                    response.pid = HK_UNWRAP(reader.read<u64>());

                for (u8 i = 0; i < specialHeader.copyHandleCount; i++)
                    response.hipcCopyHandles.add(HK_UNWRAP(reader.read<Handle>()));
                for (u8 i = 0; i < specialHeader.moveHandleCount; i++)
                    response.hipcMoveHandles.add(HK_UNWRAP(reader.read<Handle>()));
            }

            for (u8 i = 0; i < header.sendStaticCount; i++)
                response.hipcSendStatics.add(HK_UNWRAP(reader.read<hipc::Static>()));
            reader.seek(alignUp(reader.tell(), 16));

            size dataWordsLeft = header.dataWords - 4;

            u32 objectCount = 0;
            if (service->isDomain()) {
                auto domainOut = HK_UNWRAP(reader.read<cmif::DomainOutHeader>());
                dataWordsLeft -= sizeof(cmif::DomainOutHeader) / 4;
                dataWordsLeft -= domainOut.objectCount;
                objectCount = domainOut.objectCount;
            }

            auto outHeader = HK_UNWRAP(reader.read<cmif::OutHeader>());
            dataWordsLeft -= sizeof(cmif::OutHeader) / 4;
            response.result = outHeader.result;
            response.data = Span(svc::getTLS()->ipcMessageBuffer + reader.tell(), hardcodedDataBytes ?: dataWordsLeft * 4);
            reader.seek(reader.tell() + response.data.size_bytes());
            for (u8 i = 0; i < objectCount; i++)
                response.objects.add(HK_UNWRAP(reader.read<u32>()));

            // todo: recv statics

            return response;
        }
    };

    inline void printResponse(bool printResponse, u32 wordCount) {
#if !defined(HK_RELEASE)
        if (printResponse) {
            util::Stream reader(svc::getTLS()->ipcMessageBuffer, cTlsBufferSize);
            u8 buf[256] = { };
            memcpy(buf, svc::getTLS()->ipcMessageBuffer, cTlsBufferSize);
            diag::logLine("");
            for (int i = 0; i < wordCount * 4; i += 16)
                diag::logLine("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                    buf[i + 0], buf[i + 1], buf[i + 2], buf[i + 3],
                    buf[i + 4], buf[i + 5], buf[i + 6], buf[i + 7],
                    buf[i + 8], buf[i + 9], buf[i + 10], buf[i + 11],
                    buf[i + 12], buf[i + 13], buf[i + 14], buf[i + 15]);
            memcpy(svc::getTLS()->ipcMessageBuffer, buf, cTlsBufferSize);
        }
#endif
    }

    template <typename ResponseExtractor>
    inline ValueOrResult<typename util::FunctionTraits<ResponseExtractor>::ReturnType> Service::invoke(cmif::MessageTag tag, Request&& request, ResponseExtractor extractor) {
        using Return = typename util::FunctionTraits<ResponseExtractor>::ReturnType;

        request.writeToTls(this, tag);
        Result result = svc::SendSyncRequest(mSession);
        printResponse(request.mPrintResponse, 32);
        HK_TRY(result);
        auto response = Response::readFromTls(this, request.mHardcodedDataSize);
        HK_TRY(response.result);

        if constexpr (std::is_same_v<Return, void>)
            return ResultSuccess();
        else
            return extractor(response);
    }

    inline Result Service::invoke(cmif::MessageTag tag, Request&& request) {
        request.writeToTls(this, tag);
        Result result = svc::SendSyncRequest(mSession);
        printResponse(request.mPrintResponse, 32);
        HK_TRY(result);
        auto response = Response::readFromTls(this, request.mHardcodedDataSize);
        return response.result;
    }

} // namespace hk::sf
