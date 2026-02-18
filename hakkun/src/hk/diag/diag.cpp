#include "hk/diag/diag.h"
#include "hk/Result.h"
#include "hk/diag/ipclogger.h"
#include "hk/ro/RoUtil.h"
#include "hk/svc/api.h"
#include "hk/svc/types.h"
#include "hk/util/Context.h"
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

    ValueOrResult<const char*> getCurrentThreadName() {
        auto* tls = svc::getTLS();
        auto* thread = tls->nnsdkThread;
        HK_UNLESS(thread != nullptr, ResultNotAnNnsdkThread());
        return thread->threadNamePtr == nullptr ? thread->threadName : thread->threadNamePtr;
    }

    void dumpStackTrace() {
        ptr address = -1;
        int level = 1;
        if (getCurrentThreadName()
                .map([](const char* threadName) {
                    logLine("Stack Trace on Thread[%s]:", threadName);
                })
                .failed())
            logLine("Stack Trace:");

        while ((address = util::getReturnAddress(level++)) != 0) {
            auto* module = ro::getModuleContaining(address);
            if (module != nullptr) {
                const ptr offset = address - module->range().start();
                if (module->getModuleName())
                    logLine("\tReturn[%02d]: %016zX (%s + 0x%zx)", level - 1, address, module->getModuleName(), offset);

                else {
                    const u8* d = module->getBuildId();

                    static_assert(ro::cBuildIdSize == 0x10);
                    logLine("\tReturn[%02d]: %016zX ([%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x] + 0x%zx)", level - 1, address, module->getBuildId(),
                        d[0], d[1], d[2], d[3],
                        d[4], d[5], d[6], d[7],
                        d[8], d[9], d[10], d[11],
                        d[12], d[13], d[14], d[15],
                        offset);
                }
            } else
                logLine("\tReturn[%02d]: %016zX", level - 1, address);
        }
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

    constexpr char cAbortFormat[] = R"(
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
        snprintf(headerMsgBuf, sizeof(headerMsgBuf), cAbortFormat, file, line);

        logLine(headerMsgBuf);
        logLine(userMsgBuf);

        dumpStackTrace();

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
    void logBuffer(const char* buf, size length) {
        ipclogger::IpcLogger::instance()->logWithoutLine({ cast<const u8*>(buf), length });
        hkLogSink(buf, length);
    }

    void logImpl(const char* fmt, std::va_list list) {
        std::va_list listCopy;
        va_copy(listCopy, list);
        size len = vsnprintf(nullptr, 0, fmt, list);
        char buf[len + 1];
        vsnprintf(buf, len + 1, fmt, listCopy);

        ipclogger::IpcLogger::instance()->logWithoutLine({ cast<const u8*>(buf), len });
        hkLogSink(buf, len);
    }

    void log(const char* fmt, ...) {
        std::va_list args;
        va_start(args, fmt);
        logImpl(fmt, args);
        va_end(args);
    }

    void logLineImpl(const char* fmt, std::va_list list) {
        std::va_list listCopy;
        va_copy(listCopy, list);
        size len = vsnprintf(nullptr, 0, fmt, list);
        char buf[len + 2];
        vsnprintf(buf, len + 2, fmt, listCopy);
        ipclogger::IpcLogger::instance()->logWithLine({ cast<const u8*>(buf), len });

        buf[len] = '\n';
        hkLogSink(buf, len + 1);
    }

    void logLine(const char* fmt, ...) {
        std::va_list args;
        va_start(args, fmt);
        logLineImpl(fmt, args);
        va_end(args);
    }

    void debugLog(const char* fmt, ...) {
        std::va_list args;
        va_start(args, fmt);
        logLineImpl(fmt, args);
        va_end(args);
    }
#endif

} // namespace hk::diag
