#include "hk/diag/diag.h"
#include "hk/svc/api.h"

namespace hk::diag {

    hk_noreturn void abortImpl(svc::BreakReason reason, Result result, const char* file, int line, const char* msgFmt, ...) {
        svc::Break(reason, &result, sizeof(result));
    }

    hk_noreturn void abortImpl(svc::BreakReason reason, Result result, const char* file, int line, const char* msgFmt, std::va_list arg) {
        svc::Break(reason, &result, sizeof(result));
    }

} // namespace hk::diag
