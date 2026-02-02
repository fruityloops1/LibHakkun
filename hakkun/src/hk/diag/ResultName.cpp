#if !defined(HK_RELEASE) or defined(HK_RELEASE_DEBINFO)
#define INCLUDE_HK_DETAIL_DEFAULTRESULTS
#include "hk/Result.h"
#include "hk/svc/types.h"
#include "hk/util/TypeName.h"

namespace hk::diag {
    hk_noreturn void abortImpl(hk::svc::BreakReason reason, Result result, const char* file, int line, const char* msgFmt, ...);
} // namespace hk::diag

struct ResultNameEntry {
    u32 value;
    const char* name;
} __attribute__((packed));

extern ResultNameEntry cResultNames[];
extern const size cMaxNames;
static size cNumResultNames = 0;

template <hk::Derived<hk::Result> Result>
struct AddResultName {
    AddResultName() {
        if (cNumResultNames >= cMaxNames)
            hk::diag::abortImpl(hk::svc::BreakReason_Assert, 0, __FILE__, __LINE__, "ResultName buffer is full (size: 0x%zx)", cMaxNames);
        cResultNames[cNumResultNames++] = { Result(), hk::util::getTypeName<Result>() };
    }
};

#undef HK_DEFINE_RESULT
#define HK_DEFINE_RESULT(NAME, DESCRIPTION)                                                                           \
    struct Result##NAME : ::hk::ResultV<_hk_result_id_namespace::Module::cModule, DESCRIPTION + __COUNTER__ * 0> { }; \
    ::AddResultName<Result##NAME> _Add_##NAME;

constexpr size cResultCountStart = __COUNTER__;

#define HK_COLLECTING_RESULTNAMES
#include "HakkunResults.ih" // IWYU pragma: keep

namespace hk {

#include "hk/detail/DefaultResults.ih" // IWYU pragma: keep
#undef INCLUDE_HK_DETAIL_DEFAULTRESULTS

} // namespace hk

#undef HK_COLLECTING_RESULTNAMES

constexpr size cResultCount = __COUNTER__ - cResultCountStart;
ResultNameEntry cResultNames[cResultCount];
const size cMaxNames = cResultCount;

namespace hk::diag {

    const char* getResultName(hk::Result result) {
        for (size i = 0; i < cResultCount; i++) {
            if (cResultNames[i].value == result.getValue())
                return cResultNames[i].name;
        }
        return nullptr;
    }

} // namespace hk::diag
#endif
