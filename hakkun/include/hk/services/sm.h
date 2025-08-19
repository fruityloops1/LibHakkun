#pragma once

#include "hk/Result.h"
#include "hk/ValueOrResult.h"
#include "hk/diag/diag.h"
#include "hk/sf/sf.h"
#include "hk/svc/api.h"
#include "hk/util/Singleton.h"
#include "hk/util/Storage.h"
#include "hk/util/TemplateString.h"
#include <__expected/unexpect.h>
#include <cstring>
#include <expected>

namespace hk::sm {
    class ServiceManager : sf::Service {
        HK_SINGLETON(ServiceManager);

    public:
        ServiceManager(Handle session) : sf::Service(session) {}
        static ServiceManager* connect();

        Result registerClient() {
            auto request = sf::Request(0);
            request.setSendPid();
            return invokeRequest(std::move(request), [](sf::Response&) {
                return 1;
            });
        }

        template <util::TemplateString name>
        ValueOrResult<sf::Service> getServiceHandle() {
            static_assert(sizeof(name) <= 9, "name can only be eight bytes or less");

            char nameBuf[9] = {};
            std::memcpy(nameBuf, name.value, sizeof(name.value));
            return invokeRequest(sf::Request(1, nameBuf, 8), [](sf::Response& response) {
                return sf::Service::fromHandle(response.hipcMoveHandles[0]);
            });
        }
    };
}
