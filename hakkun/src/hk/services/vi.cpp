#include "hk/services/vi/service.h"

namespace hk::vi {
    HK_SINGLETON_IMPL(VideoInterface);
    HK_SINGLETON_IMPL(ApplicationDisplayService);
    HK_SINGLETON_IMPL(SystemDisplayService);
    HK_SINGLETON_IMPL(ManagerDisplayService);

    Result initialize() {
        return VideoInterface::initialize();
    }

    Result openDefaultDisplay() {
        return ApplicationDisplayService::instance()->openDefaultDisplay();
    }

    size listDisplays(std::span<DisplayInfo> displays) {
        return ApplicationDisplayService::instance()->listDisplays(displays);
    }

    Result Display::open() {
        auto rc = ApplicationDisplayService::instance()->openDisplay(*this);
        return rc.map([this](u64 id){ this->id = id; });
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
