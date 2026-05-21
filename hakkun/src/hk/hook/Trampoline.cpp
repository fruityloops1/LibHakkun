#include "hk/hook/Trampoline.h"
#include "hk/hook/MapUtil.h"
#include "hk/util/Math.h"

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
            return ptr(sTrampolinePoolData) + (ptr(this) - sRwAddr);
        }

        void TrampolineBackup::installRelocatedPrologue(Instr orig, ptr origPc) {
#ifdef __aarch64__
            using util::IntBuilder;
            using util::signExtend;

            const auto field = [](Instr i, int hi, int lo) {
                return u32((i >> lo) & u32(bits(hi - lo + 1)));
            };
            const auto isAdrp   = [&](Instr i) { return field(i, 31, 31) == 1 && field(i, 28, 24) == 0b10000; };
            const auto isAdr    = [&](Instr i) { return field(i, 31, 31) == 0 && field(i, 28, 24) == 0b10000; };
            const auto isB      = [&](Instr i) { return field(i, 31, 26) == 0b000101; };
            const auto isBL     = [&](Instr i) { return field(i, 31, 26) == 0b100101; };
            const auto isBCond  = [&](Instr i) { return field(i, 31, 24) == 0b01010100 && field(i, 4, 4) == 0; };
            const auto isCbz    = [&](Instr i) { return field(i, 30, 24) == 0b0110100; };
            const auto isCbnz   = [&](Instr i) { return field(i, 30, 24) == 0b0110101; };
            const auto isTbz    = [&](Instr i) { return field(i, 30, 24) == 0b0110110; };
            const auto isTbnz   = [&](Instr i) { return field(i, 30, 24) == 0b0110111; };
            const auto isLdrLit = [&](Instr i) { return field(i, 29, 24) == 0b011000; };

            const auto calcAdrpTarget = [&](Instr i, ptr pc) {
                const s64 imm = signExtend<s64>((field(i, 23, 5) << 2) | field(i, 30, 29), 21);
                return ptr(s64(pc & ~ptr(bits(12))) + (imm << 12));
            };
            const auto calcAdrTarget = [&](Instr i, ptr pc) {
                const s64 imm = signExtend<s64>((field(i, 23, 5) << 2) | field(i, 30, 29), 21);
                return ptr(s64(pc) + imm);
            };
            const auto calcB26Target = [&](Instr i, ptr pc) {
                return ptr(s64(pc) + (signExtend<s64>(field(i, 25, 0), 26) << 2));
            };
            const auto calcB19Target = [&](Instr i, ptr pc) {
                return ptr(s64(pc) + (signExtend<s64>(field(i, 23, 5), 19) << 2));
            };
            const auto calcTb14Target = [&](Instr i, ptr pc) {
                return ptr(s64(pc) + (signExtend<s64>(field(i, 18, 5), 14) << 2));
            };

            const auto makeMovz64 = [](int xd, u16 imm16, int hw) -> Instr {
                return IntBuilder<Instr>().set(31, 23, 0b110100101).set(22, 21, hw).set(20, 5, imm16).set(4, 0, xd);
            };
            const auto makeMovk64 = [](int xd, u16 imm16, int hw) -> Instr {
                return IntBuilder<Instr>().set(31, 23, 0b111100101).set(22, 21, hw).set(20, 5, imm16).set(4, 0, xd);
            };
            const auto makeBr  = [](int xn) -> Instr { return IntBuilder<Instr>().set(31, 10, 0b1101011000011111000000).set(9, 5, xn); };
            const auto makeBlr = [](int xn) -> Instr { return IntBuilder<Instr>().set(31, 10, 0b1101011000111111000000).set(9, 5, xn); };
            const auto makeBCondImm19 = [](int cond, s64 disp) -> Instr {
                return IntBuilder<Instr>().set(31, 24, 0b01010100).set(23, 5, (disp >> 2) & bits(19)).set(3, 0, cond);
            };
            const auto makeCbImm19 = [](bool nz, bool sf, int rt, s64 disp) -> Instr {
                return IntBuilder<Instr>().set(31, 31, sf).set(30, 25, 0b011010).set(24, 24, nz).set(23, 5, (disp >> 2) & bits(19)).set(4, 0, rt);
            };
            const auto makeTbImm14 = [](bool nz, int bitIdx, int rt, s64 disp) -> Instr {
                return IntBuilder<Instr>().set(31, 31, (bitIdx >> 5) & 1).set(30, 25, 0b011011).set(24, 24, nz).set(23, 19, bitIdx & bits(5)).set(18, 5, (disp >> 2) & bits(14)).set(4, 0, rt);
            };

            const auto emitMov64 = [&](Instr out[4], int xd, u64 addr) {
                out[0] = makeMovz64(xd, u16(addr),       0);
                out[1] = makeMovk64(xd, u16(addr >> 16), 1);
                out[2] = makeMovk64(xd, u16(addr >> 32), 2);
                out[3] = makeMovk64(xd, u16(addr >> 48), 3);
            };
            const auto emitConditionalLongJump = [&](Instr out[6], Instr invertedSkip, u64 target) {
                out[0] = invertedSkip;
                emitMov64(&out[1], 16, target);
                out[5] = makeBr(16);
            };

            const ptr trampolinePc = getRx();
            int n = 0;
            bool needsFallthrough = true;

            const auto emitLongIndirect = [&](ptr target, bool isCall) {
                emitMov64(&instrs[n], 16, target);
                n += 4;
                instrs[n++] = isCall ? makeBlr(16) : makeBr(16);
            };

            if (isAdrp(orig) || isAdr(orig)) {
                const ptr target = isAdrp(orig) ? calcAdrpTarget(orig, origPc) : calcAdrTarget(orig, origPc);
                emitMov64(&instrs[n], int(field(orig, 4, 0)), target);
                n += 4;
            } else if (isB(orig) || isBL(orig)) {
                const ptr target = calcB26Target(orig, origPc);
                const ptr branchSrc = trampolinePc + n * sizeof(Instr);
                const ptrdiff displacement = ptrdiff(target) - ptrdiff(branchSrc);
                if (abs(displacement) <= cMaxBranchDistance) {
                    instrs[n++] = isBL(orig) ? makeBL(branchSrc, target) : makeB(branchSrc, target);
                } else {
                    emitLongIndirect(target, isBL(orig));
                }
                if (isB(orig)) needsFallthrough = false;
            } else if (isBCond(orig)) {
                const ptr target = calcB19Target(orig, origPc);
                const int condition = field(orig, 3, 0);
                const Instr skip = makeBCondImm19(condition ^ 1, 6 * sizeof(Instr));
                emitConditionalLongJump(&instrs[n], skip, target);
                n += 6;
            } else if (isCbz(orig) || isCbnz(orig)) {
                const ptr target = calcB19Target(orig, origPc);
                const bool sf = field(orig, 31, 31);
                const bool nz = isCbnz(orig);
                const Instr skip = makeCbImm19(!nz, sf, field(orig, 4, 0), 6 * sizeof(Instr));
                emitConditionalLongJump(&instrs[n], skip, target);
                n += 6;
            } else if (isTbz(orig) || isTbnz(orig)) {
                const ptr target = calcTb14Target(orig, origPc);
                const bool nz = isTbnz(orig);
                const int bitIdx = (field(orig, 31, 31) << 5) | field(orig, 23, 19);
                const Instr skip = makeTbImm14(!nz, bitIdx, field(orig, 4, 0), 6 * sizeof(Instr));
                emitConditionalLongJump(&instrs[n], skip, target);
                n += 6;
            } else {
                HK_ABORT_UNLESS(!isLdrLit(orig), "Trampoline: LDR (literal) prologue not supported");
                instrs[n++] = orig;
            }

            if (needsFallthrough) {
                const ptr from = trampolinePc + n * sizeof(Instr);
                const ptr to = origPc + sizeof(Instr);
                const ptrdiff gap = ptrdiff(to) - ptrdiff(from);
                HK_ABORT_UNLESS(abs(gap) <= cMaxBranchDistance,
                    "Trampoline: back-branch exceeded max distance (%zd > %zu)", abs(gap), cMaxBranchDistance);
                instrs[n++] = makeB(from, to);
            }
#else
            instrs[0] = orig;
            const ptr from = getRx() + sizeof(Instr);
            const ptr to = origPc + sizeof(Instr);
            const ptrdiff gap = ptrdiff(to) - ptrdiff(from);
            HK_ABORT_UNLESS(abs(gap) <= cMaxBranchDistance,
                "Trampoline: branch exceeded max distance (%zd > %zu)", abs(gap), cMaxBranchDistance);
            instrs[1] = makeB(from, to);
#endif
            svc::clearCache(getRx(), sizeof(TrampolineBackup));
        }

    } // namespace detail

} // namespace hk::hook
