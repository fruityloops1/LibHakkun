#pragma once

#include "hk/services/am/detail/service.h"

namespace hk::am {

    using detail::AppType;
    template <AppType Type>
    Result initialize() {
        detail::AppletManager* appletManager = HK_TRY(am::detail::AppletManager::initialize<Type>());
        auto proxy = HK_TRY(appletManager->initializeApplicationProxy());
        HK_TRY(proxy->initializeSelfController());
        HK_TRY(proxy->initializeCommonStateGetter());
        HK_TRY(proxy->initializeWindowController());

        return ResultSuccess();
    }

    inline ValueOrResult<u64> getAppletResourceUserId() {
        return detail::WindowController::instance()->getAppletResourceUserId();
    }

} // namespace hk::am
