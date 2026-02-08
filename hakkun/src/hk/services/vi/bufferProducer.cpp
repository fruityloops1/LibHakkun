#include "hk/services/vi/android/bufferProducer.h"
#include "hk/services/vi/result.h"

namespace hk::vi::detail {
    Result convertBinderError(u32 error) {
        switch (error) {
        case 1:
            return ResultBinderPermissionDenied();
        case 2:
            return ResultBinderNameNotFound();
        case 11:
            return ResultBinderWouldBlock();
        case 12:
            return ResultBinderNoMemory();
        case 17:
            return ResultBinderAlreadyExists();
        case 19:
            return ResultBinderNoInit();
        case 22:
            return ResultBinderBadValue();
        case 32:
            return ResultBinderDeadObject();
        case 38:
            return ResultBinderInvalidOperation();
        case 61:
            return ResultBinderNotEnoughData();
        case 74:
            return ResultBinderUnknownTransaction();
        case 75:
            return ResultBinderBadIndex();
        case 110:
            return ResultBinderTimedOut();
        case -(INT32_MIN + 7):
            return ResultBinderFdsNotAllowed();
        case -(INT32_MIN + 2):
            return ResultBinderFailedTransaction();
        case -(INT32_MIN + 1):
            return ResultBinderBadType();
        default:
            return ResultBinderUnknownError();
        }
    }

    Result connect(BinderRelay& relay) {
        InParcel<0x40> parcel;
        parcel.write([](util::Stream<u8>& writer) {
            writeInterfaceToken(writer, gbpInterfaceToken, sizeof(gbpInterfaceToken) - 1);
            writer.write<u32>(0);
            writer.write<u32>(2); // NATIVE_WINDOW_API_CPU
            writer.write<u32>(false); // producerControlledByApp
        });
        auto out = HK_TRY(relay.transactParcel(Operation::Connect, 0, parcel));

        return convertBinderError(out.reader().read<u32>());
    }
}
