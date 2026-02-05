#include "hk/sf/sf.h"
#include "hk/Result.h"
#include "hk/diag/diag.h"
#include "hk/sf/utils.h"
#include <limits>

namespace hk::sf {

    u16 Service::pointerBufferSize() {
        if (mPointerBufferSize != std::numeric_limits<u16>::max())
            return mPointerBufferSize;
        return mPointerBufferSize = invokeControl(Request(nullptr, this, 3), inlineDataExtractor<u16>());
    }

    Result Service::convertToDomain() {
        return invokeControl(Request(this, 0), [this](Response& response) {
            mObject = response.objects.remove(0);
            return nullptr;
        });
    }

    Result Service::release() {
        HK_ASSERT(mObject.has_value());
        auto request = sf::Request(this, 0);
        request.setDomainClose();
        return invokeControl(move(request), inlineDataExtractor<void>());
    }

} // namespace hk::sf
