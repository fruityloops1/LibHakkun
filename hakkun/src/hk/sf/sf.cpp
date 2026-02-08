#include "hk/sf/sf.h"
#include "hk/Result.h"
#include "hk/diag/diag.h"
#include "hk/sf/utils.h"
#include <limits>

namespace hk::sf {

    u16 Service::pointerBufferSize() {
        if (mPointerBufferSize != std::numeric_limits<u16>::max())
            return mPointerBufferSize;
        return mPointerBufferSize = HK_UNWRAP(invokeControl(Request(nullptr, this, 3), inlineDataExtractor<u16>()));
    }

    Result Service::convertToDomain() {
        mObject = HK_TRY(invokeControl(Request(this, 0), inlineDataExtractor<u32>()));

        return ResultSuccess();
    }

    Result Service::release() {
        HK_ASSERT(mObject.has_value());
        auto request = sf::Request(this, 0);
        request.setDomainClose();
        return invokeControl(move(request), inlineDataExtractor<void>());
    }

} // namespace hk::sf
