#pragma once

#include "hk/hook/InstrUtil.h"
#include "hk/hook/Replace.h"
#include "hk/svc/api.h"
#include "hk/util/PoolAllocator.h"

namespace hk::hook {

    namespace detail {

        // A backup slot holds up to cMaxSlots instructions emitted by the
        // prologue relocator. For a non-PC-relative original, only the
        // first two slots are used (verbatim copy + back-branch); for
        // PC-relative originals (adrp/adr/b/bl/b.cond/cbz/cbnz/tbz/tbnz)
        // the relocator emits a movz/movk-immediate-build plus a branch
        // sequence whose worst case is 8 instructions.
        struct TrampolineBackup {
            static constexpr int cMaxSlots = 8;
            Instr instrs[cMaxSlots];

            ptr getRx() const;
        };

        // Decode `orig` (the instruction at function-entry PC `origPc`) and
        // populate `backup->instrs[]` so that executing from backup->getRx()
        // has the same architectural effect, then (unless the original was an
        // unconditional B to a fixed target) append a `b origPc + 4` so
        // control resumes inside the hooked function.
        void installRelocatedPrologue(TrampolineBackup* backup, Instr orig, ptr origPc);

        extern util::PoolAllocator<TrampolineBackup, HK_HOOK_TRAMPOLINE_POOL_SIZE> sTrampolinePool;

    } // namespace detail

    /**
     * @brief Hook to replace function in a module, while preserving the ability to call the original function
     *
     * @tparam Func
     */
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
            : Rp(forward<L>(func)) { }

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
            detail::installRelocatedPrologue(mBackup, mOrigInstr, getAt());

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
    typename std::enable_if<!util::LambdaHasCapture<L>::value, TrampolineHook<typename util::FunctionTraits<L>::FuncPtrTypeStatic>>::type trampoline(L func) {
        using Traits = util::FunctionTraits<L>;
        return { Traits::fromLambda(forward<L>(func)) };
    }

} // namespace hk::hook

template <typename Ret, typename... Args>
using HkTrampoline = hk::hook::TrampolineHook<Ret (*)(Args...)>;
template <typename Ret, typename... Args>
using HkTrampolineVarArgs = hk::hook::TrampolineHook<Ret (*)(Args..., ...)>;
