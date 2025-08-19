#include "hk/sf/sf.h"
#include "hk/Result.h"

namespace hk::sf {
    Result Service::convertToDomain() {
        auto request = Request(0);
        return invokeControl(request, [this](Response& response) {
            if (ownedHandle)
                object = response.objects.remove(0);
            return nullptr;
        });
    }
}
