#pragma once

#include "hk/ValueOrResult.h"
#include "hk/services/am/appType.h"
#include "hk/services/am/detail/applicationProxy.h"
#include "hk/services/sm.h"
#include "hk/sf/sf.h"
#include "hk/util/Singleton.h"
#include "hk/util/TemplateString.h"

namespace hk::am::detail {

    class AppletManager : public sf::Service {
        HK_SINGLETON(AppletManager);
        AppType type;

    public:
        AppletManager(sf::Service&& service, AppType type)
            : sf::Service(forward<sf::Service>(service))
            , type(type) { }

        template <AppType Type>
        static ValueOrResult<AppletManager*> initialize() {
            constexpr util::TemplateString<9> name = Type == AppType::Application ? "appletOE" : "appletAE";
            sf::Service service = HK_TRY(sm::ServiceManager::instance()->getServiceHandle<name>());
            HK_TRY(service.convertToDomain());

            createInstance(move(service), Type);
            return instance();
        }

        ValueOrResult<ApplicationProxy*> initializeApplicationProxy() {
            // OpenApplicationProxy
            std::array<u32, size_t(AppType::_Count)> ids = { 0, 100, 200, 300, 350 };

            do {
                auto input = sf::packInput(u64(0));
                auto request = sf::Request(this, ids[size(type)], &input);
                request.setSendPid();
                request.addCopyHandle(svc::CurrentProcess);
                ValueOrResult<sf::Service> result = invokeRequest(move(request), sf::subserviceExtractor(this));

                if (result.hasValue()) {
                    ApplicationProxy::createInstance(result, type);
                    return ApplicationProxy::instance();
                }

                // am switchbrew docs say that apps wait until OpenApplicationProxy stops returning 0x19280
                if (Result(result).getValue() != 0x19280)
                    HK_TRY(result);

                svc::SleepThread(10_ms);
            } while (true);
        }
    };

} // namespace hk::am::detail
