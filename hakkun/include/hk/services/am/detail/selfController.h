#pragma once

#include "hk/ValueOrResult.h"
#include "hk/services/vi/service.h"
#include "hk/sf/sf.h"
#include "hk/sf/utils.h"
#include "hk/util/Singleton.h"

namespace hk::am::detail {

    class SelfController : public sf::Service {
        HK_SINGLETON(SelfController);

    public:
        SelfController(sf::Service&& service)
            : sf::Service(forward<sf::Service>(service)) { }

        // does not work for AppType::Application
        hk_noreturn void exit() {
            HK_ABORT_UNLESS_R(sf::invokeSimple<void>(this, 0));
            while (true)
                hk::svc::SleepThread(std::numeric_limits<s64>::max());
        }

        Result setFocusHandlingMode(bool in1, bool in2, bool in3) {
            return sf::invokeSimple(this, u8(in1), u8(in2), u8(in3));
        }

        Result setOutOfFocusSuspendingEnabled(bool enabled) {
            return sf::invokeSimple(this, 16, enabled);
        }

        ValueOrResult<vi::LayerId> createManagedDisplayLayer() {
            return sf::invokeSimple<vi::LayerId>(this, 40);
        }
    };

} // namespace hk::am::detail
