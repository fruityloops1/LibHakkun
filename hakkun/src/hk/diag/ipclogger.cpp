#include "hk/diag/ipclogger.h"
#include "hk/ValueOrResult.h"
#include "hk/os/Mutex.h"
#include <atomic>

namespace hk::diag::ipclogger {
    IpcLogger IpcLogger::sInstance = IpcLogger();

    ValueOrResult<Handle> initialize() {
        // UserBuffer syscalls can't have their buffers on the stack :(
        alignas(cPageSize) static std::array<u8, cPageSize> messageBuffer;

        auto handle = HK_TRY(svc::ConnectToNamedPort("hklog"));

        util::Stream stream(messageBuffer.data(), messageBuffer.size());
        stream.write(sf::hipc::Header { .tag = 2, .dataWords = 4 });
        auto res = svc::SendSyncRequestWithUserBuffer(messageBuffer, handle);
        if (res.failed())
            svc::Break(svc::BreakReason_User, nullptr, res.getValue());

        stream.seek(0);
        auto header = stream.read<sf::hipc::Header>();
        if (header.tag == 1)
            return ResultSessionMoveFailed();
        auto special = stream.read<sf::hipc::SpecialHeader>();
        auto sessionHandle = stream.read<Handle>();

        // application processes are only permitted to have one port open at a time
        svc::CloseHandle(handle);

        return sessionHandle;
    }

    IpcLogger* IpcLogger::instance() {
        if (!sInstance.mSession.load(std::memory_order_relaxed)) {
            static os::Mutex initLock;
            auto guard = initLock.lockScoped();

            if (sInstance.mSession.load(std::memory_order_acquire))
                return &sInstance;

            auto result = initialize();

            sInstance.mSession.store(result.hasValue() ? result.getInnerValue()
                                                       : sInstance.mSession = cInvalidSession,
                std::memory_order_release);
        }

        return &sInstance;
    }
} // namespace hk::diag::ipclogger
