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
#include "hk/util/Tuple.h"
#include <type_traits>
#include <utility>

namespace hk::socket {

    class Socket : sf::Service {
        sf::Service mMonitorService;
        HK_SINGLETON(Socket);

    public:
        Socket(sf::Service&& service, sf::Service&& monitor)
            : sf::Service(std::forward<sf::Service>(service))
            , mMonitorService(std::forward<sf::Service>(monitor)) { }

        template <util::TemplateString Name = "bsd:u">
        static Socket* initialize(const ServiceConfig& config, const std::span<u8> socketBuffer) {
            sf::Service mainService = HK_UNWRAP(sm::ServiceManager::instance()->getServiceHandle<Name>()),
                        monitorService = HK_UNWRAP(sm::ServiceManager::instance()->getServiceHandle<Name>());
            createInstance(move(mainService), move(monitorService));
            instance()->registerClient(config, socketBuffer);
            instance()->startMonitoring();
            return instance();
        }

        void startMonitoring() {
            auto input = u64(0);
            auto request = sf::Request(&mMonitorService, 1, &input);
            request.setSendPid();
            HK_ABORT_UNLESS_R(mMonitorService.invokeRequest(move(request)));
        }

        void registerClient(const ServiceConfig& config, const std::span<u8> socketBuffer) {
            Handle handle;
            HK_ABORT_UNLESS_R(svc::CreateTransferMemory(&handle, ptr(socketBuffer.data()), socketBuffer.size_bytes(), svc::MemoryPermission_None));
            std::array<u8, 0x30> input = sf::packInput(config, u64(0), u64(socketBuffer.size_bytes()));
            auto request = sf::Request(this, 0, &input);
            request.addCopyHandle(handle);
            request.setSendPid();
            HK_ABORT_UNLESS_R(invokeRequest(move(request)));
        }

        using Ret = Tuple<s32, s32>;

    private:
        template <s32 Id, typename A>
            requires(std::is_convertible<A*, SocketAddr*>::value)
        Ret inFdInAddress(s32 fd, const A& address) {
            auto request = sf::Request(this, Id, &fd);

            request.addInAutoselect(&address, sizeof(A));
            return HK_UNWRAP(invokeRequest(move(request), sf::inlineDataExtractor<Ret>()));
        }

        template <s32 Id, typename A>
            requires(std::is_convertible<A*, SocketAddr*>::value)
        Ret inFdOutAddress(s32 fd, A* address) {
            auto input = sf::packInput(fd, sizeof(A));
            auto request = sf::Request(this, Id, &input);

            request.addOutAutoselect(address, sizeof(A));
            return HK_UNWRAP(invokeRequest(move(request), sf::inlineDataExtractor<Ret>()));
        }

    public:
        Ret socket(AddressFamily domain, Type type, Protocol protocol) {
            std::array<u8, 12> input = sf::packInput(u32(domain), u32(type), u32(protocol));
            return HK_UNWRAP(invokeRequest(sf::Request(this, 2, &input), sf::inlineDataExtractor<Ret>()));
        }

        Ret recv(s32 fd, std::span<u8> buffer, s32 flags) {
            auto input = sf::packInput(fd, flags);
            auto request = sf::Request(this, 8, &input);

            request.addOutAutoselect(buffer.data(), buffer.size_bytes());
            return HK_UNWRAP(invokeRequest(move(request), sf::inlineDataExtractor<Ret>()));
        }

        template <typename A, typename T>
            requires(std::is_convertible<A*, SocketAddr*>::value)
        Ret recvFrom(s32 fd, std::span<u8> buffer, s32 flags, const A& address) {
            auto input = sf::packInput(fd, flags);
            auto request = sf::Request(this, 9, &input);

            request.addOutAutoselect(buffer.data(), buffer.size_bytes());
            request.addOutAutoselect(&address, sizeof(A));
            return HK_UNWRAP(invokeRequest(move(request), sf::inlineDataExtractor<Ret>()));
        }

        template <typename T>
        Ret send(s32 fd, std::span<const T> data, s32 flags) {
            auto input = sf::packInput(fd, flags);
            auto request = sf::Request(this, 10, &input);

            request.addInAutoselect(data.data(), data.size_bytes());

            return HK_UNWRAP(invokeRequest(move(request), sf::inlineDataExtractor<Ret>()));
        }

        template <typename A, typename T>
            requires(std::is_convertible<A*, SocketAddr*>::value)
        Ret sendTo(s32 fd, std::span<const T> data, s32 flags, const A& address) {
            auto input = sf::packInput(fd, flags);
            auto request = sf::Request(this, 11, &input);
            request.addInAutoselect(data.data(), data.size_bytes());
            request.addInAutoselect(&address, sizeof(SocketAddrIpv4));
            request.enableDebug(true, true);
            return HK_UNWRAP(invokeRequest(move(request), sf::inlineDataExtractor<Ret>()));
        }

        Tuple<s32, s32, SocketAddrIpv4> accept(s32 fd) {
            SocketAddrIpv4 outAddr;
            auto input = sf::packInput(fd, sizeof(outAddr));
            auto request = sf::Request(this, 12, &input);

            request.addOutAutoselect(&outAddr, sizeof(outAddr));

            auto response = HK_UNWRAP(invokeRequest(move(request), sf::inlineDataExtractor<Tuple<s32, s32, u32>>()));
            return { response.a, response.b, outAddr };
        }

        template <typename A>
            requires(std::is_convertible<A*, SocketAddr*>::value)
        Ret bind(s32 fd, const A& address) {
            return inFdInAddress<13>(fd, address);
        }

        template <typename A, typename B>
            requires(std::is_convertible<A*, SocketAddr*>::value)
        Ret connect(s32 fd, const A& address) {
            return inFdInAddress<14>(fd, address);
        }

        template <typename A, typename T>
            requires(std::is_convertible<A*, SocketAddr*>::value)
        Ret getPeerName(s32 fd, A* address) {
            return inFdOutAddress<15>(fd, address);
        }

        template <typename A, typename T>
            requires(std::is_convertible<A*, SocketAddr*>::value)
        Ret getSockName(s32 fd, A* address) {
            return inFdOutAddress<16>(fd, address);
        }

        Ret getSockOpt(s32 fd, s32 level, s32 optName, std::span<u8> opt) {
            auto input = sf::packInput(fd, level, optName);
            auto request = sf::Request(this, 17, &input);

            request.addOutAutoselect(opt.data(), opt.size_bytes());
            return HK_UNWRAP(invokeRequest(move(request), sf::inlineDataExtractor<Ret>()));
        }

        Ret listen(s32 fd, s32 backlog) {
            auto input = sf::packInput(fd, backlog);

            return HK_UNWRAP(invokeRequest(sf::Request(this, 18, &input), sf::inlineDataExtractor<Ret>()));
        }

        Ret fcntl(s32 fd, s32 cmd, s32 flags) {
            auto input = sf::packInput(fd, cmd, flags);

            return HK_UNWRAP(sf::invokeSimple<Ret>(*this, 20, fd, cmd, flags));
        }

        Ret setSockOpt(s32 fd, s32 level, s32 optName, std::span<const u8> opt) {
            auto input = sf::packInput(fd, level, optName);
            auto request = sf::Request(this, 21, &input);

            request.addInAutoselect(opt.data(), opt.size_bytes());

            return HK_UNWRAP(invokeRequest(move(request), sf::inlineDataExtractor<Ret>()));
        }

        template <typename T>
        hk_alwaysinline inline Ret setSockOpt(s32 fd, s32 level, s32 optName, const T& opt) {
            return setSockOpt(fd, level, optName, std::span<const u8>(&opt, sizeof(T)));
        }

        Ret shutdown(s32 fd, s32 how) {
            return HK_UNWRAP(sf::invokeSimple<Ret>(*this, 22, fd, how));
        }

        Ret shutdownAllSockets(s32 how) {
            return HK_UNWRAP(sf::invokeSimple<Ret>(*this, 23, how));
        }

        Ret write(s32 fd, std::span<const u8> data) {
            auto request = sf::Request(this, 24, &fd);

            request.addInAutoselect(data.data(), data.size_bytes());

            return HK_UNWRAP(invokeRequest(move(request), sf::inlineDataExtractor<Ret>()));
        }

        Ret read(s32 fd, std::span<u8> buffer) {
            auto request = sf::Request(this, 25, &fd);

            request.addOutAutoselect(buffer.data(), buffer.size_bytes());

            return HK_UNWRAP(invokeRequest(move(request), sf::inlineDataExtractor<Ret>()));
        }

        Ret close(s32 fd) {
            return HK_UNWRAP(sf::invokeSimple<Ret>(*this, 26, fd));
        }

        Ret duplicateSocket(s32 fd) {
            return HK_UNWRAP(sf::invokeSimple<Ret>(*this, 27, fd));
        }

        // Ret recvMMsg(s32 fd, std::span<u8> buffer, u32 n, s32 flags, /* timespec */);
    };

} // namespace hk::socket
