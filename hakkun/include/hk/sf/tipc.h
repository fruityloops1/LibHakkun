#pragma once

#include "hk/ValueOrResult.h"
#include "hk/container/FixedVec.h"
#include "hk/sf/hipc.h"
#include "hk/sf/sf.h"
#include "hk/svc/api.h"
#include "hk/svc/cpu.h"

namespace hk::sf::tipc {
    static constexpr u16 closeCommandId = 15;

    class Request {
        u16 mCommandId;
        struct {
            bool mSendPid : 1 = false;
        };
        FixedVec<hipc::Buffer, 8> mHipcSendBuffers;
        FixedVec<hipc::Buffer, 8> mHipcReceiveBuffers;
        FixedVec<hipc::Buffer, 8> mHipcExchangeBuffers;
        FixedVec<Handle, 8> mHipcCopyHandles;
        Span<const u8> mData = {};

    public:
        template <typename T>
        Request(u16 commandId, Span<const T> data)
            : mCommandId(commandId)
            , mData(cast<const u8>(data)) { }

        void setSendPid() {
            mSendPid = true;
        }

        void addCopyHandle(Handle handle) {
            mHipcCopyHandles.add(handle);
        }

        template <typename T>
        void addSendBuffer(Span<T> span, hipc::BufferMode mode = hipc::BufferMode::Normal) {
            mHipcSendBuffers.add(hipc::Buffer(mode, span.data(), span.size_bytes()));
        }

        template <typename T>
        void addReceiveBuffer(Span<T> span, hipc::BufferMode mode = hipc::BufferMode::Normal) {
            mHipcReceiveBuffers.add(hipc::Buffer(mode, span.data(), span.size_bytes()));
        }

        template <typename T>
        void addExchangeBuffer(Span<T> span, hipc::BufferMode mode = hipc::BufferMode::Normal) {
            mHipcExchangeBuffers.add(hipc::Buffer(mode, span.data(), span.size_bytes()));
        }

        void writeToTls() {
            std::memset(svc::getTLS()->ipcMessageBuffer, 0, cTlsBufferSize);
            util::Stream writer(svc::getTLS()->ipcMessageBuffer, cTlsBufferSize);
            bool hasSpecialHeader = mSendPid || !mHipcCopyHandles.empty();

            writer.write(hipc::Header {
                .tag = u16(mCommandId),
                .sendStaticCount = 0,
                .sendBufferCount = u8(mHipcSendBuffers.size()),
                .recvBufferCount = u8(mHipcReceiveBuffers.size()),
                .exchBufferCount = u8(mHipcExchangeBuffers.size()),
                .dataWords = u16(alignUp(mData.size_bytes(), 4) / 4),
                .recv_static_mode = 0,
                .hasSpecialHeader = hasSpecialHeader,
            });

            if (hasSpecialHeader) {
                writer.write(hipc::SpecialHeader {
                    .sendPid = mSendPid,
                    .copyHandleCount = u8(mHipcCopyHandles.size()),
                    .moveHandleCount = 0,
                });

                if (mSendPid)
                    writer.write<u64>(0);

                writer.writeIterator<Handle>(mHipcCopyHandles);
            }

            writer.writeIterator<u8>(mData);
        }
    };

    struct Response {
        std::optional<u64> pid;
        FixedVec<Handle, 8> hipcCopyHandles;
        FixedVec<Handle, 8> hipcMoveHandles;
        Span<u8> data;

        static ValueOrResult<Response> readFromTls() {
            util::Stream reader(svc::getTLS()->ipcMessageBuffer, cTlsBufferSize);
            auto header = HK_TRY(reader.read<hipc::Header>());

            Response response;
            if (header.hasSpecialHeader) {
                auto specialHeader = HK_TRY(reader.read<hipc::SpecialHeader>());

                if (specialHeader.sendPid)
                    response.pid = HK_TRY(reader.read<u64>());

                for (u8 i = 0; i < specialHeader.copyHandleCount; i++)
                    response.hipcCopyHandles.add(HK_TRY(reader.read<Handle>()));
                for (u8 i = 0; i < specialHeader.moveHandleCount; i++)
                    response.hipcMoveHandles.add(HK_TRY(reader.read<Handle>()));
            }

            response.data = Span(svc::getTLS()->ipcMessageBuffer + reader.tell(), header.dataWords * 4);

            return response;
        }
    };

    inline ValueOrResult<Response> invokeRequest(sf::Service& service, Request&& request) {
        request.writeToTls();
        HK_TRY(svc::SendSyncRequest(service.handle()));
        return Response::readFromTls();
    }
} // namespace hk::sf::tipc
