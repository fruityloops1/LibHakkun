#include "hk/hook/Trampoline.h"
#include "hk/hook/MapUtil.h"

namespace hk::hook {

    namespace detail {

        static ptr sRwAddr = 0;
        section(.text) static TrampolineBackup sTrampolinePoolData[HK_HOOK_TRAMPOLINE_POOL_SIZE];

        static void* mapRw() {
            sRwAddr = HK_UNWRAP(mapRoToRw(ptr(sTrampolinePoolData), sizeof(sTrampolinePoolData)));
            return cast<void*>(sRwAddr);
        }

        util::PoolAllocator<TrampolineBackup, HK_HOOK_TRAMPOLINE_POOL_SIZE> sTrampolinePool { cast<TrampolineBackup*>(mapRw()) };

        ptr TrampolineBackup::getRx() const {
            ptr rw = ptr(this);

            return ptr(sTrampolinePoolData) + (rw - sRwAddr);
        }

        // ---------------------------------------------------------------
        // Prologue relocator.
        //
        // The old TrampolineHook copied the original first instruction
        // verbatim into the trampoline pool. That's correct for non
        // PC-relative encodings (stp/sub/mov/etc.) but wrong for any
        // PC-relative instruction: executing the same encoding from a
        // different PC computes a different target. On AArch64 the set
        // of PC-relative encodings that can show up as a function
        // prologue is small — adrp/adr/b/bl/b.cond/cbz/cbnz/tbz/tbnz —
        // but every one of them silently produces wrong addresses if
        // you copy verbatim, and the guest crashes the moment .orig()
        // is called.
        //
        // On AArch64 this relocator decodes the original and emits
        // equivalent code that produces the same architectural effect
        // when executed from the trampoline. For values that don't fit
        // in the original immediate field after relocation it falls
        // back to a 4-instr movz/movk address build into X16 (IP0,
        // AAPCS64 caller-saved) followed by a br/blr.
        //
        // On other architectures we preserve the previous behavior
        // (verbatim copy + back-branch). Adding equivalent relocators
        // for ARMv7-A is straightforward but left for a follow-up.
        // ---------------------------------------------------------------

#ifdef __aarch64__

        namespace {
            constexpr Instr cNopInstr = 0xd503201fu;

            constexpr bool isAdrp(Instr i)    { return (i & 0x9f000000u) == 0x90000000u; }
            constexpr bool isAdr(Instr i)     { return (i & 0x9f000000u) == 0x10000000u; }
            constexpr bool isB(Instr i)       { return (i & 0xfc000000u) == 0x14000000u; }
            constexpr bool isBL(Instr i)      { return (i & 0xfc000000u) == 0x94000000u; }
            constexpr bool isBCond(Instr i)   { return (i & 0xff000010u) == 0x54000000u; }
            constexpr bool isCbzCbnz(Instr i) { return (i & 0x7e000000u) == 0x34000000u; }
            constexpr bool isTbzTbnz(Instr i) { return (i & 0x7e000000u) == 0x36000000u; }

            constexpr int reg5(Instr i) { return int(i & 0x1fu); }

            constexpr s64 sext(u64 v, int bits) {
                u64 m = u64(1) << (bits - 1);
                return s64((v ^ m) - m);
            }
            constexpr s64 adrpTarget(Instr i, ptr pc) {
                u64 immlo = (i >> 29) & 0x3u;
                u64 immhi = (i >> 5) & 0x7ffffu;
                s64 imm = sext((immhi << 2) | immlo, 21);
                return s64(pc & ~u64(0xfff)) + (imm << 12);
            }
            constexpr s64 adrTarget(Instr i, ptr pc) {
                u64 immlo = (i >> 29) & 0x3u;
                u64 immhi = (i >> 5) & 0x7ffffu;
                return s64(pc) + sext((immhi << 2) | immlo, 21);
            }
            constexpr s64 b26Target(Instr i, ptr pc) {
                return s64(pc) + (sext(i & 0x3ffffffu, 26) << 2);
            }
            constexpr s64 b19Target(Instr i, ptr pc) {
                return s64(pc) + (sext((i >> 5) & 0x7ffffu, 19) << 2);
            }
            constexpr s64 tb14Target(Instr i, ptr pc) {
                return s64(pc) + (sext((i >> 5) & 0x3fffu, 14) << 2);
            }

            constexpr Instr makeMovz64(int Xd, u16 imm16, int hw) {
                return 0xd2800000u | (u32(hw) << 21) | (u32(imm16) << 5) | u32(Xd);
            }
            constexpr Instr makeMovk64(int Xd, u16 imm16, int hw) {
                return 0xf2800000u | (u32(hw) << 21) | (u32(imm16) << 5) | u32(Xd);
            }
            constexpr Instr makeBr(int Xn)  { return 0xd61f0000u | (u32(Xn) << 5); }
            constexpr Instr makeBlr(int Xn) { return 0xd63f0000u | (u32(Xn) << 5); }
            constexpr Instr makeB_imm26(s64 disp) {
                return 0x14000000u | (u32((disp >> 2) & 0x3ffffff));
            }
            constexpr Instr makeBL_imm26(s64 disp) {
                return 0x94000000u | (u32((disp >> 2) & 0x3ffffff));
            }
            constexpr Instr makeBCond_imm19(int cond, s64 disp) {
                return 0x54000000u | (u32((disp >> 2) & 0x7ffff) << 5) | u32(cond & 0xf);
            }
            constexpr Instr makeCb_imm19(bool nz, bool sf, int rt, s64 disp) {
                return (sf ? 0x80000000u : 0u) | 0x34000000u | (nz ? 0x01000000u : 0u)
                       | (u32((disp >> 2) & 0x7ffff) << 5) | u32(rt);
            }
            constexpr Instr makeTb_imm14(bool nz, int bit, int rt, s64 disp) {
                int b5 = (bit >> 5) & 1, b40 = bit & 0x1f;
                return (b5 ? 0x80000000u : 0u) | 0x36000000u | (nz ? 0x01000000u : 0u)
                       | (u32(b40) << 19) | (u32((disp >> 2) & 0x3fff) << 5) | u32(rt);
            }

            // Emit movz/movk*3 to fully populate `Xtemp` with `addr`. Always 4 instrs.
            inline void emitMov64(Instr* out, int Xtemp, u64 addr) {
                out[0] = makeMovz64(Xtemp, u16(addr & 0xffff), 0);
                out[1] = makeMovk64(Xtemp, u16((addr >> 16) & 0xffff), 1);
                out[2] = makeMovk64(Xtemp, u16((addr >> 32) & 0xffff), 2);
                out[3] = makeMovk64(Xtemp, u16((addr >> 48) & 0xffff), 3);
            }

            // Conditional long jump: emit
            //   <inverted-cond> +24   ; skips the 5-instr indirect jump when NOT taken
            //   movz/movk X16, target ; 4 instrs
            //   br X16                ; 1 instr
            // After the 6 emitted instrs, control falls through to b origPc+4
            // (which the caller appends). Returns the number of slots written.
            inline int emitConditionalLongJump(Instr* out, Instr invertedSkip, u64 target) {
                out[0] = invertedSkip;
                emitMov64(&out[1], 16, target);
                out[5] = makeBr(16);
                return 6;
            }
        }

        // 26-bit B/BL immediate range: ±128 MiB (bits 25..0, sign-extended, <<2).
        static constexpr s64 cBr26Min = -(s64(1) << 27);
        static constexpr s64 cBr26Max =  (s64(1) << 27) - 1;

        void installRelocatedPrologue(TrampolineBackup* backup, Instr orig, ptr origPc) {
            // Initialize all slots to NOP. If the relocator emits fewer than
            // cMaxSlots instructions and the trailing back-branch is also
            // emitted, the unused tail simply isn't reached; any defensive
            // fallthrough lands on NOPs.
            for (int i = 0; i < TrampolineBackup::cMaxSlots; ++i)
                backup->instrs[i] = cNopInstr;

            const ptr trampolinePc = backup->getRx();
            int n = 0;
            bool needsFallthrough = true;

            auto emitLongIndirect = [&](u64 target, bool isCall) {
                emitMov64(&backup->instrs[n], 16, target);
                n += 4;
                backup->instrs[n++] = isCall ? makeBlr(16) : makeBr(16);
            };

            if (isAdrp(orig) || isAdr(orig)) {
                // adrp Xd, page  /  adr Xd, addr  -> movz/movk* into Xd.
                u64 tgt = u64(isAdrp(orig) ? adrpTarget(orig, origPc)
                                            : adrTarget(orig, origPc));
                emitMov64(&backup->instrs[n], reg5(orig), tgt);
                n += 4;
            } else if (isB(orig) || isBL(orig)) {
                u64 tgt = u64(b26Target(orig, origPc));
                s64 disp = s64(tgt) - s64(trampolinePc + ptr(n) * sizeof(Instr));
                if (disp >= cBr26Min && disp <= cBr26Max) {
                    backup->instrs[n++] = isBL(orig) ? makeBL_imm26(disp)
                                                       : makeB_imm26(disp);
                } else {
                    emitLongIndirect(tgt, isBL(orig));
                }
                if (isB(orig)) needsFallthrough = false; // unconditional jump
            } else if (isBCond(orig)) {
                u64 tgt = u64(b19Target(orig, origPc));
                int cond = int(orig & 0xfu);
                Instr skip = makeBCond_imm19(cond ^ 1, 6 * sizeof(Instr));
                n += emitConditionalLongJump(&backup->instrs[n], skip, tgt);
            } else if (isCbzCbnz(orig)) {
                u64 tgt = u64(b19Target(orig, origPc));
                bool sf = ((orig >> 31) & 1u) != 0;
                bool nz = ((orig >> 24) & 1u) != 0;
                int rt = reg5(orig);
                Instr skip = makeCb_imm19(!nz, sf, rt, 6 * sizeof(Instr));
                n += emitConditionalLongJump(&backup->instrs[n], skip, tgt);
            } else if (isTbzTbnz(orig)) {
                u64 tgt = u64(tb14Target(orig, origPc));
                bool nz = ((orig >> 24) & 1u) != 0;
                int b5 = int((orig >> 31) & 1u);
                int b40 = int((orig >> 19) & 0x1fu);
                int rt = reg5(orig);
                Instr skip = makeTb_imm14(!nz, (b5 << 5) | b40, rt, 6 * sizeof(Instr));
                n += emitConditionalLongJump(&backup->instrs[n], skip, tgt);
            } else {
                // Non-PC-relative: copy verbatim. Covers stp/sub/mov/etc.
                // (LDR <reg>, <label> isn't a typical prologue but isn't
                // recognized here either — if a future hook target starts
                // with one, add a case above rather than relying on this
                // fallthrough.)
                backup->instrs[n++] = orig;
            }

            if (needsFallthrough) {
                const ptr from = trampolinePc + ptr(n) * sizeof(Instr);
                const ptr to = origPc + sizeof(Instr);
                const s64 gap = s64(to) - s64(from);
                HK_ABORT_UNLESS(abs(gap) <= cMaxBranchDistance,
                    "Trampoline: back-branch exceeded max distance (%zd > %zu)",
                    abs(gap), cMaxBranchDistance);
                backup->instrs[n++] = makeB(from, to);
            }

            svc::clearCache(trampolinePc, sizeof(TrampolineBackup));
        }

#else // !__aarch64__

        // Architecture without a relocator yet — preserve the previous
        // behavior: copy the original instruction verbatim into slot 0
        // and emit a back-branch to origPc+4 into slot 1. Execution
        // leaves the backup after slot 1, so the remaining slots are
        // never reached.
        void installRelocatedPrologue(TrampolineBackup* backup, Instr orig, ptr origPc) {
            backup->instrs[0] = orig;

            const ptr from = backup->getRx() + sizeof(Instr);
            const ptr to = origPc + sizeof(Instr);
            const s64 gap = s64(to) - s64(from);
            HK_ABORT_UNLESS(abs(gap) <= cMaxBranchDistance,
                "Trampoline: Branch exceeded max branch distance (%zd > %zu)",
                abs(gap), cMaxBranchDistance);
            backup->instrs[1] = makeB(from, to);

            svc::clearCache(backup->getRx(), sizeof(TrampolineBackup));
        }

#endif // __aarch64__

    } // namespace detail

} // namespace hk::hook
