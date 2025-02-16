#pragma once

#include "hk/hook/InstrUtil.h"
#include "hk/hook/Replace.h"
#include "hk/svc/api.h"
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

        Func getBackupFuncPtr() const { return cast<Func>(mBackup->getRx()); }

        using Rp::getAt;
        using Rp::mFunc;
        using Rp::mModule;
        using Rp::mOffset;
        using Rp::mOrigInstr;

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

            mOrigInstr = *cast<Instr*>(getAt());
            Result rc = writeBranch(mModule, mOffset, mFunc);
            if (rc.failed()) {
                mModule = nullptr;
                mOffset = 0;
                mOrigInstr = 0;
                return rc;
            }

            mBackup = detail::sTrampolinePool.allocate();
            HK_ABORT_UNLESS(mBackup != nullptr, "TrampolinePool full! Current size: 0x%x", HK_HOOK_TRAMPOLINE_POOL_SIZE);
            mBackup->origInstr = mOrigInstr; // TODO: Relocate instruction
            mBackup->bRetInstr = makeB(mBackup->getRx() + sizeof(Instr), getAt() + sizeof(Instr));
            svc::clearCache(mBackup->getRx(), sizeof(detail::TrampolineBackup));

            orig = getBackupFuncPtr();

            return ResultSuccess();
        }

        Result uninstall() override {
            HK_UNLESS(Rp::isInstalled(), ResultNotInstalled());

            detail::sTrampolinePool.free(mBackup);
            mBackup = nullptr;

            HK_TRY(Rp::mModule->writeRo(mOffset, mOrigInstr));
            mModule = nullptr;
            mOffset = 0;
            mOrigInstr = 0;

            return ResultSuccess();
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
