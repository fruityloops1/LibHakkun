#pragma once

#include "hk/sf/sf.h"
#include "hk/util/Singleton.h"

namespace hk::am::detail {

    class DisplayController : public sf::Service {
        HK_SINGLETON(DisplayController);

    public:
        DisplayController(sf::Service&& service)
            : sf::Service(forward<sf::Service>(service)) { }
    };

} // namespace hk::am::detail
