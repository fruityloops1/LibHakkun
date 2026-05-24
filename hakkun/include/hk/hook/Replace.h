#pragma once

#include "hk/container/VecSpan.h"
#include "hk/hook/Hook.h"
#include "hk/hook/a64/Assembler.h"

namespace hk::hook {

    template <typename Func>
    class Replace : public HookNInstr<detail::cHookMaxOverwriteSize>, public HookOperations<Replace<Func>> {
    protected:
        const Func mFunc = nullptr;

        const static size N = detail::cHookMaxOverwriteSize;

    public:
        Replace(Func func)
            : mFunc(func) { }

        template <typename L>
        Replace(L func)
            : mFunc(hk::util::FunctionTraits<L>::fromLambda(forward<L>(func))) { }

        Result installAtOffset(const ro::RoModule* module, ptr offset) {
            const ptr at = module->range().start() + offset;
            HK_UNLESS(!isInstalled(), ResultAlreadyInstalled());

            Instr instrs[N] { 0 };

            const ptr to = ptr(mFunc);

#if __aarch64__
            hk::VecSpan outInstrs(instrs, N);
            a64::PseudoInstructionEmitter<false> emitter(outInstrs, at);

            emitter.emitB(to);
            const size numInstrs = outInstrs.size();
            const Span<const Instr> orig = { cast<const Instr*>(at), numInstrs };

            return HookNInstr::installAtOffset(module, offset, outInstrs);
#else
            instrs[0] = makeB(at, to);
            return HookNInstr::installAtOffset(module, offset, { instrs, 1 });
#endif
        }
    };

    template <LambdaNoCaptureType L>
    Replace<typename util::FunctionTraits<L>::FuncPtrTypeStatic> replace(L func) {
        using Traits = util::FunctionTraits<L>;
        return { Traits::fromLambda(forward<L>(func)) };
    }

} // namespace hk::hook

template <typename Ret, typename... Args>
using HkReplace = hk::hook::Replace<Ret (*)(Args...)>;
