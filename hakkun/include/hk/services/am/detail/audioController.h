#pragma once

#include "hk/sf/sf.h"
#include "hk/util/Singleton.h"

namespace hk::am::detail {

    class AudioController : public sf::Service {
        HK_SINGLETON(AudioController);

    public:
        AudioController(sf::Service&& service)
            : sf::Service(forward<sf::Service>(service)) { }
    };

} // namespace hk::am::detail
