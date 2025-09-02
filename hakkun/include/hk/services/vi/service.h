#pragma once

#include "hk/ValueOrResult.h"
#include "hk/diag/diag.h"
#include "hk/services/sm.h"
#include "hk/sf/sf.h"
#include "hk/sf/utils.h"
#include "hk/svc/api.h"
#include "hk/svc/types.h"
#include "hk/types.h"
#include "hk/util/Singleton.h"
#include "hk/util/TemplateString.h"
#include "hk/util/Tuple.h"
#include <type_traits>
#include <utility>

namespace hk::vi {

    struct DisplayName {
        char data[0x40];
    };

    struct Display {
        bool isInitialized = false;
        u64 id = 0;
        DisplayName displayName;
    };

    struct DisplayInfo {
        DisplayName displayName;
        bool hasLayerLimit;
        u64 layerCountMax;
        u64 layerWidthPixelCountMax;
        u64 layerHeightPixelCountMax;
    };

    class ApplicationDisplayService : sf::Service {
        HK_SINGLETON(ApplicationDisplayService);

    public:
        ApplicationDisplayService(sf::Service&& service)
            : sf::Service(std::forward<sf::Service>(service)) { }

        size listDisplays(std::span<DisplayInfo> displays) {
            auto request = sf::Request(this, 1000);
            request.addOutMapAlias(displays.data(), displays.size_bytes());
            HK_ABORT_UNLESS_R(invokeRequest(move(request)));
            return displays.size();
        }

        u64 openDisplay(Display& display) {
            return HK_UNWRAP(sf::invokeSimple<u64>(*this, 1010, &display.displayName));
        }

        u64 openDefaultDisplay() {
            return HK_UNWRAP(sf::invokeSimple<u64>(*this, 1011));
        }

        Result closeDisplay(Display& display) {
            return sf::invokeSimpleVoid(*this, 1020, &display.id);
        }

        Result setDisplayEnabled(Display& display, bool isEnabled) {
            return sf::invokeSimpleVoid(*this, 1101, u32(isEnabled), &display.id);
        }

        ValueOrResult<Tuple<u64, u64>> getDisplayResolution(Display& display) {
            return sf::invokeSimple<Tuple<u64, u64>>(*this, 1102, &display.id);
        }

        ValueOrResult<Handle> getDisplayVsyncEvent(Display& display) {
            auto request = sf::Request(this, 5202, &display.id);
            return invokeRequest(move(request), [](sf::Response& response){ return response.nextCopyHandle(); });
        }

        Handle getDisplayVsyncEventForDebug(Display& display) {
            auto request = sf::Request(this, 5203, &display.id);
            return HK_UNWRAP(invokeRequest(move(request), [](sf::Response& response){ return response.nextCopyHandle(); }));
        }
    };

    class VideoInterface : sf::Service {
        HK_SINGLETON(VideoInterface);

    public:
        VideoInterface(sf::Service&& service)
            : sf::Service(std::forward<sf::Service>(service)) { }

        template <util::TemplateString Name = "vi:u">
        static ApplicationDisplayService* initialize() {
            sf::Service mainService = HK_UNWRAP(sm::ServiceManager::instance()->getServiceHandle<Name>());
            createInstance(move(mainService));

            ApplicationDisplayService::createInstance(instance()->getDisplayService());

            return ApplicationDisplayService::instance();
        }

        sf::Service getDisplayService() {
            return HK_UNWRAP(invokeRequest(sf::Request(this, 0), [this](sf::Response& response){ return response.nextSubservice(this); }));
        }

    };

} // namespace hk::vi
