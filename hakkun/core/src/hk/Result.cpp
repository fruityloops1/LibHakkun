#include "hk/container/Array.h"
#if HK_RESULT_ADVANCED

#include "hk/Result.h"
#include "hk/diag/diag.h"
#include <atomic>

namespace hk::detail {

    static Array<ResultDebugReference, ResultDebugReference::cMaxDebugRefs> sDebugRefs;
    static Array<ResultDebugReference::Msg, ResultDebugReference::cMaxDebugRefs> sDebugRefMsgs;
    static std::atomic<ResultDebugReference::Ref> sCurRefIdx = 1;

    Tuple<ResultDebugReference::Ref, ResultDebugReference&> ResultDebugReference::allocate(const Msg& msg) {
        if (sCurRefIdx >= cMaxDebugRefs)
            sCurRefIdx = 1;

        Ref ref = sCurRefIdx++;
        sDebugRefMsgs[ref - 1] = msg;

        return { ref, sDebugRefs[ref - 1] };
    }

    ResultDebugReference& ResultDebugReference::get(Ref idx) { return sDebugRefs[idx - 1]; }
    const ResultDebugReference::Msg& ResultDebugReference::getMsg(Ref idx) { return sDebugRefMsgs[idx - 1]; }

} // namespace hk::detail

#endif