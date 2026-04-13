#pragma once

#include "hk/container/Span.h"
#include "hk/sf/sf.h"
#include "hk/svc/api.h"
#include "hk/util/Stream.h"
#include <atomic>
#include <limits>

namespace hk::diag::ipclogger {
    class IpcLogger {
        static constexpr Handle cInvalidSession = std::numeric_limits<u32>::max();
        static IpcLogger sInstance;
        std::atomic<Handle> mSession = Handle();

        bool isDisconnected() const {
            auto session = mSession.load(std::memory_order_acquire);
            return !session || session == cInvalidSession;
        }

        void logImpl(Span<const u8> buffer, u16 tag) {
            if (isDisconnected())
                return;

            util::Stream stream(svc::getTLS()->ipcMessageBuffer, sf::cTlsBufferSize);
            stream.write(sf::hipc::Header { .tag = tag, .sendBufferCount = 1, .dataWords = 8 });
            stream.write(sf::hipc::Buffer(sf::hipc::BufferMode::Normal, u64(buffer.data()), buffer.size()));
            auto res = svc::SendSyncRequest(mSession);
            if (res.failed())
                svc::Break(svc::BreakReason_User, nullptr, res.getValue());
        }

    public:
        static IpcLogger* instance();

        void logWithLine(Span<const u8> buffer) {
            logImpl(buffer, 0);
        }

        void logWithoutLine(Span<const u8> buffer) {
            logImpl(buffer, 1);
        }
    };
} // namespace hk::diag::ipclogger
