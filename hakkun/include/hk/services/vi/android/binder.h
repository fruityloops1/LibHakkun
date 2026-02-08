#pragma once

#include "hk/ValueOrResult.h"
#include "hk/services/vi/android/parcel.h"
#include "hk/sf/sf.h"
#include "hk/sf/utils.h"
#include "hk/types.h"

namespace hk::vi::detail {

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

        BufferReleasedEvent = 15,
    };

    class BinderRelay : public sf::Service {
        u32 mId;

        Result adjustRefCount(s32 adjustBy, s32 type) {
            return sf::invokeSimple(this, 1, mId, adjustBy, type);
        }

    public:
        BinderRelay(sf::Service&& service, u32 id)
            : sf::Service(forward<sf::Service>(service))
            , mId(id) { }

        template <size size>
        ValueOrResult<OutParcel> transactParcel(Operation operation, u32 flags, InParcel<size>& parcel) {
            OutParcel outParcel;
            auto input = sf::packInput(mId, operation, flags);
            auto request = sf::Request(this, 3, &input);
            request.addInAutoselect<u8>(&parcel, sizeof(InParcel<size>));
            request.addOutAutoselect<u8>(outParcel.data);
            HK_TRY(invokeRequest(move(request)));
            return outParcel;
        }

        u32 id() { return mId; }
        Result incrementStrongRef() { return adjustRefCount(1, 1); }
        Result decrementStrongRef() { return adjustRefCount(-1, 1); }
        Result incrementWeakRef() { return adjustRefCount(1, 0); }
        Result decrementWeakRef() { return adjustRefCount(1, 0); }

        // guessing that it's an operation, there isn't any clear definitive proof though.
        // basically only used for BufferReleasedEvent.
        ValueOrResult<Handle> getEventHandle(Operation operation) {
            return sf::invokeSimple<Handle>(this, 2, mId, operation);
        }
    };
} // namespace hk::vi::detail
