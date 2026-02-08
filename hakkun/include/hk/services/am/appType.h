#pragma once

namespace hk::am {

    enum class AppType {
        Application = 0,
        SystemApplet = 1,
        LibraryApplet = 2,
        OverlayApplet = 3,
        SystemApplication = 4,

        _Count = 5,
    };

} // namespace hk::am::detail
