#include "hk/hook/Trampoline.h"
#include "hk/container/Array.h"
#include "hk/container/FixedVec.h"
#include "hk/container/VecSpan.h"
#include "hk/diag/diag.h"
#include "hk/hook/MapUtil.h"
#include "hk/prim/iterator/WithIndex.h"

#if __aarch64__
#include "hk/hook/a64/Assembler.h"
#include "hk/hook/a64/Instrs.h"
#endif

namespace hk::hook {

    Result TrampolineStaticBase::installAtOffset(const ro::RoModule* module, ptr offset, ptr to, detail::TrampolineBackup* backup) {
        const ptr at = module->range().start() + offset;

        Instr instrs[N] { 0 };

#if __aarch64__
        hk::VecSpan outInstrs(instrs, N);
        a64::PseudoInstructionEmitter<false> emitter(outInstrs, at);

        emitter.emitB(to);
        const size numInstrs = outInstrs.size();
        const Span<const Instr> orig = { cast<const Instr*>(at), numInstrs };
        backup->make(orig, at, at + numInstrs * sizeof(Instr));

        return HookNInstr::installAtOffset(module, offset, outInstrs);
#else
        instrs[0] = makeB(at, to);
        backup->make(orig, at, at + sizeof(Instr));
        return HookNInstr::installAtOffset(module, offset, { instrs, 1 });
#endif
    }

    namespace detail {

        const ptr cTrampolineRwMap = ([]() -> ptr { return HK_UNWRAP(mapRoToRw(cTrampolinesBeginRx, cTrampolinePoolSize)); })();

        size makeTrampolineBackup(
            Span<const Instr> orig,
            ptr origAddr,
            ptr origReturnAddr,
            VecSpan<Instr> out,
            ptr trampolineRx) {

            HK_ASSERT((origAddr - origReturnAddr) % sizeof(Instr) == 0);
#if __aarch64__
            using namespace a64;

            PseudoInstructionEmitter<true> emitter(out, trampolineRx);
#endif

            const auto emitReturn = [&]() {
#ifdef __aarch64__
                emitter.emitB(origReturnAddr);
#else
                out.add(makeB(trampolineRx + out.size() * sizeof(Instr), origReturnAddr));
#endif
            };

            const auto calcRxAddr = [&](size idx) -> ptr { return trampolineRx + idx * sizeof(Instr); };
            const auto calcCurRxAddr = [&]() -> ptr { return calcRxAddr(out.size()); };

            defer { svc::clearCache(trampolineRx, sizeof(Instr) * out.size()); };

            bool skipReturn = false;

            const auto reconstruct = [&](Instr orig, ptr origAddr, bool isLast) {
#if __aarch64__
                if (cwAdr.test(orig) or cwAdrp.test(orig)) {
                    const IRegType rd = grabRd(orig);
                    const ptr addr = grabAdrAdrpAddr(orig, origAddr);

                    emitter.emitMovImmediate64(rd, addr);
                    return;
                }

                const bool isB = cwB.test(orig);
                const bool isBL = cwBL.test(orig);

                if (isB or isBL) {
                    const ptr addr = grabBBLAddr(orig, origAddr);

                    if (isB) {
                        HK_ABORT_UNLESS(isLast, "cannot return from this situation");
                        skipReturn = true;
                    }
                    emitter.emitBranch(addr, isBL);
                    return;
                }

                if (cwBcond.test(orig)) {
                    HK_ABORT_UNLESS(isLast, "cannot return from this situation");

                    const ptr addr = grabBcondCbzCbnzLdrRelativeAddr(orig, origAddr);
                    const u8 condInv = grabBcondCond(orig) ^ 0b1;

                    constexpr auto bcondExpr = assemble<"b.() #{}">();

                    const size bcondIdx = out.size();
                    const ptr bcondPc = calcRxAddr(bcondIdx);
                    out.add(0);
                    emitter.emitBranch(addr, false);

                    const ptr returnPc = calcCurRxAddr();
                    out[bcondIdx] = bcondExpr.arg(condInv, returnPc).assembleOne(0, bcondPc);
                    return;
                }

                const bool isCbz = cwCbz.test(orig);
                const bool isCbnz = cwCbnz.test(orig);

                if (isCbz or isCbnz) {
                    HK_ABORT_UNLESS(isLast, "cannot return from this situation");

                    const IRegType rt = grabRd(orig);
                    const ptr addr = grabBcondCbzCbnzLdrRelativeAddr(orig, origAddr);

                    constexpr auto cbzExpr = assemble<"cbz (), #{}">();
                    constexpr auto cbnzExpr = assemble<"cbnz (), #{}">();

                    const size cbzCbnzIdx = out.size();
                    const ptr cbzCbnzPc = calcRxAddr(cbzCbnzIdx);
                    out.add(0);
                    emitter.emitBranch(addr, false);

                    const ptr returnPc = calcCurRxAddr();
                    out[cbzCbnzIdx] = isCbnz ? /* invert */
                        cbzExpr.arg(rt, returnPc).assembleOne(0, cbzCbnzPc)
                                             : cbnzExpr.arg(rt, returnPc).assembleOne(0, cbzCbnzPc);
                    return;
                }

                const bool isTbz = cwTbz.test(orig);
                const bool isTbnz = cwTbnz.test(orig);

                if (isTbz or isTbnz) {
                    HK_ABORT_UNLESS(isLast, "cannot return from this situation");

                    const IRegType rt = grabRd(orig);
                    const ptr addr = grabTbzTbnzAddr(orig, origAddr);
                    const u8 bitIdx = grabTbzTbnzBitIndex(orig);

                    constexpr auto tbzExpr = assemble<"tbz (), {}, #{}">();
                    constexpr auto tbnzExpr = assemble<"tbnz (), {}, #{}">();

                    const size tbzTbnzIdx = out.size();
                    const ptr tbzTbnzPc = calcRxAddr(tbzTbnzIdx);
                    out.add(0);
                    emitter.emitBranch(addr, false);

                    const ptr returnPc = calcCurRxAddr();
                    out[tbzTbnzIdx] = isTbnz ? /* invert */
                        tbzExpr.arg(rt, bitIdx, returnPc).assembleOne(0, tbzTbnzPc)
                                             : tbnzExpr.arg(rt, bitIdx, returnPc).assembleOne(0, tbzTbnzPc);
                    return;
                }

                HK_ABORT_UNLESS(!cwLdrLiteral.test(orig), "LDR(literal) not supported");

#endif // no specific ilp32 stuff
                out.add(orig);
            };

            for (auto [i, origInstr] : util::iterateWithIdx(orig))
                reconstruct(origInstr, origAddr + i * sizeof(Instr), i == orig.size() - 1);
            if (!skipReturn)
                emitReturn();

            return out.size();
        }

    } // namespace detail

} // namespace hk::hook
