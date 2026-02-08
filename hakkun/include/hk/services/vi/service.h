#pragma once

#include "hk/ValueOrResult.h"
#include "hk/services/sm.h"
#include "hk/services/vi/android/binder.h"
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
    struct NativeWindowParcel {
        std::array<u8, 0x100> data;
        u32 size;
    };

    struct LayerId {
        u64 value = 0;
    };

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
        LayerId id;
        u32 binderObjectId;

        Layer(LayerId id, u32 objectId)
            : id(id)
            , binderObjectId(objectId) { }
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
            return sf::invokeSimple<u64>(this, 200, &unk);
        }

        Result freeProcessHeapBlock(u64 unk) {
            return sf::invokeSimple(this, 201, &unk);
        }
    };

    class ApplicationDisplayService : sf::Service {
        HK_SINGLETON(ApplicationDisplayService);

    public:
        ApplicationDisplayService(sf::Service&& service)
            : sf::Service(std::forward<sf::Service>(service)) { }

        ValueOrResult<sf::Service> getRelayService() {
            return sf::invokeSimple<sf::Service>(this, 100);
        }

        ValueOrResult<sf::Service> getSystemDisplayService() {
            return sf::invokeSimple<sf::Service>(this, 101);
        }

        ValueOrResult<sf::Service> getManagerDisplayService() {
            return sf::invokeSimple<sf::Service>(this, 102);
        }

        ValueOrResult<sf::Service> getIndirectDisplayTransactionService() {
            return sf::invokeSimple<sf::Service>(this, 103);
        }

        ValueOrResult<size> listDisplays(std::span<DisplayInfo> displays) {
            auto request = sf::Request(this, 1000);
            request.addOutMapAlias(displays.data(), displays.size_bytes());
            HK_TRY(invokeRequest(move(request)));
            return displays.size();
        }

        ValueOrResult<u64> openDisplay(Display& display) {
            return sf::invokeSimple<u64>(this, 1010, &display.name);
        }

        ValueOrResult<u64> openDefaultDisplay() {
            return sf::invokeSimple<u64>(this, 1011);
        }

        Result closeDisplay(Display& display) {
            return sf::invokeSimple(this, 1020, &display.id);
        }

        Result setDisplayEnabled(Display& display, bool isEnabled) {
            u32 enabled = isEnabled;
            return sf::invokeSimple(this, 1101, &enabled, &display.id);
        }

        ValueOrResult<Tuple<u64, u64>> getDisplayResolution(Display& display) {
            return sf::invokeSimple<Tuple<u64, u64>>(this, 1102, &display.id);
        }

        // returns the native window size and the native window parcel.
        ValueOrResult<NativeWindowParcel> openLayer(Display& display, LayerId layerId, u64 appletResourceUserId) {
            std::array<u8, 256> nativeWindow;

            auto input = sf::packInput(u64(0), display.name, layerId, appletResourceUserId);
            auto request = sf::Request(this, 2020, &input);
            request.setSendPid();
            request.addOutMapAlias<u8>(nativeWindow);
            return invokeRequest(move(request), sf::inlineDataExtractor<u64>())
                .map([=](u64 nativeWindowSize) -> NativeWindowParcel {
                    return NativeWindowParcel { .data = nativeWindow, .size = u32(nativeWindowSize) };
                });
        }

        // returns the LayerId, native window size, and the native window parcel.
        ValueOrResult<Tuple<LayerId, NativeWindowParcel>> createStrayLayer(Display& display, u32 flags) {
            std::array<u8, 256> nativeWindow;

            auto input = sf::packInput(flags, display.id);
            auto request = sf::Request(this, 2030, &input);
            request.addOutMapAlias<u8>(nativeWindow);
            return invokeRequest(move(request), sf::inlineDataExtractor<Tuple<LayerId, u64>>())
                .map([=](Tuple<LayerId, u64> out) -> Tuple<LayerId, NativeWindowParcel> {
                    return { out.a, NativeWindowParcel { .data = nativeWindow, .size = u32(out.b) } };
                });
        }

        Result destroyStrayLayer(Layer& layer) {
            return sf::invokeSimple(this, 2031, &layer.id);
        }

        Result setLayerScalingMode(Layer& layer, u32 scalingMode) {
            return sf::invokeSimple(this, 2101, &scalingMode, &layer.id);
        }

        ValueOrResult<u64> convertScalingMode(u32 scalingMode) {
            return sf::invokeSimple(this, 2102, &scalingMode);
        }

        ValueOrResult<Tuple<u64, u64>> getIndirectLayerImageMap(std::span<u8> map, s64 width, s64 height, u64 handle, u64 aruid) {
            auto input = sf::packInput(width, height, handle, aruid);
            auto request = sf::Request(this, 2450, &input);
            request.addOutMapAlias(map, sf::hipc::BufferMode::NonSecure);
            request.setSendPid();
            return invokeRequest(move(request), sf::inlineDataExtractor<Tuple<u64, u64>>());
        }

        ValueOrResult<Tuple<u64, u64>> getIndirectLayerImageCropMap(std::span<u8> map, f32 f1, f32 f2, f32 f3, f32 f4, u64 u1, u64 u2, u64 u3, u64 aruid) {
            auto input = sf::packInput(f1, f2, f3, f4, u1, u2, u3, aruid);
            auto request = sf::Request(this, 2451, &input);
            request.addOutMapAlias(map, sf::hipc::BufferMode::NonSecure);
            request.setSendPid();
            return invokeRequest(move(request), sf::inlineDataExtractor<Tuple<u64, u64>>());
        }

        ValueOrResult<Tuple<s64, s64>> getIndirectLayerImageRequiredMemoryInfo(s64 width, s64 height) {
            return sf::invokeSimple<Tuple<s64, s64>>(this, 2460, &width, &height);
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
        static ValueOrResult<VideoInterface*> initialize() {
            switch (Type) {
            case ServiceType::Application:
                createInstance(HK_TRY(sm::ServiceManager::instance()->getServiceHandle<"vi:u">()));
                break;
            case ServiceType::System:
                createInstance(HK_TRY(sm::ServiceManager::instance()->getServiceHandle<"vi:s">()));
                break;
            case ServiceType::Manager:
                createInstance(HK_TRY(sm::ServiceManager::instance()->getServiceHandle<"vi:m">()));
                break;
            }

            instance()->mType = Type;

            ApplicationDisplayService::createInstance(HK_TRY(instance()->getDisplayService()));

            if (Type == ServiceType::Manager || Type == ServiceType::System)
                SystemDisplayService::createInstance(HK_TRY(ApplicationDisplayService::instance()->getSystemDisplayService()));
            if (Type == ServiceType::Manager)
                ManagerDisplayService::createInstance(HK_TRY(ApplicationDisplayService::instance()->getManagerDisplayService()));

            return instance();
        }

        ValueOrResult<sf::Service> getDisplayService() {
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
            return sf::invokeSimple<sf::Service>(this, commandId);
        }
    };

    Result initialize();
    ValueOrResult<Display> openDefaultDisplay();
    size listDisplays(std::span<DisplayInfo> displays);
    ValueOrResult<Layer> openLayer(Display& display, LayerId layerId, u64 aruid);

} // namespace hk::vi
