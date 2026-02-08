#include "hk/services/vi/result.h"
#include "hk/services/vi/service.h"
#include "hk/services/vi/window.h"

namespace hk::vi {

    using namespace detail;

    HK_SINGLETON_IMPL(VideoInterface);
    HK_SINGLETON_IMPL(ApplicationDisplayService);
    HK_SINGLETON_IMPL(SystemDisplayService);
    HK_SINGLETON_IMPL(ManagerDisplayService);

    Result initialize() {
        return VideoInterface::initialize();
    }

    ValueOrResult<Display> openDefaultDisplay() {
        auto display = Display(DisplayType::Default);

        display.id = HK_TRY(ApplicationDisplayService::instance()->openDefaultDisplay());

        return display;
    }

    size listDisplays(std::span<DisplayInfo> displays) {
        return ApplicationDisplayService::instance()->listDisplays(displays);
    }

    ValueOrResult<Layer> openLayer(Display& display, LayerId layerId, u64 aruid) {
        auto parcel = HK_TRY(ApplicationDisplayService::instance()->openLayer(display, layerId, aruid));

        auto header = (ParcelHeader*)parcel.data.data();
        if (sizeof(ParcelHeader) + header->objectsSize + header->payloadSize > parcel.size)
            return ResultInvalidParcelSize();
        if (header->payloadSize < 3 * sizeof(u32))
            return ResultInvalidPayloadSize();

        u32 objectId = header->payload<u32>()[2];

        return Layer(layerId, objectId);
    }

    Result Display::open() {
        auto rc = ApplicationDisplayService::instance()->openDisplay(*this);
        return rc.map([this](u64 id) { this->id = id; });
    }

    Result Display::close() {
        return ApplicationDisplayService::instance()->closeDisplay(*this);
    }

    Result Display::setEnabled(bool enabled) {
        return ApplicationDisplayService::instance()->setDisplayEnabled(*this, enabled);
    }

    ValueOrResult<Tuple<u64, u64>> Display::getResolution() {
        return ApplicationDisplayService::instance()->getDisplayResolution(*this);
    }

    ValueOrResult<Handle> Display::getVsyncEvent() {
        return ApplicationDisplayService::instance()->getDisplayVsyncEvent(*this);
    }

    ValueOrResult<Handle> Display::getVsyncEventForDebug() {
        return ApplicationDisplayService::instance()->getDisplayVsyncEventForDebug(*this);
    }

} // namespace hk::vi
