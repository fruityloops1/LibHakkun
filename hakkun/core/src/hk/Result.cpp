#if HK_RESULT_ADVANCED
#include "hk/Result.h"
#include "hk/container/Array.h"
#include "hk/diag/diag.h"
#include <atomic>

namespace hk::detail {

    static Array<ResultDebugReference, ResultDebugReference::cMaxDebugRefs> sDebugRefs;
    static std::atomic<ResultDebugReference::Ref> sCurRefIdx = 1;

    Tuple<ResultDebugReference::Ref, ResultDebugReference&> ResultDebugReference::allocate() {
        if (sCurRefIdx >= cMaxDebugRefs)
            sCurRefIdx = 1;

        Ref ref = sCurRefIdx++;

        return { ref, sDebugRefs[ref - 1] };
    }

    ResultDebugReference& ResultDebugReference::get(Ref idx) { return sDebugRefs[idx - 1]; }

} // namespace hk::detail

#endif