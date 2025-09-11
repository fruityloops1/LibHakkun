#include "hk/services/am/detail/service.h"
namespace hk::am {
    using detail::AppType;
    template<AppType Type>
    Result initialize() {
        return am::detail::AppletManager::initialize<Type>();
    }
}
