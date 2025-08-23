#pragma once

#include "hk/diag/diag.h"
#include "hk/hook/InstrUtil.h"
#include "hk/hook/Replace.h"
#include "hk/svc/api.h"
#include "hk/svc/types.h"
#include "hk/util/PoolAllocator.h"

namespace hk::hook {

    namespace detail {

        struct TrampolineBackup {
            Instr origInstr;
            Instr bRetInstr;

            ptr getRx() const;
        };

        extern util::PoolAllocator<TrampolineBackup, HK_HOOK_TRAMPOLINE_POOL_SIZE> sTrampolinePool;

    } // namespace detail

    template <typename Func>
    class TrampolineHook : public ReplaceHook<Func> {
        using Rp = ReplaceHook<Func>;

        detail::TrampolineBackup* mBackup = nullptr;

        Func getBackupFuncPtr() const {
            return mFarBackup != nullptr ? cast<Func>(&mFarBackup->origInstrForTrampoline) : cast<Func>(mBackup->getRx());
        }

        using Rp::checkBranchDistanceExceeded;
        using Rp::getAt;
        using Rp::getFarRx;
        using Rp::mFarBackup;
        using Rp::mFunc;
        using Rp::mModule;
        using Rp::mOffset;
        using Rp::mOrigInstr;
        using Rp::restoreBranch;
        using Rp::writeBranch;

    public:
        TrampolineHook(Func func)
            : Rp(func) { }

        template <typename L>
        TrampolineHook(L func)
            : Rp(func) { }

        Result installAtOffset(const ro::RoModule* module, ptr offset) override {
            HK_UNLESS(!Rp::isInstalled(), ResultAlreadyInstalled());

            mModule = module;
            mOffset = offset;

            Result rc = writeBranch();
            HK_ABORT_UNLESS_R(rc);

            const ptr to = getAt() + sizeof(Instr);
            ptr from;
            Instr* origInstr;
            Instr* bRetInstr;
            ptr rx;

            if (mFarBackup) {
                from = getFarRx() + sizeof(Instr);

                s64 distance = checkBranchDistanceExceeded(from, to);
                HK_ABORT_UNLESS(distance == 0, "Trampoline: Branch exceeded max branch distance (%zd > %zu)", distance, cMaxBranchDistance);

                origInstr = &mFarBackup->origInstrForTrampoline;
                bRetInstr = &mFarBackup->bRetInstrForTrampoline;

                rx = getFarRx();

                svc::MemoryInfo info;
                u32 page;
                svc::QueryMemory(&info, &page, rx);
                diag::debugLog("penis %zx %zx %zx %d %d", rx, info.base_address, info.size, info.state, info.permission);
            } else {
                mBackup = detail::sTrampolinePool.allocate();
                HK_ABORT_UNLESS(mBackup != nullptr, "TrampolinePool full! Current size: 0x%x", HK_HOOK_TRAMPOLINE_POOL_SIZE);

                from = mBackup->getRx() + sizeof(Instr);

                origInstr = &mBackup->origInstr;
                bRetInstr = &mBackup->bRetInstr;

                rx = mBackup->getRx();
            }

            *origInstr = mOrigInstr; // TODO: Relocate instruction, or at least abort if instruction needs to be relocated
            *bRetInstr = makeB(from, to);
            svc::clearCache(rx, mFarBackup != nullptr ? sizeof(detail::FarBackup) : sizeof(detail::TrampolineBackup));

            orig = getBackupFuncPtr();

            return ResultSuccess();
        }

        Result uninstall() override {
            HK_UNLESS(Rp::isInstalled(), ResultNotInstalled());

            if (mFarBackup == nullptr) {
                HK_ASSERT(mBackup != nullptr);
                detail::sTrampolinePool.free(mBackup);
                mBackup = nullptr;
            }

            return restoreBranch();
        }

        Func orig = nullptr;
    };

    template <typename L>
    typename std::enable_if<!util::LambdaHasCapture<L>::value, TrampolineHook<typename util::FunctionTraits<L>::FuncPtrType>>::type trampoline(L func) {
        using Func = typename util::FunctionTraits<L>::FuncPtrType;
        return { (Func)func };
    }

} // namespace hk::hook

template <typename Ret, typename... Args>
using HkTrampoline = hk::hook::TrampolineHook<Ret (*)(Args...)>;
template <typename Ret, typename... Args>
using HkTrampolineVarArgs = hk::hook::TrampolineHook<Ret (*)(Args..., ...)>;
