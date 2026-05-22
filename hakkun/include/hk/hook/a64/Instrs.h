#pragma once

#include "hk/hook/InstrUtil.h"

namespace hk::hook::a64 {

    using InstrBuilder = util::IntBuilder<Instr>;

    enum IRegType : u8 {
        X0,
        X1,
        X2,
        X3,
        X4,
        X5,
        X6,
        X7,
        X8,
        X9,
        X10,
        X11,
        X12,
        X13,
        X14,
        X15,
        X16,
        X17,
        X18,
        X19,
        X20,
        X21,
        X22,
        X23,
        X24,
        X25,
        X26,
        X27,
        X28,
        X29,
        X30,
        X31,

        IP0 = X16,
        IP1 = X17,
        FP = X29,
        LR = X30,
        XZR = X31,

        W,

        W0 = X0 | W,
        W1 = X1 | W,
        W2 = X2 | W,
        W3 = X3 | W,
        W4 = X4 | W,
        W5 = X5 | W,
        W6 = X6 | W,
        W7 = X7 | W,
        W8 = X8 | W,
        W9 = X9 | W,
        W10 = X10 | W,
        W11 = X11 | W,
        W12 = X12 | W,
        W13 = X13 | W,
        W14 = X14 | W,
        W15 = X15 | W,
        W16 = X16 | W,
        W17 = X17 | W,
        W18 = X18 | W,
        W19 = X19 | W,
        W20 = X20 | W,
        W21 = X21 | W,
        W22 = X22 | W,
        W23 = X23 | W,
        W24 = X24 | W,
        W25 = X25 | W,
        W26 = X26 | W,
        W27 = X27 | W,
        W28 = X28 | W,
        W29 = X29 | W,
        W30 = X30 | W,
        W31 = X31 | W,
        WZR = W31,

        SP,

        IRegType_Max = 0xFF & ~W
    };

    enum class IShiftType : u8 {
        LSL,
        LSR,
        ASR,
        ROR,

        Max,
    };

    constexpr WildcardInstr cwAdr = "0??10000????????????????????????";
    constexpr WildcardInstr cwAdrp = "1??10000????????????????????????";
    constexpr WildcardInstr cwB = "000101??????????????????????????";
    constexpr WildcardInstr cwBL = "100101??????????????????????????";
    constexpr WildcardInstr cwBcond = "01010100???????????????????0????";
    constexpr WildcardInstr cwCbz = "?0110100????????????????????????";
    constexpr WildcardInstr cwCbnz = "?0110101????????????????????????";
    constexpr WildcardInstr cwTbz = "?0110110????????????????????????";
    constexpr WildcardInstr cwTbnz = "?0110111????????????????????????";
    constexpr WildcardInstr cwLdrLiteral = "0?011000????????????????????????";

    constexpr bool grabSf(Instr instr) { return (instr >> 31) & 0b1; }
    constexpr bool grabLdrLiteralWidth(Instr instr) { return (instr >> 30) & 0b1; }

    constexpr IRegType grabRd(Instr instr) {
        const IRegType rd = IRegType(instr & bits(5));
        [[assume(rd <= X31)]];
        return rd;
    }

    constexpr IRegType grabRn(Instr instr) {
        const IRegType rn = IRegType((instr >> 5) & bits(5));
        [[assume(rn <= X31)]];
        return rn;
    }

    constexpr ptrdiff grabAdrAddend(Instr instr) {
        const u32 immhi = (instr >> 5) & bits(19);
        const u8 immlo = (instr >> 29) & bits(2);

        const u32 imm = (immhi << 2) | immlo;
        const s32 addend = util::signExtend(imm, 21);

        [[assume(addend >= -2097151)]];
        [[assume(addend <= 2097151)]];
        return addend;
    }

    constexpr ptr grabBcondCond(Instr instr) { return instr & bits(4); }

    constexpr u8 grabTbzTbnzBitIndex(Instr instr) {
        const u8 lower = (instr >> 19) & bits(5);
        const bool upper = (instr >> 31) & 0b1;

        const u8 idx = (upper << 5) | lower;
        return idx;
    }

    constexpr ptrdiff grabAdrpAddend(Instr instr) { return grabAdrAddend(instr) * cPageSize; }

    constexpr ptr grabAdrAdrpAddr(Instr instr, ptr pc) {
        const bool isAdr = cwAdr.test(instr);
        const bool isAdrp = cwAdrp.test(instr);

        return (isAdrp ? pc & ~bits(12) : pc) + (isAdr ? grabAdrAddend(instr) : grabAdrpAddend(instr));
    }

    constexpr ptr grabBBLAddr(Instr instr, ptr pc) {
        const u32 imm26 = instr & bits(26);
        const s32 offset = util::signExtend(imm26, 26);
        return pc + offset * sizeof(Instr);
    }

    constexpr ptr grabBcondCbzCbnzLdrRelativeAddr(Instr instr, ptr pc) {
        const u32 imm19 = (instr >> 5) & bits(19);
        const s32 offset = util::signExtend(imm19, 19);
        return pc + offset * sizeof(Instr);
    }

    constexpr ptr grabTbzTbnzAddr(Instr instr, ptr pc) {
        const u32 imm14 = (instr >> 5) & bits(14);
        const s32 offset = util::signExtend(imm14, 14);
        return pc + offset * sizeof(Instr);
    }

} // namespace hk::hook::a64
