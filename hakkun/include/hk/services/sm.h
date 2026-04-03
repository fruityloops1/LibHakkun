#pragma once

#include "hk/Result.h"
#include "hk/ValueOrResult.h"
#include "hk/sf/sf.h"
#include "hk/sf/tipc.h"
#include "hk/svc/api.h"
#include "hk/types.h"
#include "hk/util/Singleton.h"
#include "hk/util/TemplateString.h"
#include <cstring>

namespace hk::sm {

    class ServiceManager : Handle {
        HK_SINGLETON(ServiceManager);

    public:
        ServiceManager(Handle session)
            : Handle(session) { }

        ~ServiceManager() {
            svc::CloseHandle(*this);
        }

        static ValueOrResult<ServiceManager*> initialize() {
            Handle outHandle = HK_TRY(svc::ConnectToNamedPort("sm:"));
            createInstance(outHandle);
            HK_TRY(instance()->registerClient());
            return instance();
        }

        Result registerClient() {
            auto request = tipc::Request(0);
            request.setSendPid();
            return request.invoke(*this);
        }

        template <util::TemplateString Name>
        ValueOrResult<sf::Service> getServiceHandle() {
            static_assert(sizeof(Name) <= 9, "name can only be eight characters or less");

            char nameBuf[9] = {};
            std::memcpy(nameBuf, Name.value, sizeof(Name));
            auto request = tipc::Request(1, std::span(nameBuf, 8));

            return sf::Service::fromHandle(HK_TRY(request.invoke(*this)).takeNextHandle());
        }

        template <util::TemplateString Name>
        Result amsWaitService() {
            char nameBuf[9] = {};
            std::memcpy(nameBuf, Name.value, sizeof(Name));
            auto request = tipc::Request(65101, std::span(nameBuf, 8));

            return Result(request.invoke(*this));
        }
    };

} // namespace hk::sm
