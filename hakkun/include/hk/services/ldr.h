#pragma once

#include "hk/ValueOrResult.h"
#include "hk/container/StringView.h"
#include "hk/services/sm.h"
#include "hk/sf/sf.h"
#include "hk/sf/utils.h"
#include "hk/util/Singleton.h"

namespace hk::ldr {
    struct ProgramArgumentHeader {
        u32 allocated_size;
        u32 arguments_size; // size of arguments string
        u8 reserved[0x18];
        char arguments[]; // space delimited arguments string, double quotes encapsulate argumentss with spaces
    };

    class LoaderForShell : public sf::Service {
        HK_SINGLETON(LoaderForShell);

    public:
        LoaderForShell(sf::Service&& service)
            : sf::Service(forward<sf::Service>(service)) { }

        static ValueOrResult<LoaderForShell*> initialize() {
            sf::Service service = HK_TRY(sm::ServiceManager::instance()->getServiceHandle<"ldr:shel">());
            createInstance(move(service));
            return instance();
        }

        Result setProgramArgument(u64 programId, StringView args) {
            auto request = sf::Request(this, 0, &programId);
            request.addInPointer<char>(args);
            return invokeRequest(move(request));
        }

        Result flushArguments() {
            return sf::invokeSimple(this, 1);
        }
    };
} // namespace hk::ldr
