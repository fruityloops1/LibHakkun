#include "hk/services/vi/android/binder.h"
#include "hk/types.h"

namespace hk::vi {
    class Window {
        u32 x;
        u32 y;
        u32 width;
        u32 height;
        u32 usage;
        u32 format;
        detail::BinderRelay binder;

        
    };
} // namespace hk::vi
