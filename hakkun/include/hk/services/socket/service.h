#pragma once

#include "hk/ValueOrResult.h"
#include "hk/diag/diag.h"
#include "hk/services/sm.h"
#include "hk/services/socket/address.h"
#include "hk/services/socket/config.h"
#include "hk/sf/sf.h"
#include "hk/sf/utils.h"
#include "hk/svc/api.h"
#include "hk/svc/types.h"
#include "hk/types.h"
#include "hk/util/Singleton.h"
#include "hk/util/TemplateString.h"
#include <type_traits>

namespace hk::socket {

    class Socket : sf::Service {
        HK_SINGLETON(Socket);

    public:
        Socket(sf::Service&& service)
            : sf::Service(std::forward<sf::Service>(service)) { }

        template <util::TemplateString name = "bsd:u">
        static Socket* initialize() {
            createInstance(sm::ServiceManager::instance()->getServiceHandle<name>());
            return instance();
        }

        void registerClient(const ServiceConfig& config, const u8* socketBuffer, size socketBufferSize) {
            Handle handle;
            diag::debugLog("size %x", socketBufferSize);
            HK_ABORT_UNLESS_R(svc::CreateTransferMemory(&handle, ptr(socketBuffer), socketBufferSize, svc::MemoryPermission_None));
            auto input = sf::packInput(config, u64(0), u64(socketBufferSize));
            auto request = sf::Request(0, &input);
            request.addCopyHandle(handle);
            request.setSendPid();
            // request.debugAbortAfterRequest();
            request.enableDebug(true, true);
            HK_ABORT_UNLESS_R(invokeRequest(std::move(request)));
        }

        struct Ret {
            s32 returnValue;
            s32 errno;
        };

        Ret socket(AddressFamily domain, Type type, Protocol protocol) {
            auto input = sf::packInput(domain, type, protocol);
            return HK_UNWRAP(invokeRequest(sf::Request(2, &input), sf::simpleDataHandler<Ret>()));
        }

        template <typename A, typename B>
            requires(std::is_convertible<A*, SocketAddr*>::value)
        Ret bind(s32 fd, const A& address);

        template <typename A, typename B>
            requires(std::is_convertible<A*, SocketAddr*>::value)
        Ret connect(s32 fd, const A& address) {
        }

        template <typename A, typename B>
            requires(std::is_convertible<A*, SocketAddr*>::value)
        Ret sendTo(s32 fd, std::span<const B> data, s32 flags, const A& address) {
            auto input = sf::packInput(fd, flags);
            auto request = sf::Request(11, &input);
            request.addInAutoselect(data.data(), data.size_bytes());
            request.addInAutoselect(&address, sizeof(A));
            return HK_UNWRAP(invokeRequest(std::move(request), sf::simpleDataHandler<Ret>()));
        }
    };

}
