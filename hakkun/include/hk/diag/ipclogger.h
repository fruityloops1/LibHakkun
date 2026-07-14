#pragma once

#include "hk/container/Span.h"
#include <atomic>

namespace hk::diag::ipclogger {
    class IpcLogger {
        static IpcLogger sInstance;
        std::atomic<Handle> mSession = Handle();

        bool isDisconnected() const {
            return !mSession.load(std::memory_order_acquire);
        }

        void logImpl(Span<const u8> buffer, u16 tag);

    public:
        static IpcLogger& instance() { return sInstance; }
        static Result initialize();
        static void initializeWithHandle(Handle handle) { sInstance.mSession.store(handle, std::memory_order_release); }

        void logWithLine(Span<const u8> buffer) {
            logImpl(buffer, 0);
        }

        void logWithoutLine(Span<const u8> buffer) {
            logImpl(buffer, 1);
        }
    };
} // namespace hk::diag::ipclogger
