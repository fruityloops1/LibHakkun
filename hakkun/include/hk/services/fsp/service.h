#pragma once

#include "hk/ValueOrResult.h"
#include "hk/services/fsp/fileSystem.h"
#include "hk/services/sm.h"
#include "hk/sf/sf.h"
#include "hk/sf/utils.h"
#include "hk/types.h"
#include "hk/util/Singleton.h"

namespace hk::fsp {
    class FileSystemProxy : public sf::Service {
        HK_SINGLETON(FileSystemProxy);

    public:
        FileSystemProxy(sf::Service&& service)
            : sf::Service(forward<sf::Service>(service)) { }

        static ValueOrResult<FileSystemProxy*> initialize() {
            sf::Service service = HK_TRY(sm::ServiceManager::instance()->getServiceHandle<"fsp-srv">());
            service.convertToDomain();
            createInstance(move(service));
            instance()->setCurrentProcess();
            return instance();
        }

        Result setCurrentProcess() {
            auto input = u64(0);
            auto request = sf::Request(this, 1, &input);
            request.setSendPid();
            return invokeRequest(move(request));
        }

        ValueOrResult<IFileSystem> openSdCardFileSystem() {
            auto service = HK_TRY(sf::invokeSimple<sf::Service>(this, 18));

            return IFileSystem(move(service));
        }
    };
} // namespace hk::fsp
