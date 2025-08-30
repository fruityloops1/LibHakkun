#include "hk/diag/diag.h"
#include "hk/Result.h"
#include "hk/diag/ipclogger.h"
#include "hk/ro/RoUtil.h"
#include "hk/svc/api.h"
#include "hk/svc/types.h"
#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace hk::diag {

    Result setCurrentThreadName(const char* name) {
        auto* tls = svc::getTLS();
        auto* thread = tls->nnsdkThread;
        HK_UNLESS(thread != nullptr, ResultNotAnNnsdkThread());
        thread->threadNamePtr = thread->threadName;
        std::strncpy(thread->threadName, name, sizeof(thread->threadName));
        return ResultSuccess();
    }

    static void* setAbortMsg(const ro::RoModule* module, const char* msg, int idx) {
        const size arbitraryOffset = 0x69 * idx;

        Elf_Sym sym;
        sym.st_shndx = STT_FUNC;
        sym.st_name = 0x100 * idx;
        sym.st_size = 42;
        sym.st_info = 2;
        sym.st_other = 0;
        sym.st_value = arbitraryOffset;

        ptr strTab = ptr(module->getNnModule()->m_pStrTab);
        strTab -= module->range().start();
        ptr dynSym = ptr(module->getNnModule()->m_pDynSym);
        dynSym -= module->range().start();

        module->writeRo(dynSym + sizeof(Elf_Sym) * idx, &sym, sizeof(sym));
        module->writeRo(strTab + 0x100 * idx, msg, __builtin_strlen(msg) + 1);

        return (void*)(module->range().start() + arbitraryOffset);
    }

    constexpr char sAbortFormat[] = R"(
~~~ HakkunAbort ~~~
File: %s:%d
)";

    hk_noreturn void abortImpl(svc::BreakReason reason, Result result, const char* file, int line, const char* msgFmt, ...) {
        va_list arg;
        va_start(arg, msgFmt);
        abortImpl(reason, result, file, line, msgFmt, arg);
        va_end(arg);
    }

    hk_noreturn void abortImpl(svc::BreakReason reason, Result result, const char* file, int line, const char* msgFmt, std::va_list arg) {
#if !defined(HK_RELEASE) or defined(HK_RELEASE_DEBINFO)
        char userMsgBuf[0x80];
        vsnprintf(userMsgBuf, sizeof(userMsgBuf), msgFmt, arg);

        char headerMsgBuf[0x80];
        snprintf(headerMsgBuf, sizeof(headerMsgBuf), sAbortFormat, file, line);

        debugLog(headerMsgBuf);
        debugLog(userMsgBuf);

        auto* module = ro::getSelfModule();
        if (module) {
            void* headerSym = setAbortMsg(module, headerMsgBuf, 0);
            void* userSym = setAbortMsg(module, userMsgBuf, 1);
            svc::BreakWithMessage(reason, &result, sizeof(result), headerSym, userSym);
        } else
#endif
            svc::Break(reason, &result, sizeof(result));
    }

    extern "C" void __attribute__((weak)) hkLogSink(const char* msg, size len) {
        svc::OutputDebugString(msg, len);
    }

#if !defined(HK_RELEASE) or defined(HK_RELEASE_DEBINFO)
    void debugLogNoLineImpl(const char* buf, size length) {
        ipclogger::IpcLogger::instance()->logWithoutLine({cast<const u8*>(buf), length});
        hkLogSink(buf, length);
    }
    void debugLogNoLine(const char* fmt, ...) {
        std::va_list args;
        va_start(args, fmt);
        size len = vsnprintf(nullptr, 0, fmt, args);
        char buf[len + 1];
        vsnprintf(buf, len + 1, fmt, args);
        va_end(args);

        ipclogger::IpcLogger::instance()->logWithoutLine({cast<const u8*>(buf), len});
        hkLogSink(buf, len);
    }
    void debugLogImpl(const char* buf, size length) {
        ipclogger::IpcLogger::instance()->logWithLine({cast<const u8*>(buf), length});
        hkLogSink(buf, length);
        const char cNewline = '\n';
        hkLogSink(&cNewline, 1);
    }
    void debugLog(const char* fmt, ...) {
        std::va_list args;
        va_start(args, fmt);
        size len = vsnprintf(nullptr, 0, fmt, args);
        char buf[len + 2];
        vsnprintf(buf, len + 2, fmt, args);
        va_end(args);
        ipclogger::IpcLogger::instance()->logWithLine({cast<const u8*>(buf), len});

        buf[len] = '\n';
        hkLogSink(buf, len + 1);
    }
#endif

} // namespace hk::diag
