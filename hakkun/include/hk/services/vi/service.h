#pragma once

#include "hk/ValueOrResult.h"
#include "hk/diag/diag.h"
#include "hk/services/sm.h"
#include "hk/sf/sf.h"
#include "hk/sf/utils.h"
#include "hk/types.h"
#include "hk/util/Singleton.h"
#include "hk/util/Tuple.h"
#include <utility>

namespace hk::vi {

    enum class DisplayType {
        Null,
        Default,
        External,
        Edid,
        Internal
    };

    using DisplayName = std::array<char, 0x40>;

    using NativeWindow = std::array<u8, 0x100>;

    struct Display {
        bool isInitialized = false;
        u64 id = 0;
        DisplayName name;

        Display()
            : name("Default") { }
        Display(DisplayType type) {
            switch (type) {
            case DisplayType::Null:
                std::strncpy(name.data(), "Null", name.size());
                break;
            case DisplayType::External:
                std::strncpy(name.data(), "External", name.size());
                break;
            case DisplayType::Edid:
                std::strncpy(name.data(), "Edid", name.size());
                break;
            case DisplayType::Internal:
                std::strncpy(name.data(), "Internal", name.size());
                break;
            case DisplayType::Default:
                std::strncpy(name.data(), "Default", name.size());
                break;
            }
        }
        ~Display() = default;

        Result open();
        Result close();
        Result setEnabled(bool enabled);
        ValueOrResult<Tuple<u64, u64>> getResolution();
        ValueOrResult<Handle> getVsyncEvent();
        ValueOrResult<Handle> getVsyncEventForDebug();
    };

    struct DisplayInfo {
        DisplayName name;
        bool hasLayerLimit;
        u64 layerCountMax;
        u64 layerWidthPixelCountMax;
        u64 layerHeightPixelCountMax;
    };

    struct Layer {
        bool isInitialized = false;
        u64 id = 0;
    };

    class SystemDisplayService : sf::Service {
        HK_SINGLETON(SystemDisplayService);

    public:
        SystemDisplayService(sf::Service&& service)
            : sf::Service(std::forward<sf::Service>(service)) { }
    };

    class ManagerDisplayService : sf::Service {
        HK_SINGLETON(ManagerDisplayService);

    public:
        ManagerDisplayService(sf::Service&& service)
            : sf::Service(std::forward<sf::Service>(service)) { }

        ValueOrResult<u64> allocateProcessHeapBlock(u64 unk) {
            return sf::invokeSimple<u64>(*this, 200, &unk);
        }

        Result freeProcessHeapBlock(u64 unk) {
            return sf::invokeSimple(*this, 201, &unk);
        }
    };

    class ApplicationDisplayService : sf::Service {
        HK_SINGLETON(ApplicationDisplayService);

    public:
        ApplicationDisplayService(sf::Service&& service)
            : sf::Service(std::forward<sf::Service>(service)) { }

        sf::Service getRelayService() {
            return HK_UNWRAP(invokeRequest(sf::Request(this, 100), [this](sf::Response& response) { return response.nextSubservice(this); }));
        }

        sf::Service getSystemDisplayService() {
            return HK_UNWRAP(invokeRequest(sf::Request(this, 101), [this](sf::Response& response) { return response.nextSubservice(this); }));
        }

        sf::Service getManagerDisplayService() {
            return HK_UNWRAP(invokeRequest(sf::Request(this, 102), [this](sf::Response& response) { return response.nextSubservice(this); }));
        }

        sf::Service getIndirectDisplayTransactionService() {
            return HK_UNWRAP(invokeRequest(sf::Request(this, 103), [this](sf::Response& response) { return response.nextSubservice(this); }));
        }

        size listDisplays(std::span<DisplayInfo> displays) {
            auto request = sf::Request(this, 1000);
            request.addOutMapAlias(displays.data(), displays.size_bytes());
            HK_ABORT_UNLESS_R(invokeRequest(move(request)));
            return displays.size();
        }

        ValueOrResult<u64> openDisplay(Display& display) {
            return sf::invokeSimple<u64>(*this, 1010, &display.name);
        }

        ValueOrResult<u64> openDefaultDisplay() {
            return sf::invokeSimple<u64>(*this, 1011);
        }

        Result closeDisplay(Display& display) {
            return sf::invokeSimple(*this, 1020, &display.id);
        }

        Result setDisplayEnabled(Display& display, bool isEnabled) {
            u32 enabled = isEnabled;
            return sf::invokeSimple(*this, 1101, &enabled, &display.id);
        }

        ValueOrResult<Tuple<u64, u64>> getDisplayResolution(Display& display) {
            return sf::invokeSimple<Tuple<u64, u64>>(*this, 1102, &display.id);
        }

        ValueOrResult<Tuple<u64, NativeWindow>> openLayer(Display& display, u64 layerId, u64 appletResourceUserId) {
            NativeWindow nativeWindow;
            
            auto input = sf::packInput(display.name, layerId, appletResourceUserId);
            auto request = sf::Request(this, 2020, &input);
            request.setSendPid();
            request.addOutMapAlias(nativeWindow.data(), nativeWindow.size());
            return invokeRequest(move(request), sf::simpleDataHandler<Tuple<u64, NativeWindow>>());
        }

        ValueOrResult<Tuple<u64, u64, NativeWindow>> createStrayLayer(Display& display, u32 flags) {
            NativeWindow nativeWindow;
            
            auto input = sf::packInput(flags, display.id);
            auto request = sf::Request(this, 2030, &input);
            request.addOutMapAlias(nativeWindow.data(), nativeWindow.size());
            return invokeRequest(move(request), sf::simpleDataHandler<Tuple<u64, u64, NativeWindow>>());
        }

        Result destroyStrayLayer(Layer& layer) {
            return sf::invokeSimple(*this, 2031, &layer.id);
        }

        Result setLayerScalingMode(Layer& layer, u32 scalingMode) {
            return sf::invokeSimple(*this, 2101, &scalingMode, &layer.id);
        }

        ValueOrResult<u64> convertScalingMode(u32 scalingMode) {
            return sf::invokeSimple(*this, 2102, &scalingMode);
        }

        ValueOrResult<Tuple<u64, u64>> getIndirectLayerImageMap(std::span<u8> map, s64 width, s64 height, u64 handle, u64 aruid) {
            auto input = sf::packInput(width, height, handle, aruid);
            auto request = sf::Request(this, 2450, &input);
            request.addOutMapAlias(map.data(), map.size(), sf::hipc::BufferMode::NonSecure);
            request.setSendPid();
            return invokeRequest(move(request), sf::simpleDataHandler<Tuple<u64, u64>>());
        }

        ValueOrResult<Tuple<u64, u64>> getIndirectLayerImageCropMap(std::span<u8> map, f32 f1, f32 f2, f32 f3, f32 f4, u64 u1, u64 u2, u64 u3, u64 aruid) {
            auto input = sf::packInput(f1, f2, f3, f4, u1, u2, u3, aruid);
            auto request = sf::Request(this, 2451, &input);
            request.addOutMapAlias(map.data(), map.size(), sf::hipc::BufferMode::NonSecure);
            request.setSendPid();
            return invokeRequest(move(request), sf::simpleDataHandler<Tuple<u64, u64>>());
        }

        ValueOrResult<Tuple<s64, s64>> getIndirectLayerImageRequiredMemoryInfo(s64 width, s64 height) {
            return sf::invokeSimple<Tuple<s64, s64>>(*this, 2460, &width, &height);
        }

        ValueOrResult<Handle> getDisplayVsyncEvent(Display& display) {
            auto request = sf::Request(this, 5202, &display.id);
            return invokeRequest(move(request), [](sf::Response& response) { return response.nextCopyHandle(); });
        }

        ValueOrResult<Handle> getDisplayVsyncEventForDebug(Display& display) {
            auto request = sf::Request(this, 5203, &display.id);
            return invokeRequest(move(request), [](sf::Response& response) { return response.nextCopyHandle(); });
        }
    };

    class VideoInterface : sf::Service {
        HK_SINGLETON(VideoInterface);

    public:
        enum class ServiceType {
            Application,
            System,
            Manager,
        };

    private:
        ServiceType mType;

    public:
        VideoInterface(sf::Service&& service)
            : sf::Service(std::forward<sf::Service>(service)) { }

        template <ServiceType Type = ServiceType::Application>
        static VideoInterface* initialize() {
            util::Storage<sf::Service> service;
            switch (Type) {
            case ServiceType::Application:
                service.create(sm::ServiceManager::instance()->getServiceHandle<"vi:u">());
                break;
            case ServiceType::System:
                service.create(sm::ServiceManager::instance()->getServiceHandle<"vi:s">());
                break;
            case ServiceType::Manager:
                service.create(sm::ServiceManager::instance()->getServiceHandle<"vi:m">());
                break;
            }
            createInstance(service.take());
            instance()->mType = Type;

            ApplicationDisplayService::createInstance(instance()->getDisplayService());
            SystemDisplayService::createInstance(ApplicationDisplayService::instance()->getSystemDisplayService());
            ManagerDisplayService::createInstance(ApplicationDisplayService::instance()->getManagerDisplayService());

            return instance();
        }

        sf::Service getDisplayService() {
            u32 commandId;
            switch (mType) {
            case ServiceType::Application:
                commandId = 0;
                break;
            case ServiceType::System:
                commandId = 1;
                break;
            case ServiceType::Manager:
                commandId = 2;
                break;
            }
            return HK_UNWRAP(invokeRequest(sf::Request(this, commandId), [this](sf::Response& response) { return response.nextSubservice(this); }));
        }
    };

    void initialize();
    Result openDefaultDisplay();
    size listDisplays(std::span<DisplayInfo> displays);

} // namespace hk::vi
