#pragma once

#include "hk/Result.h"
#include "hk/ValueOrResult.h"
#include "hk/services/nv/ioctl.h"
#include "hk/services/nv/result.h"
#include "hk/services/sm.h"
#include "hk/sf/sf.h"
#include "hk/sf/utils.h"
#include "hk/svc/api.h"
#include "hk/svc/types.h"
#include "hk/types.h"
#include "hk/util/Singleton.h"
#include <optional>
#include <string_view>

namespace hk::nvdrv {

    enum class ServiceType {
        Application,
        Applet,
        Sysmodule,
        Factory
    };

    class NvidiaDriver : public sf::Service {
        HK_SINGLETON(NvidiaDriver)
    public:
        NvidiaDriver(sf::Service&& service)
            : sf::Service(forward<sf::Service>(service)) { }

        template <ServiceType Type, size TransferMemSize = 128_KB>
        static ValueOrResult<NvidiaDriver*> initialize(std::optional<u64> appletResourceUserId, std::array<u8, TransferMemSize>* transferMemory) {
            static_assert(TransferMemSize == alignUp(TransferMemSize, cPageSize), "transfer memory must be page aligned");

            switch (Type) {
            case ServiceType::Application:
                createInstance(HK_TRY(sm::ServiceManager::instance()->getServiceHandle<"nvdrv">()));
                break;
            case ServiceType::Applet:
                createInstance(HK_TRY(sm::ServiceManager::instance()->getServiceHandle<"nvdrv:a">()));
                break;
            case ServiceType::Sysmodule:
                createInstance(HK_TRY(sm::ServiceManager::instance()->getServiceHandle<"nvdrv:s">()));
                break;
            case ServiceType::Factory:
                createInstance(HK_TRY(sm::ServiceManager::instance()->getServiceHandle<"nvdrv:t">()));
                break;
            }

            Handle transferMemoryHandle = HK_TRY(svc::CreateTransferMemory(ptr(transferMemory), TransferMemSize, svc::MemoryPermission_None));

            size transferMemorySize = TransferMemSize;
            auto request = sf::Request(instance(), 3, &transferMemorySize);
            request.addCopyHandle(svc::CurrentProcess);
            request.addCopyHandle(transferMemoryHandle);
            HK_TRY(instance()->invokeRequest(move(request), sf::inlineDataExtractor<u32>()).mapToResult(convertErrorToResult));

            if (appletResourceUserId.has_value())
                HK_TRY(instance()->setAppletResourceUserId(*appletResourceUserId));

            return instance();
        }

        Result setAppletResourceUserId(u64 appletResourceUserId) {
            auto input = sf::packInput(u64(0), appletResourceUserId);
            auto request = sf::Request(this, 8, &input);
            return invokeRequest(move(request), sf::inlineDataExtractor<u32>())
                .mapToResult(convertErrorToResult);
        }

        ValueOrResult<u32> open(std::string_view view) {
            auto request = sf::Request(this, 0);
            request.addInAutoselect(view.data(), view.size());
            return invokeRequest(move(request), sf::inlineDataExtractor<u32>())
                .mapToResult(convertErrorToResult);
        }

        Result close(u32 fd) {
            return sf::invokeSimple<u32>(this, 2, &fd)
                .mapToResult(convertErrorToResult);
        }

        template <typename A, typename I = u8>
        Result ioctl(u32 fd, Ioctl ioCode, std::span<A> argument, std::optional<std::span<I>> secondInput = std::nullopt) {
            static_assert(ioCode.encoding.argumentSize == sizeof(A));

            auto input = sf::packInput(fd, ioCode);
            // choose between ioctl and ioctl2
            auto request = sf::Request(this, secondInput.has_value() ? 11 : 1, &input);

            if (ioCode.encoding.read)
                request.addInAutoselect(argument.data(), argument.size_bytes());
            else
                request.addInAutoselect(0, 0);

            if (secondInput.has_value())
                request.addInAutoselect(*secondInput, (secondInput).size_bytes());

            if (ioCode.encoding.write)
                request.addOutAutoselect(argument.data(), argument.size_bytes());
            else
                request.addOutAutoselect(0, 0);

            return invokeRequest(move(request), sf::inlineDataExtractor<u32>())
                .mapToResult(convertErrorToResult);
        }

        ValueOrResult<Handle> queryEvent(u32 fd, u32 eventId) {
            auto input = sf::packInput(fd, eventId);
            return invokeRequest(sf::Request(this, 4, &input), [](sf::Response& response) -> ValueOrResult<Handle> {
                HK_TRY(convertErrorToResult(sf::inlineDataExtractor<u32>()(response)));
                return response.nextCopyHandle();
            });
        }
    };

}
