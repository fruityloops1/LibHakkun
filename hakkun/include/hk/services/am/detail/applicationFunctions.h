#pragma once

#include "hk/ValueOrResult.h"
#include "hk/sf/sf.h"
#include "hk/sf/utils.h"
#include "hk/util/Singleton.h"

namespace hk::am::detail {

    class ApplicationFunctions : public sf::Service {
        HK_SINGLETON(ApplicationFunctions);

    public:
        ApplicationFunctions(sf::Service&& service)
            : sf::Service(forward<sf::Service>(service)) { }

        Result createApplicationAndRequestToStart(u64 applicationId) {
            return sf::invokeSimple(this, 12, applicationId);
        }

        ValueOrResult<bool> notifyRunning() {
            return sf::invokeSimple<bool>(this, 40);
        }

        Result requestToSleep() {
            return sf::invokeSimple(this, 72);
        }
    };

} // namespace hk::am::detail
