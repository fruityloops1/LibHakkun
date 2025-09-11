#pragma once

#include "hk/ValueOrResult.h"
#include "hk/sf/sf.h"
#include "hk/sf/utils.h"
#include "hk/util/Singleton.h"

namespace hk::am::detail {

    class SelfController : public sf::Service {
        HK_SINGLETON(SelfController);

    public:
        SelfController(sf::Service&& service)
            : sf::Service(forward<sf::Service>(service)) { }

        ValueOrResult<u64> createManagedDisplayLayer() {
            return sf::invokeSimple<u64>(this, 40);
        }
    };

} // namespace hk::am::detail
