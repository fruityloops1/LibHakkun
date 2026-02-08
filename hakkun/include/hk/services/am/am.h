#pragma once

#include "hk/diag/diag.h"
#include "hk/services/am/appType.h"
#include "hk/services/am/detail/service.h"

namespace hk::am {

    template <AppType Type>
    Result initialize() {
        detail::AppletManager* appletManager = HK_TRY(am::detail::AppletManager::initialize<Type>());
        auto proxy = HK_TRY(appletManager->initializeApplicationProxy());
        HK_TRY(proxy->initializeApplicationFunctions());
        HK_TRY(proxy->initializeLibraryAppletCreator());
        HK_TRY(proxy->initializeCommonStateGetter());
        HK_TRY(proxy->initializeSelfController());
        HK_TRY(proxy->initializeWindowController());
        HK_TRY(proxy->initializeAudioController());
        HK_TRY(proxy->initializeDisplayController());
        HK_TRY(proxy->initializeDebugFunctions());

        return ResultSuccess();
    }

    inline ValueOrResult<u64> getAppletResourceUserId() {
        return detail::WindowController::instance()->getAppletResourceUserId();
    }

} // namespace hk::am
