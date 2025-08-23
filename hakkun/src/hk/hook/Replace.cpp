#include "hk/hook/Replace.h"
#include "hk/hook/MapUtil.h"
#include "hk/hook/Trampoline.h"
#include "hk/types.h"

namespace hk::hook {

    namespace detail {
        static u8 __attribute__((aligned(cPageSize))) sFarBackupPoolData[alignUpPage(HK_HOOK_TRAMPOLINE_POOL_SIZE * sizeof(FarBackup))];

        util::PoolAllocator<FarBackup, HK_HOOK_TRAMPOLINE_POOL_SIZE> sFarBackupPool { cast<FarBackup*>(sFarBackupPoolData) };
    } // namespace detail

} // namespace hk::hook
