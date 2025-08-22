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
#include <utility>

namespace hk::socket {

    class Socket : sf::Service {
        sf::Service monitorService;
        HK_SINGLETON(Socket);

    public:
        Socket(sf::Service&& service, sf::Service&& monitor)
            : sf::Service(std::forward<sf::Service>(service))
            , monitorService(std::forward<sf::Service>(monitor)) { }

        template <util::TemplateString name = "bsd:u">
        static Socket* initialize(const ServiceConfig& config, const std::span<u8> socketBuffer) {
            auto monitor = startMonitoring<name>();
            createInstance(
                sm::ServiceManager::instance()->getServiceHandle<name>(),
                std::move(monitor)
            );
            instance()->registerClient(config, socketBuffer);
            return instance();
        }

        template <util::TemplateString name>
        static sf::Service startMonitoring() {
            auto monitorService = HK_UNWRAP(sm::ServiceManager::instance()->getServiceHandle<name>());
            auto input = u64(0);
            auto request = sf::Request(1, &input);
            request.setSendPid();
            HK_ABORT_UNLESS_R(monitorService.invokeRequest(std::move(request)));
            return monitorService;
        }

        void registerClient(const ServiceConfig& config, const std::span<u8> socketBuffer) {
            Handle handle;
            HK_ABORT_UNLESS_R(svc::CreateTransferMemory(&handle, ptr(socketBuffer.data()), socketBuffer.size_bytes(), svc::MemoryPermission_None));
            std::array<u8, 0x30> input = sf::packInput(config, u64(0xDEADBEEFCAFEBABE), u64(socketBuffer.size_bytes()));
            auto request = sf::Request(0, &input);
            request.addCopyHandle(handle);
            request.setSendPid();
            HK_ABORT_UNLESS_R(invokeRequest(std::move(request)));
        }

        struct Ret {
            s32 returnValue;
            s32 errno;
        };

        Ret socket(AddressFamily domain, Type type, Protocol protocol) {
            std::array<u8, 12> input = sf::packInput(u32(domain), u32(type), u32(protocol));
            return HK_UNWRAP(invokeRequest(sf::Request(2, &input), sf::simpleDataHandler<Ret>()));
        }

        template <typename A, typename B>
            requires(std::is_convertible<A*, SocketAddr*>::value)
        Ret bind(s32 fd, const A& address);

        template <typename A, typename B>
            requires(std::is_convertible<A*, SocketAddr*>::value)
        Ret connect(s32 fd, const A& address) {
        }

        template <typename B>
        // requires(std::is_convertible_v<A*, SocketAddr*>)
        Ret sendTo(s32 fd, std::span<const B> data, s32 flags, const SocketAddrIpv4& address) {
            auto input = sf::packInput(fd, flags);
            diag::debugLog("sized %x %x %x %x", fd, flags, input.size(), address);
            auto request = sf::Request(11, &input);
            request.addInAutoselect(data.data(), data.size_bytes());
            request.addInAutoselect(&address, sizeof(SocketAddrIpv4));
            request.enableDebug(true, true);
            return HK_UNWRAP(invokeRequest(std::move(request), sf::simpleDataHandler<Ret>()));
        }
    };

}
