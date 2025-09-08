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

        Result startProcess(u64 pid) {
            return sf::invokeSimple(*this, 1, &pid);
        }

        ValueOrResult<u64> getProcessId(u64 titleId) {
            return sf::invokeSimple<u64>(*this, 2, titleId);
        }

        ValueOrResult<Handle> hookToCreateProcess(u64 titleId) {
            auto request = sf::Request(this, 3, &titleId);
            return invokeRequest(move(request), sf::handleExtractor());
        }

        ValueOrResult<u64> getApplicationProcessId() {
            return sf::invokeSimple<u64>(*this, 4);
        }

        ValueOrResult<Handle> hookToCreateApplicationProcess() {
            auto request = sf::Request(this, 5);
            return invokeRequest(move(request), sf::handleExtractor());
        }

        void clearHook(u32 bitflags) {
            HK_ABORT_UNLESS_R(sf::invokeSimple(*this, 6, &bitflags));
        }

        ValueOrResult<u64> getProgramId(u64 pid) {
            return sf::invokeSimple<u64>(*this, 7, pid);
        }
    };

} // namespace hk::pm
