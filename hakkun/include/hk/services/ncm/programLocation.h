#pragma once

#include "hk/types.h"

namespace hk::ncm {
    enum class StorageId : u8 {
        None = 0,
        Host = 1,
        GameCard = 2,
        BuiltInSystem = 3,
        BuiltInUser = 4,
        SdCard = 5,
        Any = 6,
    };

    struct ProgramLocation {
        ProgramLocation(u64 programId, StorageId storageId)
            : programId(programId)
            , storageId(storageId) { }

        ProgramLocation(u64 programId)
            : programId(programId)
            , storageId(StorageId::None) { }

        u64 programId;
        StorageId storageId;
    };
} // namespace hk::ncm
