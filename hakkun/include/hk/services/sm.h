#pragma once

#include "hk/Result.h"
#include "hk/ValueOrResult.h"
#include "hk/sf/sf.h"
#include "hk/sf/utils.h"
#include "hk/svc/api.h"
#include "hk/types.h"
#include "hk/util/Singleton.h"
#include "hk/util/TemplateString.h"
#include <cstring>

namespace hk::sm {

    class ServiceManager : sf::Service {
        HK_SINGLETON(ServiceManager);

    public:
        ServiceManager(Handle session)
            : sf::Service(session) { }
        static ValueOrResult<ServiceManager*> initialize() {
            Handle outHandle = HK_TRY(svc::ConnectToNamedPort("sm:"));

            createInstance(outHandle);
            return instance();
        }

        Result registerClient() {
            u64 placeholderPid = 0;

            auto request = sf::Request(this, 0, &placeholderPid);
            request.setSendPid();
            return invokeRequest(move(request));
        }

        template <util::TemplateString Name>
        ValueOrResult<sf::Service> getServiceHandle() {
            static_assert(sizeof(Name) <= 9, "name can only be eight characters or less");

            char nameBuf[9] = {};
            std::memcpy(nameBuf, Name.value, sizeof(Name));
            auto request = sf::Request(this, 1, Span(nameBuf, 8));
            return invokeRequest(move(request), [](sf::Response& response) {
                return sf::Service::fromHandle(response.hipcMoveHandles[0]);
            });
        }

        template <util::TemplateString Name>
        ValueOrResult<Handle> registerService(bool isLight, s32 maxSessions) {
            static_assert(sizeof(Name) <= 9, "name can only be eight characters or less");

            hk::Array<u8, 8> data = {};
            std::memcpy(data.data(), Name.value, util::min(sizeof(Name), 8));
            auto input = sf::packInput(data, u8(isLight), maxSessions);
            auto request = sf::Request(this, 2, &input);
            return invokeRequest(move(request), sf::moveHandleExtractor());
        }
    };

} // namespace hk::sm
