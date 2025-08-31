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
    void debugLog(const char* msg, ...) { // no format
        const size len = strlen(msg);

        svc::OutputDebugString(msg, len);
    }
#endif

} // namespace hk::diag
