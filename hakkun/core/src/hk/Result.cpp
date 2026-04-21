#if HK_RESULT_ADVANCED

#include "hk/Result.h"
#include "hk/diag/diag.h"
#include <atomic>

namespace hk::detail {

    static ResultDebugReference sBuffer[ResultDebugReference::cMaxDebugRefs];
    static std::atomic<ResultDebugReference::Ref> sCurRefIdx = 1;

    Tuple<ResultDebugReference::Ref, ResultDebugReference&> ResultDebugReference::allocate() {
        if (sCurRefIdx >= cMaxDebugRefs)
            sCurRefIdx = 1;

        Ref ref = sCurRefIdx++;
        return { ref, sBuffer[ref - 1] };
    }

    ResultDebugReference& ResultDebugReference::get(Ref idx) {
        HK_ABORT_UNLESS(idx <= cMaxDebugRefs, "hk::detail::ResultDebugReference: invalid (%d)", idx);
        return sBuffer[idx - 1];
    }

} // namespace hk::detail

#endif