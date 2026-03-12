#include "hk/services/apm/service.h"
#include "hk/util/Singleton.h"

namespace hk::apm {

    HK_SINGLETON_IMPL(ApmManager);
    HK_SINGLETON_IMPL(ApmSession);

} // namespace hk::apm
