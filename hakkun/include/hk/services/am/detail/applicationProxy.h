#pragma once

#include "hk/ValueOrResult.h"
#include "hk/services/am/detail/commonStateGetter.h"
#include "hk/services/am/detail/selfController.h"
#include "hk/services/am/detail/windowController.h"
#include "hk/sf/sf.h"
#include "hk/sf/utils.h"
#include "hk/util/Singleton.h"
namespace hk::am::detail {
    class ApplicationProxy : public sf::Service {
        HK_SINGLETON(ApplicationProxy);

        template<typename T>
        ValueOrResult<T*> initializeSubservice() {
            return sf::invokeSimple<sf::Service>(*this, 0).map([](sf::Service service) {
                T::createInstance(move(service));
                return T::instance();
            });
        }
    public:
        ApplicationProxy(sf::Service&& service)
            : sf::Service(forward<sf::Service>(service)) { }

        ValueOrResult<CommonStateGetter*> initializeCommonStateGetter() {
            return initializeSubservice<CommonStateGetter>();
        }

        ValueOrResult<SelfController*> initializeSelfController() {
            return initializeSubservice<SelfController>();
        }

        ValueOrResult<WindowController*> initializeWindowController() {
            return initializeSubservice<WindowController>();
        }
    };
}
