#pragma once

#include "hk/services/sm.h"
#include "hk/sf/sf.h"
#include "hk/sf/utils.h"
#include "hk/types.h"
#include "hk/util/Singleton.h"

namespace hk::pm {

    class ProcessManagerForDebugMonitor : public sf::Service {
        HK_SINGLETON(ProcessManagerForDebugMonitor);

    public:
        static ProcessManagerForDebugMonitor* initialize() {
            auto service = HK_UNWRAP(sm::ServiceManager::instance()->getServiceHandle<"pm:dmnt">());
            createInstance(move(service));
            return instance();
        }

        size getJitDebugProcessIdList(std::span<u64> debugProcesses) {
            auto request = sf::Request(this, 1);
            request.addOutMapAlias(debugProcesses.data(), debugProcesses.size_bytes());
            HK_ABORT_UNLESS_R(invokeRequest(move(request)));
            return debugProcesses.size();
        }

        void startProcess(u64 pid) {
            HK_ABORT_UNLESS_R(invokeRequest(sf::Request(this, 1, &pid)));
        }

        u64 getProcessId(u64 titleId) {
            return HK_UNWRAP(sf::invokeSimple<u64>(*this, 2, titleId));
        }

        Handle hookToCreateProcess(u64 titleId) {
            auto request = sf::Request(this, 3, &titleId);
            return HK_UNWRAP(invokeRequest(move(request), [](sf::Response& response){ return response.nextCopyHandle(); }));
        }

        u64 getApplicationProcessId() {
            return HK_UNWRAP(sf::invokeSimple<u64>(*this, 4));
        }
    
        Handle hookToCreateApplicationProcess() {
            auto request = sf::Request(this, 5);
            return HK_UNWRAP(invokeRequest(move(request), [](sf::Response& response){ return response.nextCopyHandle(); }));
        }

        void clearHook(u32 bitflags) {
            HK_ABORT_UNLESS_R(invokeRequest(sf::Request(this, 6, &bitflags)));
        }

        u64 getProgramId(u64 pid) {
            return HK_UNWRAP(sf::invokeSimple<u64>(*this, 7, pid));
        }
    };

} // namespace hk::pm
