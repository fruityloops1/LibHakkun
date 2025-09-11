#pragma once

#include "hk/Result.h"
#include "hk/ValueOrResult.h"
#include "hk/services/am/detail/applicationProxy.h"
#include "hk/services/sm.h"
#include "hk/sf/sf.h"
#include "hk/sf/utils.h"
#include "hk/svc/api.h"
#include "hk/svc/types.h"
#include "hk/types.h"
#include "hk/util/Singleton.h"
#include "hk/util/TemplateString.h"

namespace hk::am::detail {
    enum class AppType {
        System = 0,
        Application = 1,
    };

    class AppletManager : public sf::Service {
        HK_SINGLETON(AppletManager);

    public:
        AppletManager(sf::Service&& service)
            : sf::Service(forward<sf::Service>(service)) { }

        template <AppType Type>
        static Result initialize() {
            constexpr util::TemplateString<9> name = Type == AppType::Application ? "appletOE" : "appletAE";
            sf::Service service = HK_TRY(sm::ServiceManager::instance()->getServiceHandle<name>());
            HK_TRY(service.convertToDomain());

            // OpenApplicationProxy
            auto input = sf::packInput(u64(0), u64(0));
            auto request = sf::Request(&service, 0, &input);
            request.setSendPid();
            request.addCopyHandle(svc::CurrentProcess);

            do {
                ValueOrResult<sf::Service> result = service.invokeRequest(move(request), sf::subserviceExtractor(&service));
                if (result.hasValue()) {
                    ApplicationProxy::createInstance(result);
                    return ResultSuccess();
                }

                // am switchbrew docs say that apps wait until OpenApplicationProxy stops returning 0x19280
                if (Result(result).getValue() != 0x19280)
                    HK_TRY(result);

                svc::SleepThread(10_ms);
            } while (true);
        }
    };
}
