#include "hk/diag/ipclogger.h"
#include "hk/ValueOrResult.h"
#include "hk/os/Mutex.h"
#include <atomic>

namespace hk::diag::ipclogger {
    IpcLogger IpcLogger::sInstance = IpcLogger();
    __attribute__((weak)) bool IpcLogger::mInitializeManually = false;

    Result IpcLogger::initialize() {
        // UserBuffer syscalls can't have their buffers on the stack :(
        alignas(cPageSize) static std::array<u8, cPageSize> messageBuffer;
        bool failed = true;

        auto handle = HK_TRY(svc::ConnectToNamedPort("hklog"));

        // application processes are only permitted to have one port open at a time
        defer { svc::CloseHandle(handle); };
        defer_if(failed) {
            sInstance.mSession = cInvalidSession;
        };

        util::Stream stream(messageBuffer.data(), messageBuffer.size());
        stream.write(sf::hipc::Header { .tag = 2, .dataWords = 4 });
        auto res = svc::SendSyncRequestWithUserBuffer(std::span<u8>(messageBuffer), handle);
        if (res.failed())
            svc::Break(svc::BreakReason_User, nullptr, res.getValue());

        stream.seek(0);
        auto header = HK_UNWRAP(stream.read<sf::hipc::Header>());
        if (header.tag == 1)
            return MAKE_RESULT(ResultSessionMoveFailed());

        auto special = HK_UNWRAP(stream.read<sf::hipc::SpecialHeader>());
        HK_ASSERT(special.moveHandleCount == 1);
        auto sessionHandle = HK_UNWRAP(stream.read<Handle>());

        failed = false;
        sInstance.mSession.store(sessionHandle, std::memory_order_release);
        return ResultSuccess();
    }

    IpcLogger* IpcLogger::instance() {
        if (!sInstance.mSession.load(std::memory_order_relaxed)) {
            static os::Mutex initLock;
            auto guard = initLock.lockScoped();

            if (sInstance.mSession.load(std::memory_order_acquire))
                return &sInstance;

            auto result = initialize();
        }

        return &sInstance;
    }
} // namespace hk::diag::ipclogger
