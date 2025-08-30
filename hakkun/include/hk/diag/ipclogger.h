#include "hk/sf/sf.h"
#include "hk/svc/api.h"
#include "hk/util/Stream.h"
#include <limits>
#include <span>

namespace hk::diag::ipclogger {
    class IpcLogger {
        static constexpr Handle cInvalidSession = std::numeric_limits<Handle>::max();
        static IpcLogger sInstance;
        Handle mSession = 0;

        bool isDisconnected() const {
            return !mSession || mSession == cInvalidSession;
        }

        void logImpl(std::span<const u8> buffer, u16 tag) {
            if (isDisconnected())
                return;

            hk::util::Stream stream(hk::svc::getTLS()->ipcMessageBuffer, hk::sf::cTlsBufferSize);
            stream.write(hk::sf::hipc::Header { .tag = tag, .sendBufferCount = 1, .dataWords = 8 });
            stream.write(hk::sf::hipc::Buffer(sf::hipc::BufferMode::Normal, u64(buffer.data()), buffer.size()));
            auto res = svc::SendSyncRequest(mSession);
            if (res.failed())
                svc::Break(svc::BreakReason_User, nullptr, res.getValue());
        }
    public:
        static IpcLogger* instance();

        void logWithLine(std::span<const u8> buffer) {
            logImpl(buffer, 0);
        }

        void logWithoutLine(std::span<const u8> buffer) {
            logImpl(buffer, 1);
        }
    };
} // namespace hk::diag::ipclogger
