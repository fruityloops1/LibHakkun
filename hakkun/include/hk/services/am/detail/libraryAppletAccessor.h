#pragma once

#include "hk/sf/sf.h"

namespace hk::am::detail {
    class LibraryAppletAccessor : public sf::Service {
    public:
        LibraryAppletAccessor(sf::Service&& service)
            : sf::Service(forward<sf::Service>(service)) { }
    };
}
