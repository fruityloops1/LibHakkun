#pragma once

#include "hk/ValueOrResult.h"
#include "hk/services/am/appType.h"
#include "hk/services/am/detail/applicationFunctions.h"
#include "hk/services/am/detail/audioController.h"
#include "hk/services/am/detail/commonStateGetter.h"
#include "hk/services/am/detail/debugFunctions.h"
#include "hk/services/am/detail/displayController.h"
#include "hk/services/am/detail/libraryAppletCreator.h"
#include "hk/services/am/detail/selfController.h"
#include "hk/services/am/detail/windowController.h"
#include "hk/sf/sf.h"
#include "hk/sf/utils.h"
#include "hk/util/Singleton.h"

namespace hk::am::detail {

    class ApplicationProxy : public sf::Service {
        HK_SINGLETON(ApplicationProxy);

        template <typename T, u32 command>
        ValueOrResult<T*> initializeSubservice() {
            return sf::invokeSimple<sf::Service>(this, 0).map([](sf::Service service) {
                T::createInstance(move(service));
                return T::instance();
            });
        }

    public:
        ApplicationProxy(sf::Service&& service, AppType type)
            : sf::Service(forward<sf::Service>(service)) { }

        ValueOrResult<ApplicationFunctions*> initializeApplicationFunctions() {
            return initializeSubservice<ApplicationFunctions, 20>();
        }

        ValueOrResult<AudioController*> initializeAudioController() {
            return initializeSubservice<AudioController, 3>();
        }

        ValueOrResult<CommonStateGetter*> initializeCommonStateGetter() {
            return initializeSubservice<CommonStateGetter, 0>();
        }

        ValueOrResult<DebugFunctions*> initializeDebugFunctions() {
            return initializeSubservice<DebugFunctions, 1000>();
        }

        ValueOrResult<DisplayController*> initializeDisplayController() {
            return initializeSubservice<DisplayController, 4>();
        }

        ValueOrResult<LibraryAppletCreator*> initializeLibraryAppletCreator() {
            return initializeSubservice<LibraryAppletCreator, 11>();
        }

        ValueOrResult<SelfController*> initializeSelfController() {
            return initializeSubservice<SelfController, 1>();
        }

        ValueOrResult<WindowController*> initializeWindowController() {
            return initializeSubservice<WindowController, 2>();
        }
    };

} // namespace hk::am::detail
