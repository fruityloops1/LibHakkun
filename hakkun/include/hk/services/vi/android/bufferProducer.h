#pragma once

#include "hk/services/vi/android/binder.h"

namespace hk::vi::detail {

    constexpr char gbpInterfaceToken[] = "android.gui.IGraphicBufferProducer";

    
    Result connect(BinderRelay& relay);
    Result requestBuffer(BinderRelay& relay);

} // namespace hk::vi::detail
