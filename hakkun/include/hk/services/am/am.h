#include "hk/services/am/detail/service.h"
namespace hk::am {
    template<bool Application>
    Result initialize() {
        am::detail::AppletManager::initialize<Application>();
    }
}
