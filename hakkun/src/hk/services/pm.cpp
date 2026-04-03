#include "hk/services/pm.h"

namespace hk::pm {

    HK_SINGLETON_IMPL(ProcessManagerForDebugMonitor);
    HK_SINGLETON_IMPL(ProcessManagerForShell);
    HK_SINGLETON_IMPL(ProcessManagerBootMode);

} // namespace hk::pm
