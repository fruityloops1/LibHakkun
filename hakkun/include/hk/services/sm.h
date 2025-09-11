#pragma once

#include "hk/Result.h"
#include "hk/ValueOrResult.h"
#include "hk/sf/sf.h"
#include "hk/svc/api.h"
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
            return invokeRequest(sf::Request(this, 1, std::span(nameBuf, 8)), [](sf::Response& response) {
                return sf::Service::fromHandle(response.hipcMoveHandles[0]);
            });
        }
    };

} // namespace hk::sm
