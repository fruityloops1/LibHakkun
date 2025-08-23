#pragma once

#include "hk/services/sm.h"
#include "hk/sf/sf.h"
#include "hk/util/Singleton.h"

namespace hk::pm {

    class ProcessManagerForDebugMonitor : public sf::Service {
        HK_SINGLETON(ProcessManagerForDebugMonitor);

    public:
        static ProcessManagerForDebugMonitor* connect() {
            auto service = HK_UNWRAP(sm::ServiceManager::instance()->getServiceHandle<"pm:dmnt">());
            createInstance(std::move(service));
            return instance();
        }

        u64 getApplicationProcessId() {
            auto request = sf::Request(4);
            request.enableDebug(true, true);
            return invokeRequest(std::move(request), [](sf::Response& response) {
                HK_ASSERT(response.data.size_bytes() >= 8);
                return *cast<u64*>(response.data.data());
            });
        }
    };

} // namespace hk::pm
