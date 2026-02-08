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
            auto request = sf::Request(this, 1);
            request.forceDataSize(8);
            return invokeRequest(move(request), sf::inlineDataExtractor<u64>());
        }

        Result acquireForegroundRights() {
            return sf::invokeSimple(this, 10);
        }
    };

} // namespace hk::am::detail
