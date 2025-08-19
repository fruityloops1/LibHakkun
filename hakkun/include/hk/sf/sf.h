#pragma once

#include "hk/Result.h"
#include "hk/ValueOrResult.h"
#include "hk/diag/diag.h"
#include "hk/sf/cmif.h"
#include "hk/sf/hipc.h"
#include "hk/svc/api.h"
#include "hk/svc/types.h"
#include "hk/types.h"
#include "hk/util/FixedCapacityArray.h"
#include "hk/util/Lambda.h"
#include "hk/util/Stream.h"
#include <cstring>
#include <optional>
#include <span>

/*
Untested:
- buffers
- statics
- recv statics
- domains
    - subservices
*/

namespace hk::sf {
    class Request;
    struct Response;

    class Service {
        friend class Request;
        friend struct Response;
        Handle mSession;
        std::optional<u32> mObject;
        // Whether the session handle is owned by the current service.
        // Domain subservices don't own their own handles.
        bool ownedHandle;

        template <typename ResponseHandler>
        inline ValueOrResult<typename util::FunctionTraits<ResponseHandler>::ReturnType> invoke(cmif::MessageTag tag, Request&& request, ResponseHandler handler);
        inline Result invoke(cmif::MessageTag tag, Request&& request);
        template <typename ResponseHandler>
        inline ValueOrResult<typename util::FunctionTraits<ResponseHandler>::ReturnType> invokeControl(Request&& request, ResponseHandler handler) {
            return invoke(cmif::MessageTag::Control, std::forward<Request>(request), handler);
        }

        static Service domainSubservice(Service* parent, u32 object) {
            return Service(parent->mSession, object, false);
        }

        Service(Handle session, std::optional<u32> object, bool isRoot)
            : mSession(session)
            , mObject(object)
            , ownedHandle(isRoot) { }

    public:
        Service(Service&& old)
            : mSession(old.mSession)
            , ownedHandle(old.ownedHandle) {
            old.ownedHandle = false;
        }
        Service(Handle handle)
            : mSession(handle)
            , mObject()
            , ownedHandle(true) { }
        static Service fromHandle(Handle session) {
            return Service(session, std::nullopt, false);
        }

        ~Service() {
            if (ownedHandle) {
                svc::CloseHandle(mSession);
                HK_ABORT("closed when valid...", 0);
            }
        }

        Handle handle() {
            return mSession;
        }

        bool isDomain() {
            return mObject.has_value();
        }

        bool isDomainSubservice() {
            return !ownedHandle;
        }

        // A service might have many interfaces, and the kernel only has so many session handles,
        // so the client may choose to convert its active object handle to a domain object.
        // Domain objects allow a session to multiplex accesses to interfaces, saving on session handles.
        Result convertToDomain();

        template <typename ResponseHandler>
        inline ValueOrResult<typename util::FunctionTraits<ResponseHandler>::ReturnType> invokeRequest(Request&& request, ResponseHandler handler) {
            return invoke(cmif::MessageTag::Request, std::forward<Request>(request), handler);
        }
        inline Result invokeRequest(Request&& request) {
            return invoke(cmif::MessageTag::Request, std::forward<Request>(request));
        }
    };

    class Request {
        bool mPrintRequest = false;
        bool mPrintResponse = false;
        bool mSendPid = false;
        u8 mHipcStaticIdx = 0;
        u32 mCommandId = 0;
        u32 mToken = 0;
        util::FixedCapacityArray<u32, 8> mObjects;
        util::FixedCapacityArray<Handle, 8> mHipcCopyHandles;
        util::FixedCapacityArray<Handle, 8> mHipcMoveHandles;
        util::FixedCapacityArray<hipc::Static, 8> mHipcSendStatics;
        util::FixedCapacityArray<hipc::Buffer, 8> mHipcSendBuffers;
        util::FixedCapacityArray<hipc::Buffer, 8> mHipcReceiveBuffers;
        util::FixedCapacityArray<hipc::Buffer, 8> mHipcExchangeBuffers;
        util::FixedCapacityArray<hipc::ReceiveStatic, 8> mHipcReceiveStatics;
        std::span<const u8> mData = {};

    public:
        Request(u32 command)
            : mCommandId(command) {
        }
        template <typename T>
        Request(u32 command, const T* data, size size)
            : mCommandId(command)
            , mData(cast<const u8*>(data), sizeof(T) * size) {
        }

        void enableDebug(bool before, bool after) {
            mPrintRequest = before;
            mPrintResponse = after;
        }

        void setToken(u32 token) {
            this->mToken = token;
        }

        void setSendPid() {
            mSendPid = true;
        }

        void addCopyHandle(Handle handle) {
            mHipcCopyHandles.add(handle);
        }

        void addMoveHandle(Handle handle) {
            mHipcMoveHandles.add(handle);
        }

        void addInAutoselect(hipc::BufferMode mode, void* data, u64 size) {
            mHipcSendStatics.add(hipc::Static(
                mHipcStaticIdx++,
                0,
                0));
            mHipcSendBuffers.add(hipc::Buffer(
                mode,
                u64(data),
                u64(size)));
        }

        void addOutAutoselect(hipc::BufferMode mode, void* data, u64 size) {
            mHipcReceiveStatics.add(hipc::ReceiveStatic());
            mHipcReceiveBuffers.add(hipc::Buffer(
                mode,
                u64(data),
                size));
        }

        void addInPointer(hipc::BufferMode mode, void* data, u16 size) {
            mHipcSendStatics.add(hipc::Static(
                mHipcStaticIdx++,
                u64(data),
                size));
        }

        void addOutPointer(hipc::BufferMode mode, void* data, u64 size) {
            mHipcReceiveStatics.add(hipc::ReceiveStatic());
            mHipcReceiveBuffers.add(hipc::Buffer(
                mode,
                u64(data),
                size));
        }

        void addOutFixedSizePointer(hipc::BufferMode mode, void* data, u16 size) {
            mHipcReceiveStatics.add(hipc::ReceiveStatic(
                u64(data),
                size));
        }

        void addInMapAlias(hipc::BufferMode mode, void* data, u64 size) {
            mHipcSendBuffers.add(hipc::Buffer(
                mode,
                u64(data),
                size));
        }

        void addOutMapAlias(hipc::BufferMode mode, void* data, u64 size) {
            mHipcReceiveBuffers.add(hipc::Buffer(
                mode,
                u64(data),
                size));
        }

        void addInOutMapAlias(hipc::BufferMode mode, void* data, u64 size) {
            mHipcExchangeBuffers.add(hipc::Buffer(
                mode,
                u64(data),
                size));
        }

    private:
        friend class Service;
        void writeToTls(Service* service, cmif::MessageTag tag) {
            std::memset(svc::getTLS()->ipcMessageBuffer, 0, 256);
            util::Stream writer(svc::getTLS()->ipcMessageBuffer, sizeof(svc::ThreadLocalRegion::ipcMessageBuffer));
            bool hasSpecialHeader = mSendPid || !mHipcCopyHandles.empty() || !mHipcMoveHandles.empty();

            struct Sizes {
                u16 hipcDataSize;
                u16 cmifDataSize;
            };

            Sizes sizes = [this, service]() {
                u16 hipcDataSize = 16;
                u16 cmifDataSize = sizeof(cmif::InHeader) + mData.size_bytes();

                if (!service->ownedHandle) {
                    hipcDataSize += sizeof(cmif::DomainInHeader);
                    hipcDataSize += sizeof(u32) * mObjects.size();
                }

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
                .hasSpecialHeader = hasSpecialHeader,
            });

            if (hasSpecialHeader) {
                writer.write(hipc::SpecialHeader {
                    .sendPid = mSendPid,
                    .copyHandleCount = u8(mHipcCopyHandles.size()),
                    .moveHandleCount = u8(mHipcCopyHandles.size()),
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
                    .tag = cmif::DomainTag::Request,
                    .objectCount = u8(mObjects.size()),
                    .dataSize = sizes.cmifDataSize,
                    .objectId = service->mObject.value(),
                    .token = mToken,
                });

            writer.write(cmif::InHeader {
                .magic = cmif::cInHeaderMagic,
                .version = 0,
                .command = mCommandId,
                .token = mToken,
            });

            writer.writeIterator<u8>(mData);
            writer.seek(alignUp(writer.tell(), 4));

            if (service->isDomain())
                writer.writeIterator<u32>(mObjects);

            writer.seek(alignUp(writer.tell(), 16));
            writer.writeIterator<hipc::ReceiveStatic>(mHipcReceiveStatics);

            if (mPrintRequest) {
                u8 buf[256] = {};
                memcpy(buf, svc::getTLS()->ipcMessageBuffer, sizeof(svc::ThreadLocalRegion::ipcMessageBuffer));
                diag::debugLog("");
                for (int i = 0; i < sizes.hipcDataSize; i += 16)
                    diag::debugLog("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                        buf[i + 0], buf[i + 1], buf[i + 2], buf[i + 3],
                        buf[i + 4], buf[i + 5], buf[i + 6], buf[i + 7],
                        buf[i + 8], buf[i + 9], buf[i + 10], buf[i + 11],
                        buf[i + 12], buf[i + 13], buf[i + 14], buf[i + 15]);
                memcpy(svc::getTLS()->ipcMessageBuffer, buf, sizeof(svc::ThreadLocalRegion::ipcMessageBuffer));
            }
        }
    };

    struct Response {
        Result result;
        util::FixedCapacityArray<u32, 8> objects;
        util::FixedCapacityArray<Handle, 8> hipcCopyHandles;
        util::FixedCapacityArray<Handle, 8> hipcMoveHandles;
        util::FixedCapacityArray<hipc::Static, 8> hipcSendStatics;
        std::optional<u64> pid;
        std::span<u8> data;

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
        static Response readFromTls(Service* service, bool printResponse) {
            util::Stream reader(svc::getTLS()->ipcMessageBuffer, sizeof(svc::ThreadLocalRegion::ipcMessageBuffer));

            auto header = reader.read<hipc::Header>();

            Response response;
            if (header.hasSpecialHeader) {
                auto specialHeader = reader.read<hipc::SpecialHeader>();

                if (specialHeader.sendPid)
                    response.pid = reader.read<u64>();

                for (u8 i = 0; i < specialHeader.copyHandleCount; i++)
                    response.hipcCopyHandles.add(reader.read<Handle>());
                for (u8 i = 0; i < specialHeader.moveHandleCount; i++)
                    response.hipcMoveHandles.add(reader.read<Handle>());
            }

            for (u8 i = 0; i < header.sendStaticCount; i++)
                response.hipcSendStatics.add(reader.read<hipc::Static>());
            reader.seek(alignUp(reader.tell(), 16));

            size dataWordsLeft = header.dataWords;

            if (service->isDomain()) {
                auto domainOut = reader.read<cmif::DomainOutHeader>();
                for (u8 i = 0; i < domainOut.objectCount; i++)
                    response.objects.add(reader.read<u32>());
                dataWordsLeft -= sizeof(cmif::DomainOutHeader) / 4 + response.objects.size();
            }

            auto outHeader = reader.read<cmif::OutHeader>();
            dataWordsLeft -= sizeof(cmif::OutHeader) / 4;
            response.result = outHeader.result;
            response.data = std::span(svc::getTLS()->ipcMessageBuffer + reader.tell(), dataWordsLeft * 4);

            if (printResponse) {
                u8 buf[256] = {};
                memcpy(buf, svc::getTLS()->ipcMessageBuffer, sizeof(svc::ThreadLocalRegion::ipcMessageBuffer));
                diag::debugLog("");
                for (int i = 0; i < header.dataWords * 4; i += 16)
                    diag::debugLog("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                        buf[i + 0], buf[i + 1], buf[i + 2], buf[i + 3],
                        buf[i + 4], buf[i + 5], buf[i + 6], buf[i + 7],
                        buf[i + 8], buf[i + 9], buf[i + 10], buf[i + 11],
                        buf[i + 12], buf[i + 13], buf[i + 14], buf[i + 15]);
                memcpy(svc::getTLS()->ipcMessageBuffer, buf, sizeof(svc::ThreadLocalRegion::ipcMessageBuffer));
            }

            return response;
        }
    };

    template <typename ResponseHandler>
    inline ValueOrResult<typename util::FunctionTraits<ResponseHandler>::ReturnType> Service::invoke(cmif::MessageTag tag, Request&& request, ResponseHandler handler) {
        request.writeToTls(this, tag);
        HK_TRY(svc::SendSyncRequest(mSession));
        auto response = Response::readFromTls(this, request.mPrintResponse);
        HK_TRY(response.result);
        return handler(response);
    }

    inline Result Service::invoke(cmif::MessageTag tag, Request&& request) {
        request.writeToTls(this, tag);
        HK_TRY(svc::SendSyncRequest(mSession));
        auto response = Response::readFromTls(this, request.mPrintResponse);
        return response.result;
    }
}
