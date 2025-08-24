#include "hk/sf/sf.h"
#include "hk/Result.h"
#include "hk/sf/utils.h"

namespace hk::sf {
    u16 Service::pointerBufferSize() {
        return mPointerBufferSize = invokeControl(Request(this, 1), simpleDataHandler<u16>());
    }
    Result Service::convertToDomain() {
        return invokeControl(Request(this, 0), [this](Response& response) {
            mObject = response.objects.remove(0);
            return nullptr;
        });
    }
}
