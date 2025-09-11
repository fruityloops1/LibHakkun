#include "hk/services/am/detail/applicationProxy.h"
#include "hk/services/am/detail/commonStateGetter.h"
#include "hk/services/am/detail/libraryAppletCreator.h"
#include "hk/services/am/detail/selfController.h"
#include "hk/services/am/detail/service.h"
#include "hk/services/am/detail/windowController.h"
#include "hk/util/Singleton.h"

namespace hk::am {
    HK_SINGLETON_IMPL(detail::AppletManager);
    HK_SINGLETON_IMPL(detail::ApplicationProxy);
    HK_SINGLETON_IMPL(detail::LibraryAppletCreator);
    HK_SINGLETON_IMPL(detail::CommonStateGetter);
    HK_SINGLETON_IMPL(detail::SelfController);
    HK_SINGLETON_IMPL(detail::WindowController);
} // namespace hk::am
