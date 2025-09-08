#pragma once

#include "hk/services/am/detail/commonStateGetter.h"
#include "hk/sf/sf.h"
#include "hk/sf/utils.h"
#include "hk/util/Singleton.h"
namespace hk::am::detail {
    class ApplicationProxy : public sf::Service {
        HK_SINGLETON(ApplicationProxy);

    public:
        ApplicationProxy(sf::Service&& service)
            : sf::Service(forward<sf::Service>(service)) { }

        void initializeCommonStateGetter() {
            CommonStateGetter::createInstance(HK_UNWRAP(sf::invokeSimple<sf::Service>(*this, 0)));
        }

        class SelfController* initalizeSelfController() {
            HK_TODO("build SelfController");
        }
    };
}
