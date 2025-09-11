#pragma once

#include "hk/sf/sf.h"
#include "hk/sf/utils.h"
#include "hk/util/Singleton.h"

namespace hk::am::detail {

    class WindowController : public sf::Service {
        HK_SINGLETON(WindowController);

    public:
        WindowController(sf::Service&& service)
            : sf::Service(forward<sf::Service>(service)) { }

        ValueOrResult<u64> getAppletResourceUserId() {
            return sf::invokeSimple<u64>(this, 1);
        }
    };

} // namespace hk::am::detail
