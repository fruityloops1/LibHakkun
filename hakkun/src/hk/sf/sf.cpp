#include "hk/sf/sf.h"
#include "hk/Result.h"
#include "hk/diag/diag.h"
#include "hk/sf/utils.h"
#include <limits>

namespace hk::sf {

    u16 Service::pointerBufferSize() {
        if (mPointerBufferSize != std::numeric_limits<u16>::max())
            return mPointerBufferSize;
        Request req = Request(nullptr, this, 3);
        return mPointerBufferSize = HK_UNWRAP(invokeControl(forward<Request>(req), inlineDataExtractor<u16>()));
    }

    Result Service::convertToDomain() {
        mObject = HK_TRY(invokeControl(Request(this, 0), inlineDataExtractor<u32>()));

        return ResultSuccess();
    }

    Result Service::release() {
        if (mObject.has_value()) {
            auto request = sf::Request(this, 0);
            request.setDomainClose();
            return invokeRequest(move(request));
        }

        auto request = sf::Request(this, 0);
        return invoke(cmif::MessageTag::Close, move(request));
    }

} // namespace hk::sf
