#pragma once

#include "hk/ValueOrResult.h"
#include "hk/services/ncm/programLocation.h"
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

        ValueOrResult<size> getJitDebugProcessIdList(Span<u64> debugProcesses) {
            auto request = sf::Request(this, 1);
            request.addOutMapAlias(debugProcesses.data(), debugProcesses.size_bytes());
            HK_TRY(invokeRequest(move(request)));
            return debugProcesses.size();
        }

        Result startProcess(u64 pid) {
            return sf::invokeSimple(this, 1, &pid);
        }

        ValueOrResult<u64> getProcessId(u64 titleId) {
            return sf::invokeSimple<u64>(this, 2, titleId);
        }

        ValueOrResult<Handle> hookToCreateProcess(u64 titleId) {
            return invokeSimple<Handle>(this, 3, &titleId);
        }

        ValueOrResult<u64> getApplicationProcessId() {
            return sf::invokeSimple<u64>(this, 4);
        }

        ValueOrResult<Handle> hookToCreateApplicationProcess() {
            return invokeSimple<Handle>(this, 5);
        }

        Result clearHook(u32 bitflags) {
            return sf::invokeSimple(this, 6, &bitflags);
        }

        ValueOrResult<u64> getProgramId(u64 pid) {
            return sf::invokeSimple<u64>(this, 7, pid);
        }
    };

    enum LaunchFlags {
        LaunchFlags_None = 0,
        LaunchFlags_SignalOnExit = bit(0),
        LaunchFlags_SignalOnStart = bit(1),
        LaunchFlags_SignalOnException = bit(2),
        LaunchFlags_SignalOnDebugEvent = bit(3),
        LaunchFlags_StartSuspended = bit(4),
        LaunchFlags_DisableAslr = bit(5)
    };

    class ProcessManagerForShell : public sf::Service {
        HK_SINGLETON(ProcessManagerForShell);

    public:
        ProcessManagerForShell(sf::Service&& service)
            : sf::Service(forward<sf::Service>(service)) { }

        static ValueOrResult<ProcessManagerForShell*> initialize() {
            sf::Service service = HK_TRY(sm::ServiceManager::instance()->getServiceHandle<"pm:shell">());
            createInstance(move(service));
            return instance();
        }

        Result terminateProcess(u64 processId) {
            return sf::invokeSimple(this, 1, processId);
        }

        Result terminateProgram(u64 programId) {
            return sf::invokeSimple(this, 2, programId);
        }

        ValueOrResult<u64> launchProgram(LaunchFlags launchFlags, ncm::ProgramLocation location) {
            return sf::invokeSimple<u64>(this, 0, launchFlags, location);
        }
    };

} // namespace hk::pm
