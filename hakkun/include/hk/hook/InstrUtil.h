#pragma once

#include "hk/diag/diag.h"
#include "hk/hook/results.h"
#include "hk/ro/RoModule.h"
#include "hk/ro/RoUtil.h"
#include "hk/sail/detail.h"
#include "hk/svc/api.h"
#include "hk/svc/types.h"
#include "hk/types.h"
#include "hk/util/TemplateString.h"
#include "hk/util/hash.h"

namespace hk::hook {

#ifdef __aarch64__
    using Instr = u32;

    namespace detail {

        enum UnconditionalBranchOp
        {
            Branch = 0b000101,
            BranchLink = 0b100101,
        };

        constexpr u32 cImm26Mask = 0b11111111111111111111111111;

        constexpr Instr makeUnconditionalBranchRelative(UnconditionalBranchOp operand, s32 branchToRelativeAddress) {
            branchToRelativeAddress /= 4;

            return (branchToRelativeAddress & cImm26Mask) | (operand << 26);
        }

        constexpr Instr makeUnconditionalBranch(UnconditionalBranchOp operand, ptr pcAddr, ptr branchToAddr) {
            s32 branchToRelativeAddress = branchToAddr - pcAddr;

            return makeUnconditionalBranchRelative(operand, branchToRelativeAddress);
        }

        inline Result writeUnconditionalBranch(UnconditionalBranchOp operand, const ro::RoModule* module, ptr offset, ptr branchToAddr) {
            return module->writeRo(offset, makeUnconditionalBranch(operand, module->range().start() + offset, branchToAddr));
        }

    } // namespace detail

#define _HK_HOOK_DETAIL_WRITEFUNC(NAME, IMPLFUNC, ...)                                               \
    template <typename Func>                                                                         \
    hk_alwaysinline Result write##NAME(const ro::RoModule* module, ptr offset, Func* branchToFunc) { \
        return IMPLFUNC(__VA_ARGS__);                                                                \
    }                                                                                                \
    template <typename Func>                                                                         \
    hk_alwaysinline Result write##NAME##AtPtr(ptr addr, Func* branchToFunc) {                        \
        auto* module = ro::getModuleContaining(addr);                                                \
        ptr offset = addr - module->range().start();                                                 \
        return write##NAME(module, offset, branchToFunc);                                            \
    }                                                                                                \
    template <util::TemplateString Symbol, typename Func>                                            \
    hk_alwaysinline Result write##NAME##AtSym(Func* branchToFunc) {                                  \
        ptr addr;                                                                                    \
        if constexpr (sail::sUsePrecalcHashes) {                                                     \
            constexpr u32 symMurmur = util::hashMurmur(Symbol.value);                                \
            addr = sail::lookupSymbolFromDb<true>(&symMurmur);                                       \
        } else {                                                                                     \
            addr = sail::lookupSymbolFromDb<false>(Symbol.value);                                    \
        }                                                                                            \
                                                                                                     \
        return write##NAME##AtPtr(addr, branchToFunc);                                               \
    }

    _HK_HOOK_DETAIL_WRITEFUNC(Branch, detail::writeUnconditionalBranch, detail::UnconditionalBranchOp::Branch, module, offset, ptr(branchToFunc));
    _HK_HOOK_DETAIL_WRITEFUNC(BranchLink, detail::writeUnconditionalBranch, detail::UnconditionalBranchOp::BranchLink, module, offset, ptr(branchToFunc));

    constexpr Instr makeB(ptr pcAddr, ptr branchToAddr) { return detail::makeUnconditionalBranch(detail::UnconditionalBranchOp::Branch, pcAddr, branchToAddr); }
    constexpr Instr makeBL(ptr pcAddr, ptr branchToAddr) { return detail::makeUnconditionalBranch(detail::UnconditionalBranchOp::BranchLink, pcAddr, branchToAddr); }

#undef _HK_HOOK_DETAIL_WRITEFUNC

    hk_alwaysinline inline Result readADRPGlobal(ptr* out, Instr* at, s32 loInstrOffset = 1) {
        ptr pc = ptr(at);
        Instr adrpInstr = at[0];

        HK_UNLESS(((adrpInstr >> 31) & 0b1) == 1, ResultMismatchedInstruction());
        HK_UNLESS(((adrpInstr >> 24) & 0b11111) == 0b10000, ResultMismatchedInstruction());

        u32 immhi = (adrpInstr >> 5) & 0b111111111111111111;
        u32 immlo = (adrpInstr >> 29) & 0b11;

        s64 offset = ((immhi << 2) | immlo) << 12;

        if (offset & (1LL << 32)) // sext
            offset |= 0b11111111111100000000000000000000;

        ptr page = (pc & ~0xFFF) + offset;

        Instr loInstr = at[loInstrOffset];

        if (
            ((loInstr >> 31) & 0b1) == 0b1
            && ((loInstr >> 22) & 0b11111111) == 0b11100101) { // LDR Unsigned Offset
            u32 imm12 = (loInstr >> 10) & 0b111111111111;
            u32 size = (loInstr >> 30) & 0b11;

            u32 addend = imm12 << size;

            ptr* globalPtr = cast<ptr*>(page + addend);

            svc::MemoryInfo info;
            u32 pageInfo;
            svc::QueryMemory(&info, &pageInfo, ptr(globalPtr));
            HK_UNLESS(info.state != 0 && info.permission & svc::MemoryPermission_Read, ResultInvalidRead());

            *out = *globalPtr;
        } else if (
            ((loInstr >> 23) & 0b11111111) == 0b00100010) { // ADD immediate
            bool sh = (loInstr >> 22) & 0b1;
            u32 imm12 = (loInstr >> 10) & 0b111111111111;

            u32 offset = sh ? imm12 << 12 : imm12;
            *out = page + offset;
        } else
            return ResultMismatchedInstruction();
        return ResultSuccess();
    }

#else
#error unsupported architecture
#endif

} // namespace hk::hook
