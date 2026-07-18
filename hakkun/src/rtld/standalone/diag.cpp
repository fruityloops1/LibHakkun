#include "hk/diag/diag.h"
#include "hk/svc/api.h"
#include <cstring>

namespace hk::diag {

    const char* getResultName(hk::Result) { return nullptr; }

    hk_noreturn void abortImpl(svc::BreakReason reason, Result result, const char* file, int line, const char* msgFmt, ...) {
        svc::Break(reason, &result, sizeof(result));
    }

    hk_noreturn void abortImpl(svc::BreakReason reason, Result result, const char* file, int line, const char* msgFmt, std::va_list arg) {
        svc::Break(reason, &result, sizeof(result));
    }

#if !defined(HK_RELEASE) or defined(HK_RELEASE_DEBINFO)
    // no format

    void logBuffer(const char* buf, size length) { svc::OutputDebugString(buf, length); }
    void logImpl(const char* fmt, std::va_list list) { svc::OutputDebugString(fmt, strlen(fmt)); }
    void log(const char* fmt, ...) { svc::OutputDebugString(fmt, strlen(fmt)); }
    void logLineImpl(const char* fmt, std::va_list list) { svc::OutputDebugString(fmt, strlen(fmt)); }
    void logLine(const char* fmt, ...) { svc::OutputDebugString(fmt, strlen(fmt)); }
    void debugLog(const char* msg, ...) { svc::OutputDebugString(msg, strlen(msg)); }
#endif

} // namespace hk::diag
