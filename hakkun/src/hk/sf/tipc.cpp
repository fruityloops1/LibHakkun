#include "hk/sf/tipc.h"
#include "hk/ValueOrResult.h"
#include "hk/diag/diag.h"
#include "hk/sf/hipc.h"
#include "hk/svc/cpu.h"
#include "hk/types.h"
#include "hk/util/Stream.h"

namespace hk::tipc {
    using namespace sf::hipc;
    constexpr size cTlsBufferSize = sizeof(svc::ThreadLocalRegion::ipcMessageBuffer);

    void Request::writeToTls() {
        bool hasSpecialHeader = mSendPid || !mHipcCopyHandles.empty();
        util::Stream writer(svc::getTLS()->ipcMessageBuffer, cTlsBufferSize);
        std::memset(svc::getTLS()->ipcMessageBuffer, 0, cTlsBufferSize);

        writer.write(Header {
            .tag = u16(16 + mCommandId),
            .sendStaticCount = 0,
            .sendBufferCount = u8(mHipcSendBuffers.size()),
            .recvBufferCount = u8(mHipcReceiveBuffers.size()),
            .exchBufferCount = u8(mHipcExchangeBuffers.size()),
            .dataWords = u16(alignUp(mData.size_bytes(), 4) / 4),
            .recvStaticMode = 0,
            .hasSpecialHeader = hasSpecialHeader,
        });

        if (hasSpecialHeader) {
            writer.write(SpecialHeader {
                .sendPid = mSendPid,
                .copyHandleCount = u8(mHipcCopyHandles.size()),
                .moveHandleCount = 0,
            });

            if (mSendPid)
                writer.write<u64>(0);

            writer.writeIterator<Handle>(mHipcCopyHandles);
        }

        writer.writeIterator<Buffer>(mHipcSendBuffers);
        writer.writeIterator<Buffer>(mHipcReceiveBuffers);
        writer.writeIterator<Buffer>(mHipcExchangeBuffers);
        writer.writeIterator<u8>(mData);

#if !defined(HK_RELEASE)
        if (mPrintRequest) {
            u8 buf[256] = {};
            memcpy(buf, svc::getTLS()->ipcMessageBuffer, cTlsBufferSize);
            diag::logLine("Request: ");
            for (int i = 0; i < writer.tell(); i += 16)
                diag::logLine("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                    buf[i + 0], buf[i + 1], buf[i + 2], buf[i + 3],
                    buf[i + 4], buf[i + 5], buf[i + 6], buf[i + 7],
                    buf[i + 8], buf[i + 9], buf[i + 10], buf[i + 11],
                    buf[i + 12], buf[i + 13], buf[i + 14], buf[i + 15]);
            memcpy(svc::getTLS()->ipcMessageBuffer, buf, cTlsBufferSize);
        }

        if (mAbortAfterRequest)
            HK_ABORT("Aborted after response (debug)", 0);
#endif
    }

    ValueOrResult<Response> Response::readFromTls(bool printResponse) {
        util::Stream reader(svc::getTLS()->ipcMessageBuffer, cTlsBufferSize);
        Response response = Response();

        Header header = HK_UNWRAP(reader.read<Header>());
        if (header.hasSpecialHeader) {
            SpecialHeader specialHeader = HK_UNWRAP(reader.read<SpecialHeader>());

            response.mHipcHandles = HK_UNWRAP(reader.readArray<Handle, 8>(specialHeader.moveHandleCount));
        }

#if !defined(HK_RELEASE)
        if (printResponse) {
            util::Stream reader(svc::getTLS()->ipcMessageBuffer, cTlsBufferSize);
            u8 buf[256] = {};
            memcpy(buf, svc::getTLS()->ipcMessageBuffer, cTlsBufferSize);
            diag::logLine("Response:");
            u32 wordCount = reader.tell() / 4 + header.dataWords;
            for (int i = 0; i < wordCount * 4; i += 16)
                diag::logLine("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                    buf[i + 0], buf[i + 1], buf[i + 2], buf[i + 3],
                    buf[i + 4], buf[i + 5], buf[i + 6], buf[i + 7],
                    buf[i + 8], buf[i + 9], buf[i + 10], buf[i + 11],
                    buf[i + 12], buf[i + 13], buf[i + 14], buf[i + 15]);
            memcpy(svc::getTLS()->ipcMessageBuffer, buf, cTlsBufferSize);
        }
#endif

        // result is stored in the data words
        header.dataWords -= 1;
        HK_TRY(Result(HK_TRY(reader.read<u32>())));

        response.mData.set(svc::getTLS()->ipcMessageBuffer + reader.tell(), (header.dataWords * 4));

        return response;
    }
}
