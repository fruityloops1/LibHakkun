#pragma once

#include "hk/ValueOrResult.h"
#include "hk/services/am/detail/libraryAppletAccessor.h"
#include "hk/sf/sf.h"
#include "hk/sf/utils.h"
#include "hk/util/Singleton.h"

namespace hk::am::detail {
    enum class LibraryAppletMode : u32 {
        AllForeground,
        PartialForeground,
        NoUi,
        PartialForegroundWithIndirectLayer,
        AllForegroundInitiallyHidden,
    };
    class LibraryAppletCreator : public sf::Service {
        HK_SINGLETON(LibraryAppletCreator);

    public:
        LibraryAppletCreator(sf::Service&& service)
            : sf::Service(forward<sf::Service>(service)) { }

        ValueOrResult<LibraryAppletAccessor> createLibraryApplet(u32 appletId, LibraryAppletMode mode) {
            auto input = sf::packInput(appletId, mode);
            return invokeRequest(sf::Request(this, 0, &input), sf::subserviceExtractor(this))
                .map([](sf::Service&& service) {
                    return LibraryAppletAccessor(forward<sf::Service>(service));
                });
        }

        void createStorage() {
            HK_TODO();
        }
    };
}
