#pragma once

#include "hk/ValueOrResult.h"
#include "hk/services/sm.h"
#include "hk/sf/sf.h"
#include "hk/sf/utils.h"
#include "hk/types.h"
#include "hk/util/Singleton.h"

namespace hk::pm {

    class ProcessManagerForDebugMonitor : public sf::Service {
        HK_SINGLETON(ProcessManagerForDebugMonitor);

    public:
        static ValueOrResult<ProcessManagerForDebugMonitor*> initialize() {
            auto service = HK_TRY(sm::ServiceManager::instance()->getServiceHandle<"pm:dmnt">());
            createInstance(move(service));
            return instance();
        }

        ValueOrResult<size> getJitDebugProcessIdList(std::span<u64> debugProcesses) {
            auto request = sf::Request(this, 1);
            request.addOutMapAlias(debugProcesses.data(), debugProcesses.size_bytes());
            HK_TRY(invokeRequest(move(request)));
            return debugProcesses.size();
        }

        Result startProcess(u64 pid) {
            return sf::invokeSimple(*this, 1, &pid);
        }

        ValueOrResult<u64> getProcessId(u64 titleId) {
            return sf::invokeSimple<u64>(*this, 2, titleId);
        }

        ValueOrResult<Handle> hookToCreateProcess(u64 titleId) {
            return invokeSimple<Handle>(*this, 3, &titleId);
        }

        ValueOrResult<u64> getApplicationProcessId() {
            return sf::invokeSimple<u64>(*this, 4);
        }

        ValueOrResult<Handle> hookToCreateApplicationProcess() {
            return invokeSimple<Handle>(*this, 5);
        }

        Result clearHook(u32 bitflags) {
            return sf::invokeSimple(*this, 6, &bitflags);
        }

        ValueOrResult<u64> getProgramId(u64 pid) {
            return sf::invokeSimple<u64>(*this, 7, pid);
        }
    };

} // namespace hk::pm
