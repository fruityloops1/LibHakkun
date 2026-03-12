#pragma once

#include "hk/ValueOrResult.h"
#include "hk/services/sm.h"
#include "hk/sf/utils.h"
#include "hk/util/Singleton.h"

namespace hk::apm {
    enum class PerformanceMode {
        Invalid = -1,
        Normal = 0,
        Boost = 1,
    };

    // sourced from https://switchbrew.org/wiki/PTM_services#PerformanceConfiguration
    enum class PerformanceConfiguration : u32 {
        // CPU clock (MHz): 0 	    GPU clock (MHz): 0 	    Memory clock (MHz): 0
        Invalid = 0x00000000,
        // CPU clock (MHz): 1020.0 	GPU clock (MHz): 384.0 	Memory clock (MHz): 1600.0
        Cpu1020MhzGpu384Mhz = 0x00010000,
        // CPU clock (MHz): 1020.0 	GPU clock (MHz): 768.0 	Memory clock (MHz): 1600.0
        Cpu1020MhzGpu768Mhz = 0x00010001,
        // CPU clock (MHz): 1224.0 	GPU clock (MHz): 691.2 	Memory clock (MHz): 1600.0
        Cpu1224MhzGpu691Mhz = 0x00010002,
        // CPU clock (MHz): 1020.0 	GPU clock (MHz): 230.4 	Memory clock (MHz): 1600.0
        Cpu1020MhzGpu230Mhz = 0x00020000,
        // CPU clock (MHz): 1020.0 	GPU clock (MHz): 307.2 	Memory clock (MHz): 1600.0
        Cpu1020MhzGpu307Mhz = 0x00020001,
        // CPU clock (MHz): 1224.0 	GPU clock (MHz): 230.4 	Memory clock (MHz): 1600.0
        Cpu1224MhzGpu230Mhz = 0x00020002,
        // CPU clock (MHz): 1020.0 	GPU clock (MHz): 307.2 	Memory clock (MHz): 1331.2
        Cpu1020MhzGpu307MhzEmc1331Mhz = 0x00020003,
        // CPU clock (MHz): 1020.0 	GPU clock (MHz): 384.0 	Memory clock (MHz): 1331.2
        Cpu1020MhzGpu384MhzEmc1331Mhz = 0x00020004,
        // CPU clock (MHz): 1020.0 	GPU clock (MHz): 307.2 	Memory clock (MHz): 1065.6
        Cpu1020MhzGpu307MhzEmc1065Mhz = 0x00020005,
        // CPU clock (MHz): 1020.0 	GPU clock (MHz): 384.0 	Memory clock (MHz): 1065.6
        Cpu1020MhzGpu384MhzEmc1065Mhz = 0x00020006,
        // CPU clock (MHz): 1020.0 	GPU clock (MHz): 460.8 	Memory clock (MHz): 1600.0
        Cpu1020MhzGpu460MhzEmc1600Mhz = 0x92220007,
        // CPU clock (MHz): 1020.0 	GPU clock (MHz): 460.8 	Memory clock (MHz): 1331.2
        Cpu1020MhzGpu460MhzEmc1331Mhz = 0x92220008,
        // CPU clock (MHz): 1785.0 	GPU clock (MHz): 76.8 	Memory clock (MHz): 1600.0
        Cpu1785MhzGpu76MhzEmc1600Mhz = 0x92220009,
        // CPU clock (MHz): 1785.0 	GPU clock (MHz): 76.8 	Memory clock (MHz): 1331.2
        Cpu1785MhzGpu76MhzEmc1331Mhz = 0x9222000A,
        // CPU clock (MHz): 1020.0 	GPU clock (MHz): 76.8 	Memory clock (MHz): 1600.0
        Cpu1020MhzGpu76MhzEmc1600Mhz = 0x9222000B,
        // CPU clock (MHz): 1020.0 	GPU clock (MHz): 76.8 	Memory clock (MHz): 1331.2
        Cpu1020MhzGpu76MhzEmc1331Mhz = 0x9222000C,
    };

    class ApmSession : public sf::Service {
        HK_SINGLETON(ApmSession)
    public:
        ApmSession(sf::Service&& service)
            : sf::Service(forward<sf::Service>(service)) { }

        Result setPerformanceConfiguration(PerformanceMode mode, PerformanceConfiguration config) {
            return sf::invokeSimple(this, 0, mode, config);
        }

        ValueOrResult<PerformanceConfiguration> getPerformanceConfiguration(PerformanceMode mode, PerformanceConfiguration config) {
            return sf::invokeSimple<PerformanceConfiguration>(this, 0, mode);
        }
    };

    class ApmManager : public sf::Service {
        HK_SINGLETON(ApmManager)
    public:
        ApmManager(sf::Service&& service)
            : sf::Service(forward<sf::Service>(service)) { }

        static ValueOrResult<ApmManager*> initialize() {
            auto service = HK_TRY(sm::ServiceManager::instance()->getServiceHandle<"apm">());
            service.convertToDomain();
            createInstance(move(service));
            return instance();
        }

        ValueOrResult<ApmSession*> openSession() {
            return sf::invokeSimple<sf::Service>(this, 0).map([](sf::Service service) {
                ApmSession::createInstance(move(service));
                return ApmSession::instance();
            });
        }

        ValueOrResult<PerformanceMode> getPerformanceMode() {
            return sf::invokeSimple<PerformanceMode>(this, 1);
        }
    };

    inline Result initialize() {
        auto manager = HK_TRY(ApmManager::initialize());
        HK_TRY(manager->openSession());
        return ResultSuccess();
    }
} // namespace hk::apm
