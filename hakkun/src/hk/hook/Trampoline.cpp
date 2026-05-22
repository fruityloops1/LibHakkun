#include "hk/hook/Trampoline.h"
#include "hk/container/Array.h"
#include "hk/hook/MapUtil.h"
#include "hk/hook/a64/Assembler.h"
#include "hk/hook/a64/Instrs.h"

namespace hk::hook {

    namespace detail {

        static ptr sRwAddr = 0;
        section(.text) static constinit TrampolineBackup sTrampolinePoolData[HK_HOOK_TRAMPOLINE_POOL_SIZE];

        static void* mapRw() {
            sRwAddr = HK_UNWRAP(mapRoToRw(ptr(sTrampolinePoolData), sizeof(sTrampolinePoolData)));
            return cast<void*>(sRwAddr);
        }

        util::PoolAllocator<TrampolineBackup, HK_HOOK_TRAMPOLINE_POOL_SIZE> sTrampolinePool { cast<TrampolineBackup*>(mapRw()) };

        ptr TrampolineBackup::getRx() const {
            ptr rw = ptr(this);

            return ptr(sTrampolinePoolData) + (rw - sRwAddr);
        }

        void TrampolineBackup::make(Instr orig, ptr origAddr) {
            const ptr meRx = getRx();

            size numInstrs = 0;

            const auto calcCurOffset = [&]() -> size { return sizeof(Instr) * numInstrs; };
            const auto calcCurRxAddr = [&]() -> ptr { return meRx + calcCurOffset(); };

            const auto emitN = [&]<size N>(Array<Instr, N> instrs) {
                HK_ABORT_UNLESS(numInstrs + N - 1 < cMaxNumInstrs, "exceeded TrampolineBackup size");
                util::copy(this->instrs + numInstrs, instrs.data(), N);
                numInstrs += N;
            };

            const auto emit = [&](Instr instr) {
                HK_ABORT_UNLESS(numInstrs < cMaxNumInstrs, "exceeded TrampolineBackup size");
                this->instrs[numInstrs++] = instr;
            };

            const auto emitBranch = [&](ptr from, ptr to, bool link) {
                const s64 gap = to - from;
                HK_ABORT_UNLESS(abs(gap) <= cMaxBranchDistance, "Trampoline: Branch exceeded max branch distance (%zd > %zu)", abs(gap), cMaxBranchDistance);
                emit(link ? makeBL(from, to) : makeB(from, to));
            };

            const auto emitB = [&](ptr from, ptr to) { emitBranch(from, to, false); };
            const auto emitBL = [&](ptr from, ptr to) { emitBranch(from, to, true); };

            const auto emitReturn = [&]() { emitB(calcCurRxAddr(), origAddr + sizeof(Instr)); };

            defer { svc::clearCache(meRx, sizeof(Instr) * numInstrs); };

#if __aarch64__
            using namespace a64;

            const auto emitMovImmediate64 = [&](IRegType reg, u64 value) {
                const ptr pc = calcCurRxAddr();
                const u64 upper = value & ~bits(12);
                const u64 lower = value & bits(12);

                constexpr auto expr = assemble<R"(
                    adrp (), {}
                    add (), (), {}
                    )">();

                emitN(expr.arg(reg, upper, reg, reg, lower).assemble(pc));
            };

            const auto emitAbsBranchWithReturn = [&](ptr addr) {
                constexpr Instr cBr = assemble<"br ip">().assembleOne(0, 0);

                emitMovImmediate64(IP0, addr);
                emit(cBr);
                emitReturn();
            };

            const bool isAdr = cwAdr.test(orig);
            const bool isAdrp = cwAdrp.test(orig);

            if (isAdr or isAdrp) {
                const IRegType rd = grabRd(orig);
                const ptr addr = grabAdrAdrpAddr(orig, origAddr);

                constexpr auto adrExpr = assemble<"adr (), {}">();
                constexpr auto adrpExpr = assemble<"adrp (), {}">();

                const ptr pc = calcCurRxAddr();

                emit(isAdr
                        ? adrExpr.arg(rd, addr).assembleOne(0, pc)
                        : adrpExpr.arg(rd, addr).assembleOne(0, pc));
                emitReturn();
                return;
            }

            const bool isB = cwB.test(orig);
            const bool isBL = cwBL.test(orig);

            if (isB or isBL) {
                const ptr addr = grabBBLAddr(orig, origAddr);

                emitBranch(calcCurRxAddr(), addr, isBL);
                if (isBL)
                    emitReturn();
                return;
            }

            if (cwBcond.test(orig)) {
                const ptr addr = grabBcondCbzCbnzLdrRelativeAddr(orig, origAddr);
                const u8 condInv = grabBcondCond(orig) ^ 0b1;

                constexpr auto bcondExpr = assemble<"b.() #16">();

                emit(bcondExpr.arg(condInv).assembleOne(0, calcCurRxAddr()));
                emitAbsBranchWithReturn(addr);
                return;
            }

            const bool isCbz = cwCbz.test(orig);
            const bool isCbnz = cwCbnz.test(orig);

            if (isCbz or isCbnz) {
                const IRegType rt = grabRd(orig);
                const ptr addr = grabBcondCbzCbnzLdrRelativeAddr(orig, origAddr);

                constexpr auto cbzExpr = assemble<"cbz (), #16">();
                constexpr auto cbnzExpr = assemble<"cbnz (), #16">();

                const ptr pc = calcCurRxAddr();

                emit(isCbnz ? /* invert */ cbzExpr.arg(rt).assembleOne(0, pc) : cbnzExpr.arg(rt).assembleOne(0, pc));
                emitAbsBranchWithReturn(addr);
                return;
            }

            const bool isTbz = cwTbz.test(orig);
            const bool isTbnz = cwTbnz.test(orig);

            if (isTbz or isTbnz) {
                const IRegType rt = grabRd(orig);
                const ptr addr = grabTbzTbnzAddr(orig, origAddr);
                const u8 bitIdx = grabTbzTbnzBitIndex(orig);

                constexpr auto tbzExpr = assemble<"tbz (), {}, #16">();
                constexpr auto tbnzExpr = assemble<"tbnz (), {}, #16">();

                const ptr pc = calcCurRxAddr();

                emit(isTbnz ? /* invert */ tbzExpr.arg(rt, bitIdx).assembleOne(0, pc) : tbnzExpr.arg(rt, bitIdx).assembleOne(0, pc));
                emitAbsBranchWithReturn(addr);
                return;
            }

            HK_ABORT_UNLESS(!cwLdrLiteral.test(orig), "LDR(literal) not supported");

#endif // no specific ilp32 stuff

            emit(orig);
            emitReturn();
        }

    } // namespace detail

} // namespace hk::hook
