#pragma once

#include "hk/ValueOrResult.h"
#include "hk/services/vi/parcel.h"
#include "hk/sf/sf.h"
#include "hk/sf/utils.h"
#include "hk/types.h"

namespace hk::vi::binder {
    class Binder : public sf::Service {
        u32 mId;

        Result adjustRefCount(s32 adjustBy, s32 type) {
            return sf::invokeSimple(*this, 1, mId, adjustBy, type);
        }

    public:
        enum class Operation : u32 {
            RequestBuffer = 1,
            SetBufferCount = 2,
            DequeueBuffer = 3,
            DetachBuffer = 4,
            DetachNextBuffer = 5,
            AttachBuffer = 6,
            QueueBuffer = 7,
            CancelBuffer = 8,
            Query = 9,
            Connect = 10,
            Disconnect = 11,
            SetSidebandStream = 12,
            AllocateBuffers = 13,
        };

        Binder(sf::Service&& service, u32 id)
            : sf::Service(forward<sf::Service>(service))
            , mId(id) { }

        template <size size>
        ValueOrResult<parcel::OutParcel> transactParcel(Operation operation, u32 flags, parcel::InParcel<size>& parcel) {
            parcel::OutParcel outParcel;
            auto input = sf::packInput(mId, operation, flags);
            auto request = sf::Request(this, 3, &input);
            request.addInAutoselect(&parcel, sizeof(parcel::InParcel<size>));
            request.addOutAutoselect(&outParcel, sizeof(parcel::OutParcel));
            HK_TRY(invokeRequest(move(request)));
            return outParcel;
        }

        Result incrementStrongRef() { return adjustRefCount(1, 1); }
        Result decrementStrongRef() { return adjustRefCount(-1, 1); }
        Result incrementWeakRef() { return adjustRefCount(1, 0); }
        Result decrementWeakRef() { return adjustRefCount(1, 0); }

        ValueOrResult<Handle> getEventHandle(u32 unknown) {
            auto input = sf::packInput(mId, unknown);
            return invokeRequest(sf::Request(this, 2, &input), [](sf::Response& response) {
                return response.nextCopyHandle();
            });
        }
    };
}
