#pragma once

#include "hk/container/Array.h"
#include "hk/container/VecSpan.h"
#include "hk/diag/diag.h"
#include "hk/hook/InstrUtil.h"
#include "hk/hook/a64/Instrs.h"
#include "hk/hook/results.h"
#include "hk/prim/iterator/WithIndex.h"
#include "hk/ro/RoModule.h"
#include "hk/types.h"
#include "hk/util/Context.h"
#include "hk/util/Math.h"
#include "hk/util/TemplateString.h"
#include "hk/util/Tuple.h"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <tuple>
#include <utility>

namespace hk::hook::a64 {

    template <size N>
    struct Err : util::TemplateString<N> {
        template <typename... Args>
        constexpr Err(const char (&str)[N], Args...)
            : util::TemplateString<N>(str) {
            if (__builtin_is_constant_evaluated()) {
                char error[0];
                (void)error[1];
            } else {
                HK_ABORT("Assembler Error: %s", this->value);
            }
        }
    };

    class ArgGetter {
        const u64* mArgs = nullptr;

    public:
        constexpr ArgGetter(const u64* args)
            : mArgs(args) { }

        constexpr u64 get(size idx) const { return mArgs[idx]; }
    };

    struct IImm {
        enum _CreateArg {
            CreateArg
        };
        enum _CreateRegArg {
            CreateRegArg
        };
        enum Type : u8 {
            None,
            Imm,
            Arg,
            RegImm,
            RegArg,
        };

        u64 valueImm = 0;
        Type type = None;

        constexpr u64 resolve(const ArgGetter* args) const {
            switch (type) {
            case RegImm:
            case Imm:
                return valueImm;
            case RegArg:
            case Arg:
                if (args != nullptr)
                    return args->get(valueImm);
            default:
                return 0;
            }
        }

        constexpr IImm() = default;
        constexpr IImm(u64 value)
            : type(Imm)
            , valueImm(value) {
        }

        constexpr IImm(IRegType reg)
            : type(RegImm)
            , valueImm(u64(reg)) { }

        constexpr IImm(_CreateArg, size argIdx)
            : type(Arg)
            , valueImm(argIdx) {
        }

        constexpr IImm(_CreateRegArg, size argIdx)
            : type(RegArg)
            , valueImm(argIdx) {
        }

        constexpr bool isValid() const { return type != None; }
        constexpr bool isImm() const { return type == Imm or type == RegImm; }
        constexpr bool isReg() const { return type >= RegImm; }
    } __attribute__((packed));

    enum class IInstrType : u8 {
        Nop,
        Ret,
        Br,
        Blr,
        Svc,
        AddImm,
        MovToFromSp = AddImm,
        SubImm,
        AddReg,
        SubReg,
        MovImm,
        Movz = MovImm,
        Movn = MovImm,
        Movk,
        OrrReg,
        AndReg,
        Adr,
        Adrp,
        Bcond,
        Cbz,
        Cbnz,
        Tbz,
        Tbnz,
        LdrLiteral,
    };

    constexpr u64 abs(s64 value) {
        if (value < 0)
            return -value;
        return value;
    }

    constexpr u8 reg(IRegType reg) {
        return static_cast<u8>(reg) & 0b11111;
    }
    constexpr bool isW(IRegType reg) {
        return static_cast<u8>(reg) & static_cast<u8>(IRegType::W);
    }
    constexpr bool isX(IRegType reg) {
        return !isW(reg);
    }

    template <bool Neg, size Width, size NumShift>
    constexpr bool canBeImm(u64 value) {
        if (util::isRepresentable(Width, value, NumShift))
            return true;

        if (Neg && util::isRepresentable(Width, ~value, NumShift))
            return true;
        return false;
    }

    constexpr auto canBeImm16Shift4xNeg = canBeImm<true, 16, 4>;
    constexpr auto canBeImm12Shift2x = canBeImm<false, 12, 2>;
    constexpr auto canBeImm21Neg = canBeImm<true, 21, 1>;
    constexpr auto canBeImm19Neg = canBeImm<true, 19, 1>;
    constexpr auto canBeImm14Neg = canBeImm<true, 14, 1>;
    constexpr auto canBeImm6 = canBeImm<false, 6, 1>;

    struct IInstr {
        IInstrType type;
        IImm rn = IRegType_Max;
        IImm rd = IRegType_Max;
        IImm rm = IRegType_Max;
        IShiftType shift = IShiftType::LSL;
        IImm imm;
        IImm shiftImm = 0;

        constexpr static u32 cOpNop = 0xD503201F;
        constexpr static u32 cOpRet = 0b1101011001011111000000;
        constexpr static u32 cOpSvc = 0b11010100000000000000000000000001;
        constexpr static u32 cOpAddImm = 0b00100010;
        constexpr static u32 cOpSubImm = 0b10100010;
        constexpr static u32 cOpAddReg = 0b0001011;
        constexpr static u32 cOpSubReg = 0b1001011;
        constexpr static u32 cOpAndImm = 0b00100100;
        constexpr static u32 cOpAndReg = 0b0001010;
        constexpr static u32 cOpMovz = 0b10100101;
        constexpr static u32 cOpMovn = 0b00100101;
        constexpr static u32 cOpMovk = 0b11100101;
        constexpr static u32 cOpOrrReg = 0b0101010;
        constexpr static u32 cOpBr = 0b1101011000011111000000;
        constexpr static u32 cOpBlr = 0b1101011000111111000000;
        constexpr static u32 cOpAdr = 0b00010000;
        constexpr static u32 cOpAdrp = 0b10010000;
        constexpr static u32 cOpBcond = 0b01010100;
        constexpr static u32 cOpCbz = 0b0110100;
        constexpr static u32 cOpCbnz = 0b0110101;
        constexpr static u32 cOpTbz = 0b0110110;
        constexpr static u32 cOpTbnz = 0b0110111;
        constexpr static u32 cOpLdrLiteral = 0b011000;

        constexpr static u32 cOpInvalid = 0xFFFFFFFF;

        constexpr IInstr()
            : type(IInstrType::AddImm) {
        }

    private:
    public:
        constexpr Instr hk_alwaysinline assemble(ptr addr, const ArgGetter* args) const {
            const u64 rdImm = this->rd.resolve(args);
            const u64 rnImm = this->rn.resolve(args);
            const u64 rmImm = this->rm.resolve(args);

            HK_ABORT_UNLESS(rdImm < IRegType_Max, "rd(%zu) out of range", rdImm);
            HK_ABORT_UNLESS(rnImm < IRegType_Max, "rn(%zu) out of range", rnImm);
            HK_ABORT_UNLESS(rmImm < IRegType_Max, "rm(%zu) out of range", rmImm);

            IRegType rd = IRegType(rdImm);
            IRegType rn = IRegType(rnImm);
            IRegType rm = IRegType(rmImm);
            if (rd == SP)
                rd = X31;
            if (rn == SP)
                rn = X31;

            switch (type) {
            case IInstrType::Nop: {
                return InstrBuilder()
                    .set(31, 0, cOpNop);
            }
            case IInstrType::Ret: {
                HK_ABORT_UNLESS(!(rn & W), "Ret: X register required");

                rn = rn == IRegType_Max ? LR : rn;

                return InstrBuilder()
                    .set(31, 10, cOpRet)
                    .set(9, 5, reg(rn))
                    .set(4, 0, 0);
            }
            case IInstrType::Br: {
                HK_ABORT_UNLESS(!(rn & W), "Br: X register required");
                return InstrBuilder()
                    .set(31, 10, cOpBr)
                    .set(9, 5, reg(rn))
                    .set(4, 0, 0);
            }
            case IInstrType::Blr: {
                HK_ABORT_UNLESS(!(rn & W), "Blr: X register required");
                return InstrBuilder()
                    .set(31, 10, cOpBlr)
                    .set(9, 5, reg(rn))
                    .set(4, 0, 0);
            }
            case IInstrType::Svc: {
                const u16 immV = imm.resolve(args);

                return InstrBuilder()
                    .set(31, 0, cOpSvc)
                    .set(20, 5, immV);
            }
            case IInstrType::AddImm:
            case IInstrType::SubImm: {
                const u32 immV = imm.resolve(args);

                HK_ABORT_UNLESS(canBeImm12Shift2x(immV), "AddImm: Immediate out of range (0x%x)", immV);

                return InstrBuilder()
                    .set(31, 31, isX(rd))
                    .set(30, 23, type == IInstrType::AddImm ? cOpAddImm : cOpSubImm)
                    .set(22, 22, immV > bits(12))
                    .set(21, 10, immV >> (immV > bits(12) ? 12 : 0))
                    .set(9, 5, reg(rn))
                    .set(4, 0, reg(rd));
            }
            case IInstrType::MovImm: {
                u64 immV = imm.resolve(args);

                HK_ABORT_UNLESS(canBeImm16Shift4xNeg(immV), "MovImm: Immediate out of range (0x%zx)", immV);

                const u64 invImmV = ~immV;
                bool useMovn = false;
                if (util::countSetBits(immV) > util::countSetBits(invImmV)) {
                    useMovn = true;
                    immV = invImmV;
                }

                const u32 hw = (immV > bits(48)) ? 48
                    : (immV > bits(32))          ? 32
                    : (immV > bits(16))          ? 16
                                                 : 0;

                return InstrBuilder()
                    .set(31, 31, isX(rd))
                    .set(30, 23, useMovn ? cOpMovn : cOpMovz)
                    .set(22, 21, hw / 16)
                    .set(20, 5, (immV >> hw) & bits(16))
                    .set(4, 0, reg(rd));
            }
            case IInstrType::Movk: {
                const u64 immV = imm.resolve(args);
                const u64 hw = shiftImm.resolve(args);

                HK_ABORT_UNLESS(canBeImm16Shift4xNeg(immV), "Movk: Immediate out of range (0x%zx)", immV);
                HK_ABORT_UNLESS(hw == 0 or hw == 16 or hw == 32 or hw == 48, "Movk: Shift out of range (0x%zx)", hw);

                return InstrBuilder()
                    .set(31, 31, isX(rd))
                    .set(30, 23, cOpMovk)
                    .set(22, 21, hw / 16)
                    .set(20, 5, immV)
                    .set(4, 0, reg(rd));
            }
            case IInstrType::SubReg:
            case IInstrType::AddReg: {
                u32 shiftV = shiftImm.resolve(args);
                return InstrBuilder()
                    .set(31, 31, isX(rd))
                    .set(30, 24, type == IInstrType::AddReg ? cOpAddReg : cOpSubReg)
                    .set(23, 22, u8(shift))
                    .set(21, 21, 0)
                    .set(20, 16, reg(rm))
                    .set(15, 10, shiftV & bits(6))
                    .set(9, 5, reg(rn))
                    .set(4, 0, reg(rd));
            }
            case IInstrType::AndReg: {
                HK_ABORT_UNLESS((rd & W) == (rn & W), "AndReg: mismatching register widths (rd: %d, rn: %d)", rd, rn);
                HK_ABORT_UNLESS((rd & W) == (rm & W), "AndReg: mismatching register widths (rd: %d, rm: %d)", rd, rm);

                u32 shiftV = shiftImm.resolve(args);

                return InstrBuilder() //, penis
                    .set(31, 31, isX(rd))
                    .set(30, 24, cOpAndReg)
                    .set(23, 22, u8(shift))
                    .set(21, 21, 0)
                    .set(20, 16, reg(rm))
                    .set(15, 10, shiftV & bits(6))
                    .set(9, 5, reg(rn))
                    .set(4, 0, reg(rd));
            }
            case IInstrType::OrrReg: {
                HK_ABORT_UNLESS((rd & W) == (rn & W), "OrrReg: mismatching register widths (rd: %d, rn: %d)", rd, rn);
                HK_ABORT_UNLESS((rd & W) == (rm & W), "OrrReg: mismatching register widths (rd: %d, rm: %d)", rd, rm);
                HK_ABORT_UNLESS(rd != SP && rn != SP && rm != SP, "OrrReg: SP not allowed");

                u32 shiftV = shiftImm.resolve(args);
                return InstrBuilder()
                    .set(31, 31, isX(rd))
                    .set(30, 24, cOpOrrReg)
                    .set(23, 22, u8(shift))
                    .set(21, 21, 0)
                    .set(20, 16, reg(rm))
                    .set(15, 10, shiftV & bits(6))
                    .set(9, 5, reg(rn))
                    .set(4, 0, reg(rd));
            }
            case IInstrType::Adr:
            case IInstrType::Adrp: {
                HK_ABORT_UNLESS(!(rd & W), "X register required");

                const u64 immV = imm.resolve(args);
                const bool isAdrp = type == IInstrType::Adrp;

                if (isAdrp)
                    HK_ABORT_UNLESS(isAlignedPage(immV), "Adrp: Immediate not aligned (0x%zx)", immV);

                const u64 base = isAdrp ? addr & ~bits(12) : addr;
                const s64 diff = immV - base;

                const u64 diffImm = isAdrp ? diff >> 12 : diff;
                HK_ABORT_UNLESS(canBeImm21Neg(diffImm), "Adr/Adrp: Immediate out of range (0x%zx)", immV);

                const u8 immlo = diffImm & bits(2);
                const u32 immhi = (diffImm >> 2) & bits(24);

                return InstrBuilder()
                    .set(31, 24, isAdrp ? cOpAdrp : cOpAdr)
                    .set(30, 29, immlo)
                    .set(23, 5, immhi)
                    .set(4, 0, reg(rd));
            }
            case IInstrType::Bcond: {
                const u64 immV = imm.resolve(args);
                const u64 base = addr & ~bits(2);
                const s64 diff = (immV - base) / sizeof(Instr);

                HK_ABORT_UNLESS(canBeImm19Neg(diff), "B.cond: Immediate out of range (0x%zx, pc: 0x%zx)", immV, base);

                const u64 condV = shiftImm.resolve(args);
                HK_ABORT_UNLESS(condV <= bits(4), "B.cond: Cond out of range (%zu)", condV);

                return InstrBuilder()
                    .set(31, 24, cOpBcond)
                    .set(23, 5, static_cast<u32>(diff))
                    .set(4, 4, 0)
                    .set(3, 0, condV);
            }
            case IInstrType::Cbz:
            case IInstrType::Cbnz: {
                const u64 immV = imm.resolve(args);
                const u64 base = addr & ~bits(2);
                const s64 diff = (immV - base) / sizeof(Instr);

                HK_ABORT_UNLESS(canBeImm19Neg(diff), "Cbz/Cbnz: Immediate out of range (0x%zx, pc: 0x%zx)", immV, base);

                return InstrBuilder()
                    .set(31, 31, isX(rd))
                    .set(30, 24, type == IInstrType::Cbz ? cOpCbz : cOpCbnz)
                    .set(23, 5, static_cast<u32>(diff))
                    .set(4, 0, rd);
            }
            case IInstrType::Tbz:
            case IInstrType::Tbnz: {
                const u64 immV = imm.resolve(args);
                const u64 base = addr & ~bits(2);
                const s64 diff = (immV - base) / sizeof(Instr);

                HK_ABORT_UNLESS(canBeImm14Neg(diff), "Tbz/Tbnz: Immediate out of range (0x%zx, pc: 0x%zx)", immV, base);

                const u32 shiftV = shiftImm.resolve(args);
                HK_ABORT_UNLESS(shiftV <= 63, "Tbz/Tbnz: BitPos out of range (%u)", shiftV);

                return InstrBuilder()
                    .set(31, 31, bool(shiftV & bit(5)))
                    .set(30, 24, type == IInstrType::Tbz ? cOpTbz : cOpTbnz)
                    .set(23, 19, shiftV & bits(5))
                    .set(18, 5, static_cast<u16>(diff))
                    .set(4, 0, rd);
            }
            case IInstrType::LdrLiteral: {
                const u64 immV = imm.resolve(args);
                const u64 base = addr & ~bits(2);
                const s64 diff = (immV - base) / sizeof(Instr);

                HK_ABORT_UNLESS(canBeImm19Neg(diff), "LdrLiteral: Immediate out of range (0x%zx, pc: 0x%zx)", immV, base);

                return InstrBuilder()
                    .set(31, 30, isX(rd))
                    .set(30, 24, cOpLdrLiteral)
                    .set(23, 5, static_cast<u32>(diff))
                    .set(4, 0, rd);
            }
            default:
                return 0;
            }
        }
    };

    template <size N>
    struct Expr : util::TemplateString<N> {
        constexpr static size cLineBufferSize = 1024;
        constexpr static size cErrBufSize = 1024;

        template <util::TemplateString Delims>
        constexpr static bool isDelimiter(char c) {
            for (size i = 0; Delims.value[i] != '\0'; ++i) {
                if (c == Delims.value[i])
                    return true;
            }
            return false;
        };

        template <util::TemplateString Delims, typename Func>
        constexpr static void executeCallbackBySplit(const char* str, size len, Func&& func) {
            size idx = 0;

            while (idx < len) {
                while (isDelimiter<Delims>(str[idx]))
                    idx++;
                if (str[idx] == '\0')
                    return;

                char buf[cLineBufferSize];
                size bufIdx = 0;
                while (!isDelimiter<Delims>(str[idx + bufIdx]) && str[idx + bufIdx] != '\0')
                    buf[bufIdx++] = str[idx + bufIdx];
                buf[bufIdx] = '\0';

                if (bufIdx)
                    func(buf, bufIdx);

                idx += bufIdx;
            }
        }

        constexpr static bool isEqualStringIgnoreCase(const char* a, const char* b) {
            if (*a == '\0' || *b == '\0')
                return false;

            if (!a || !b)
                return a == b;

            while (*a && *b) {
                char lowerA = (*a >= 'A' && *a <= 'Z') ? (*a + ('a' - 'A')) : *a;
                char lowerB = (*b >= 'A' && *b <= 'Z') ? (*b + ('a' - 'A')) : *b;

                if (lowerA != lowerB)
                    return false;

                ++a;
                ++b;
            }

            return *a == *b;
        }

        enum class ParseMnemonic {
            None,
            Nop,
            Ret,
            Br,
            Blr,
            Svc,
            Mov,
            Movk,
            Orr,
            Add,
            Sub,
            And,
            Adr,
            Adrp,
            BcondArg,
            Cbz,
            Cbnz,
            Tbz,
            Tbnz,
            Ldr,
        };

        enum {
            End = 0,
            Rd = bit(0),
            Rn = bit(1),
            Rm = bit(2),
            Imm = bit(3),
            ShiftType = bit(4),
            Shift = bit(5),

            Optional = bit(6),
        };

        constexpr static size cMaxOperands = 5;

        using Mnemonic = Tuple<ParseMnemonic, const char*, Array<u8, cMaxOperands + 1>>;

        constexpr static Mnemonic cMnemonics[] {
            { ParseMnemonic::None, "\0", { } },
            { ParseMnemonic::Nop, "nop", { } },
            { ParseMnemonic::Ret, "ret", { Rn | Optional } },
            { ParseMnemonic::Br, "br", { Rn } },
            { ParseMnemonic::Blr, "blr", { Rn } },
            { ParseMnemonic::Svc, "svc", { Imm } },
            { ParseMnemonic::Mov, "mov", { Rd, Rn | Imm } },
            { ParseMnemonic::Movk, "movk", { Rd, Imm, ShiftType | Optional, Shift | Optional } },
            { ParseMnemonic::Orr, "orr", { Rd, Rn, Rm, ShiftType | Optional, Shift | Optional } },
            { ParseMnemonic::Add, "add", { Rd, Rn, Rm | Imm, ShiftType | Optional, Shift | Optional } },
            { ParseMnemonic::Sub, "sub", { Rd, Rn, Rm | Imm, ShiftType | Optional, Shift | Optional } },
            { ParseMnemonic::And, "and", { Rd, Rn, Rm /* | Imm*/, ShiftType | Optional, Shift | Optional } },
            { ParseMnemonic::Adr, "adr", { Rd, Imm } },
            { ParseMnemonic::Adrp, "adrp", { Rd, Imm } },
            { ParseMnemonic::BcondArg, "b.()", { Imm } },
            { ParseMnemonic::Cbz, "cbz", { Rd, Imm } },
            { ParseMnemonic::Cbnz, "cbnz", { Rd, Imm } },
            { ParseMnemonic::Tbz, "tbz", { Rd, Shift, Imm } },
            { ParseMnemonic::Tbnz, "tbnz", { Rd, Shift, Imm } },
            { ParseMnemonic::Ldr, "ldr", { Rd, Imm } },
        };

        constexpr int countOptionalOperands(const Mnemonic& m) const {
            int count = 0;
            for (size i = 0; i < cMaxOperands; i++) {
                if (m.c[i] & Optional)
                    count++;
            }
            return count;
        }

        constexpr static const Mnemonic& findMnemonic(const char* opcodeStr) {
            for (const auto& m : cMnemonics) {
                if (isEqualStringIgnoreCase(m.b, opcodeStr))
                    return m;
            }
            return cMnemonics[0];
        }

        template <bool Throw, size EN, typename... Args>
        constexpr static void throwErr(const char (&str)[EN], Args... args) {
            if (Throw)
                Err(str, args...);
        };

        template <bool Throw, size BufferSize>
        constexpr void processInstrs(Array<IInstr, BufferSize>& instrs, int* outSize) const {
            *outSize = 0;
            size argIndex = 0;
            executeCallbackBySplit<"\n">(this->value, N, [&](const char* line, size lenLine) {
                constexpr static char cOperandDelims[] = " ,#\t\v\r";

                const Mnemonic* m = &cMnemonics[0];
                IImm rd, rn, rm;
                IImm imm;
                IShiftType shift = IShiftType::LSL;
                IImm shiftImm;

                auto checkImmRange = [&](const IImm& imm, auto check) {
                    if (imm.isImm()) {
                        if (!check(imm.resolve(nullptr))) {
                            throwErr<Throw>("Immediate out of range");
                        }
                    }
                };

                auto finalize = [&]() -> IInstr {
                    IInstr instr;
                    const auto type = m->a;

                    const IRegType rdImmVal = IRegType(rd.resolve(nullptr));
                    const IRegType rnImmVal = IRegType(rn.resolve(nullptr));
                    const IRegType rmImmVal = IRegType(rm.resolve(nullptr));

                    instr.rd = rd;
                    instr.rn = rn;
                    instr.rm = rm;
                    switch (type) {
                    case ParseMnemonic::Nop: {
                        instr.type = IInstrType::Nop;
                        break;
                    }
                    case ParseMnemonic::Ret: {
                        instr.type = IInstrType::Ret;
                        break;
                    }
                    case ParseMnemonic::Br:
                    case ParseMnemonic::Blr: {
                        instr.type = m->a == ParseMnemonic::Br ? IInstrType::Br : IInstrType::Blr;
                        break;
                    }
                    case ParseMnemonic::Svc: {
                        if (imm.isImm() && imm.resolve(nullptr) > bits(16)) {
                            throwErr<Throw>("SVC imm16 out of range", imm.resolve(nullptr));
                            return { };
                        }

                        instr.type = IInstrType::Svc;
                        instr.imm = imm;
                        break;
                    }
                    case ParseMnemonic::Mov: {
                        if (!rn.isValid() && imm.isValid()) {
                            instr.type = IInstrType::MovImm;
                            instr.imm = imm;

                            checkImmRange(imm, canBeImm16Shift4xNeg);
                        } else {
                            if (!rd.isImm())
                                throwErr<Throw>("mov rd must be imm when not MovImm");
                            if (!rn.isImm())
                                throwErr<Throw>("mov rn must be imm when not MovImm");

                            if ((rdImmVal & W) != (rnImmVal & W)) {
                                throwErr<Throw>("Mismatching register widths");
                                return { };
                            }

                            if (rdImmVal == SP || rnImmVal == SP) {
                                instr.type = IInstrType::MovToFromSp;
                            } else {
                                if (!rn.isImm())
                                    throwErr<Throw>("mov rn must be imm");

                                instr.type = IInstrType::OrrReg;
                                instr.rm = rn;
                                instr.rn = XZR;
                            }
                            instr.imm = 0;
                        }
                        break;
                    }
                    case ParseMnemonic::Movk: {
                        instr.type = IInstrType::Movk;

                        instr.imm = imm;
                        checkImmRange(imm, canBeImm16Shift4xNeg);

                        if (shift != IShiftType::LSL) {
                            throwErr<Throw>("Shift type not allowed", shift);
                            return { };
                        }

                        if (shiftImm.isImm() && rd.isImm()) {
                            u32 hw = shiftImm.resolve(nullptr);
                            if (!(hw == 0 or hw == 16 or ((rdImmVal & W) == 0) and (hw == 32 or hw == 48))) {
                                throwErr<Throw>("Shift out of range", hw);
                                return { };
                            }
                        }

                        instr.shiftImm = shiftImm;
                        break;
                    }
                    case ParseMnemonic::Sub:
                    case ParseMnemonic::Add: {
                        if (imm.isValid()) {
                            instr.type = type == ParseMnemonic::Add ? IInstrType::AddImm : IInstrType::SubImm;
                            instr.imm = imm;

                            checkImmRange(imm, canBeImm12Shift2x);
                        } else {
                            instr.type = type == ParseMnemonic::Add ? IInstrType::AddReg : IInstrType::SubReg;

                            if (shift == IShiftType::ROR) {
                                throwErr<Throw>("Shift type ROR not allowed");
                                return { };
                            }
                            instr.shift = shift;

                            checkImmRange(shiftImm, canBeImm6);
                            instr.shiftImm = shiftImm;
                        }
                        break;
                    }
                    case ParseMnemonic::And: {
                        instr.type = IInstrType::AndReg;
                        break;
                    }
                    case ParseMnemonic::Orr: {
                        instr.rd = rd;

                        if (rd.isImm() && rn.isImm() && rm.isImm())
                            if (rdImmVal == SP || rnImmVal == SP || rmImmVal == SP) {
                                throwErr<Throw>("Register SP not allowed");
                                return { };
                            }

                        instr.type = IInstrType::OrrReg;

                        if (shiftImm.isValid()) {
                            instr.shift = shift;
                            checkImmRange(shiftImm, canBeImm6);
                            instr.shiftImm = shiftImm;
                        }

                        break;
                    }
                    case ParseMnemonic::Adr:
                    case ParseMnemonic::Adrp: {
                        if (rd.isImm())
                            if (rdImmVal & W) {
                                throwErr<Throw>("X register required");
                                return { };
                            }

                        instr.type = type == ParseMnemonic::Adr ? IInstrType::Adr : IInstrType::Adrp;
                        instr.imm = imm;
                        break;
                    }
                    case ParseMnemonic::BcondArg: {
                        instr.type = IInstrType::Bcond;
                        instr.imm = imm;
                        instr.shiftImm = shiftImm;
                        break;
                    }
                    case ParseMnemonic::Cbz:
                    case ParseMnemonic::Cbnz: {
                        instr.type = type == ParseMnemonic::Cbz ? IInstrType::Cbz : IInstrType::Cbnz;
                        instr.imm = imm;
                        break;
                    }
                    case ParseMnemonic::Tbz:
                    case ParseMnemonic::Tbnz: {
                        instr.type = type == ParseMnemonic::Tbz ? IInstrType::Tbz : IInstrType::Tbnz;
                        instr.imm = imm;
                        instr.shiftImm = shiftImm;
                        break;
                    }
                    case ParseMnemonic::Ldr: {
                        instr.type = IInstrType::LdrLiteral;
                        instr.imm = imm;
                        break;
                    }
                    default: {
                        throwErr<Throw>("Shouldn't happen");
                        return { };
                    }
                    }

                    return instr;
                };

                auto parseRegisterOperand = [&](const char* token, size lenToken) -> IRegType {
                    if (lenToken < 2 || lenToken > 3) {
                        return IRegType_Max;
                    }
                    constexpr static std::pair<IRegType, const char*> cSpecialRegs[] {
                        { FP, "fp" },
                        { LR, "lr" },
                        { SP, "sp" },
                        { XZR, "xzr" },
                        { WZR, "wzr" },
                        { IP0, "ip" },
                        { IP0, "ip0" },
                        { IP1, "ip1" },
                    };

                    for (auto pair : cSpecialRegs) {
                        if (isEqualStringIgnoreCase(token, pair.second))
                            return pair.first;
                    }

                    int result = 0;

                    {
                        const char type[2] { token[0], '\0' };
                        if (isEqualStringIgnoreCase(type, "w"))
                            result |= W;
                        else if (isEqualStringIgnoreCase(type, "x"))
                            ;
                        else {
                            return IRegType_Max;
                        }
                    }

                    token++;
                    lenToken--;

                    int regIdx = 0;
                    for (size_t i = 0; i < lenToken; i++) {
                        if (token[i] < '0' || token[i] > '9') {
                            return IRegType_Max;
                        }
                        regIdx = regIdx * 10 + (token[i] - '0');
                    }

                    if (regIdx < 0 || regIdx > 30) {
                        return IRegType_Max;
                    }

                    result |= regIdx;

                    return IRegType(result);
                };

                auto parseImmediateOperand = [&](const char* token, size_t lenToken, u64* out) -> bool {
                    if (lenToken < 1) {
                        return false;
                    }

                    if (lenToken > 2 && token[0] == '0' && (token[1] == 'b' || token[1] == 'B')) {
                        u64 result = 0;
                        for (size i = 2; i < lenToken; i++) {
                            if (token[i] != '0' && token[i] != '1' || i > 64 + 1)
                                return false;

                            result = (result << 1) | (token[i] - '0');
                        }
                        *out = result;
                        return true;
                    }

                    if (lenToken > 2 && token[0] == '0' && (token[1] == 'x' || token[1] == 'X')) {
                        u64 result = 0;
                        for (size i = 2; i < lenToken; i++) {
                            int digitValue;
                            if (token[i] >= '0' && token[i] <= '9')
                                digitValue = token[i] - '0';
                            else if (token[i] >= 'a' && token[i] <= 'f')
                                digitValue = token[i] - 'a' + 10;
                            else if (token[i] >= 'A' && token[i] <= 'F')
                                digitValue = token[i] - 'A' + 10;
                            else
                                return false;

                            if (i > 16 + 1)
                                return false;

                            result = (result << 4) | digitValue;
                        }
                        *out = result;
                        return true;
                    }

                    u64 result = 0;
                    bool isNegative = false;

                    size startIdx = 0;
                    if (token[0] == '-') {
                        isNegative = true;
                        startIdx = 1;
                        if (lenToken == 1) {
                            *out = -1;
                            return false;
                        }
                    }

                    for (size i = startIdx; i < lenToken; i++) {
                        if (token[i] < '0' || token[i] > '9')
                            return false;

                        result = result * 10 + (token[i] - '0');
                    }
                    *out = isNegative ? -result : result;
                    return true;
                };

                auto parseShiftTypeOperand = [&](const char* token, size lenToken) -> IShiftType {
                    if (isEqualStringIgnoreCase(token, "lsl"))
                        return IShiftType::LSL;
                    else if (isEqualStringIgnoreCase(token, "lsr"))
                        return IShiftType::LSR;
                    else if (isEqualStringIgnoreCase(token, "asr"))
                        return IShiftType::ASR;
                    else if (isEqualStringIgnoreCase(token, "ror"))
                        return IShiftType::ROR;

                    return IShiftType::Max;
                };

                int operandIdx = -1;
                int numOptionalsToFind = 0;
                int numOptionalsToResolve = 0;

                executeCallbackBySplit<cOperandDelims>(line, lenLine, [&](const char* token, size lenToken) {
                    if (operandIdx == -1) {
                        m = &findMnemonic(token);

                        if (m->a == ParseMnemonic::None)
                            throwErr<Throw>("Invalid or unimplemented mnemonic");

                        numOptionalsToFind = numOptionalsToResolve = countOptionalOperands(*m);

                        if (m->a == ParseMnemonic::BcondArg)
                            shiftImm = IImm(IImm::CreateArg, argIndex++);

                        operandIdx++;
                        return;
                    }

                    int op = m->c[operandIdx];

                    if (op == End) {
                        throwErr<Throw>("Extra token");
                        return;
                    }

                    if (op & Optional) {
                        numOptionalsToFind--;
                    }

                    const bool needsResolve = util::countSetBits(op) == 1 or not(op & Optional);
                    bool resolved = false;

                    defer {
                        if (needsResolve && !resolved)
                            throwErr<Throw>("Invalid operand");

                        if (op & Optional && resolved) {
                            numOptionalsToResolve--;
                        }
                    };

                    if (op & Rd or op & Rn or op & Rm) {
                        IImm& dst = op & Rd
                            ? rd
                            : op & Rn
                            ? rn
                            : rm;

                        if (isEqualStringIgnoreCase(token, "()")) {
                            dst = IImm(IImm::CreateRegArg, argIndex++);
                            operandIdx++;
                            resolved = true;
                            return;
                        } else {
                            IRegType res = parseRegisterOperand(token, lenToken);
                            if (res != IRegType_Max) {
                                dst = res;
                                operandIdx++;
                                resolved = true;
                                return;
                            }
                        }
                    }

                    if ((op & Imm || op & Shift) && !resolved) {

                        IImm& dest = (op & Imm) ? imm : shiftImm;

                        if (isEqualStringIgnoreCase(token, "{}")) {
                            dest = IImm(IImm::CreateArg, argIndex++);
                            operandIdx++;
                            resolved = true;
                            return;
                        } else {
                            u64 immV;
                            if (parseImmediateOperand(token, lenToken, &immV)) {
                                dest = immV;
                                operandIdx++;
                                resolved = true;
                                return;
                            }
                        }
                    }

                    if (op & ShiftType && !resolved) {
                        IShiftType shiftV = parseShiftTypeOperand(token, lenToken);
                        if (shiftV != IShiftType::Max) {
                            shift = shiftV;
                            operandIdx++;
                            resolved = true;
                            return;
                        }
                    }
                });

                if (operandIdx != -1) {
                    int op = m->c[operandIdx];

                    if (numOptionalsToFind != 0 && numOptionalsToFind != countOptionalOperands(*m) || numOptionalsToFind != numOptionalsToResolve)
                        throwErr<Throw>("Incomplete optional expression");

                    if (op == End or op & Optional)
                        instrs[(*outSize)++] = finalize();
                    else
                        throwErr<Throw>("Incomplete expression");
                }
            });
        }

    public:
        constexpr Expr(const char (&str)[N])
            : util::TemplateString<N>(str) {
        }

        constexpr int calcNumInstrs() const {
            int numInstrs = 0;
            Array<IInstr, 1024> instrs;
            processInstrs<false>(instrs, &numInstrs);
            return numInstrs;
        }
    };

    template <bool Uninstallable, size NumInstrs, size MaxArgs = 16>
    class AsmBlock {
        Array<IInstr, NumInstrs> mInstrs { };
        Array<u64, MaxArgs> mArgs { };
        size mNumArgs = 0;

        ptr mOffset = 0;
        Instr mOrigInstrs[NumInstrs] { };
        const ro::RoModule* mModule = nullptr;

    public:
        constexpr AsmBlock(const Array<IInstr, NumInstrs>& instrs)
            : mInstrs(instrs) {
        }

        constexpr AsmBlock(const AsmBlock& other) = default;

        hk_alwaysinline constexpr void assemble(ptr startAddr, Instr* out) const {
            const ArgGetter args(mArgs.data());
            for (const auto& instr : mInstrs) {
                *(out++) = instr.assemble(startAddr, &args);
                startAddr += sizeof(Instr);
            }
        }

        hk_alwaysinline constexpr Array<Instr, NumInstrs> assemble(ptr startAddr) const {
            Array<Instr, NumInstrs> outInstrs;
            const ArgGetter args(mArgs.data());
            for (const auto [i, instr] : util::iterateWithIdx(mInstrs))
                outInstrs[i] = instr.assemble(startAddr + i * sizeof(Instr), &args);
            return outInstrs;
        }

        hk_alwaysinline constexpr Instr assembleOne(size idx, ptr addr) const {
            const ArgGetter args(mArgs.data());
            {
                size i = 0;
                for (const auto& instr : mInstrs) {
                    Instr result = instr.assemble(addr, &args);
                    if (idx == i++)
                        return result;
                }
            }
            return IInstr::cOpInvalid;
        }

        template <typename... Args>
        constexpr AsmBlock arg(Args... pArgs) const {
            static_assert(sizeof...(pArgs) <= MaxArgs);
            auto args = { static_cast<u64>(pArgs)... };
            const size numArgs = args.size();

            AsmBlock result(*this);
            result.mNumArgs = numArgs;
            {
                size i = 0;
                for (u64 arg : args) {
                    result.mArgs[i++] = arg;
                }
            }

            return result;
        }

        constexpr u64 getArg(size idx) const { return mArgs[idx]; }

        Result tryInstallAtOffset(const ro::RoModule* module, ptr offset) {
            if constexpr (Uninstallable)
                HK_UNLESS(!isInstalled(), ResultAlreadyInstalled());

            Instr instrs[NumInstrs];
            assemble(module->range().start() + offset, instrs);

            if constexpr (Uninstallable)
                __builtin_memcpy(mOrigInstrs, (void*)(module->range().start() + offset), NumInstrs * sizeof(Instr));
            HK_TRY(module->writeRo(offset, instrs, sizeof(instrs)));

            mOffset = offset;
            mModule = module;

            return ResultSuccess();
        }

        Result tryInstallAtMainOffset(ptr offset) {
            return tryInstallAtOffset(ro::getMainModule(), offset);
        }

        Result uninstall() {
            if constexpr (not Uninstallable)
                return ResultNotUninstallable();
            HK_UNLESS(isInstalled(), ResultNotInstalled());

            HK_TRY(mModule->writeRo(mOffset, mOrigInstrs, NumInstrs * sizeof(Instr)));

            mOffset = 0;
            mModule = nullptr;

            return ResultSuccess();
        }

        template <typename T>
        Result tryInstallAtPtr(T* addr) {
            auto* module = ro::getModuleContaining(ptr(addr));
            HK_UNLESS(module != nullptr, ResultOutOfBounds());

            return tryInstallAtOffset(module, ptr(addr) - module->range().start());
        }

        template <util::TemplateString Symbol>
        hk_alwaysinline Result tryInstallAtSym() {
            ptr addr = util::lookupSymbol<Symbol>();

            return tryInstallAtPtr(cast<void*>(addr));
        }

        AsmBlock& installAtOffset(const ro::RoModule* module, ptr offset) {
            tryInstallAtOffset(module, offset);
            return *this;
        }

        AsmBlock& installAtMainOffset(ptr offset) {
            tryInstallAtMainOffset(offset);
            return *this;
        }

        template <typename T>
        AsmBlock& installAtPtr(T* addr) {
            tryInstallAtPtr(addr);
            return *this;
        }

        template <typename T, typename R, typename... Args>
        AsmBlock& installAtPtr(R (T::*addr)(Args...)) {
            return installAtPtr(pun<void*>(addr));
        }

        template <util::TemplateString Symbol>
        AsmBlock& installAtSym() {
            tryInstallAtSym<Symbol>();
            return *this;
        }

        bool isInstalled() const { return mOffset != 0; }
    };

    /**
     * @brief Assembles AArch64 instruction block
     *
     * @tparam E
     * @tparam Uninstallable Whether or not installed blocks should be uninstallable
     * @tparam N
     * @return Object that can assemble to Instr or be installed in a module
     */
    template <Expr E, bool Uninstallable = false, int N = E.calcNumInstrs()>
    consteval AsmBlock<Uninstallable, N> assemble() {
        Expr e(E.value);

        Array<IInstr, N> instrs;
        int numInstrs;
        E.template processInstrs<true>(instrs, &numInstrs);

        Array<IInstr, N> result;
        for (size i = 0; i < N; i++)
            result[i] = instrs[i];
        return AsmBlock<Uninstallable, N>(result);
    }

    /////////////////////////////////////////////////////////////////////////////////////////

    template <bool CanUseInstrAsData = false>
    class PseudoInstructionEmitter {
        VecSpan<Instr>& mOut;
        const ptr mRx = 0;

        size calcCurOffset() const { return sizeof(Instr) * mOut.size(); }
        size calcCurRxAddr() const { return mRx + calcCurOffset(); }

    public:
        constexpr PseudoInstructionEmitter(VecSpan<Instr>& out, ptr rx)
            : mOut(out)
            , mRx(rx) { }

        size emit(Instr instr) {
            mOut.add(instr);
            return 1;
        }

        size emit(Span<const Instr> instrs) {
            mOut.append(instrs);
            return instrs.size();
        }

        size /* max when CanUseInstrAsData: 2 or 1 + 2 at the end, when !CanUseInstrAsData: 4 */ emitMovImmediate64(IRegType reg, u64 value) {
            const ptr pc = calcCurRxAddr();
            const ptr pcUpper = pc & ~bits(12);
            const u64 upper = value & ~bits(12);
            const u64 lower = value & bits(12);

            constexpr size adrAdrpMaxDiff = bit(21) / 2 - 1;
            constexpr auto adrExpr = assemble<"adr (), {}">();

            if (abs(pc - value) <= adrAdrpMaxDiff) {
                emit(adrExpr.arg(reg, value).assemble(pc));
                return 1;
            } else if ((abs(pcUpper - upper) / cPageSize) <= adrAdrpMaxDiff) {
                constexpr auto adrpExpr = assemble<"adrp (), {}">();
                constexpr auto addExpr = assemble<"add (), (), {}">();

                emit(adrpExpr.arg(reg, upper).assemble(pc));
                if (upper != value) {
                    emit(addExpr.arg(reg, reg, lower).assemble(pc + sizeof(Instr)));
                    return 2;
                }
                return 1;
            } else {
                if constexpr (CanUseInstrAsData) {
                    constexpr auto ldrExpr = assemble<"ldr (), {}">();

                    HK_ASSERT(mOut.capacity() - mOut.size() >= 3);
                    const size addrOffs = mOut.capacity() - 2 - 1;

                    emit(ldrExpr.arg(reg, mRx + addrOffs * sizeof(Instr)).assemble(pc));

                    mOut.data()[addrOffs] = Instr(value & bits(32));
                    mOut.data()[addrOffs + 1] = Instr((value >> 32) & bits(32));
                    mOut.set(mOut.data(), mOut.size(), mOut.capacity() - 2);
                    return 1;
                }

                // !CanUseInstrAsData
                const size startSize = mOut.size();

                const u16 val16[4] {
                    u16(value & bits(16)),
                    u16((value >> 16) & bits(16)),
                    u16((value >> 32) & bits(16)),
                    u16((value >> 48) & bits(16))
                };

                const u16 pc16[4] {
                    u16(pc & bits(16)),
                    u16((pc >> 16) & bits(16)),
                    u16((pc >> 32) & bits(16)),
                    u16((pc >> 48) & bits(16))
                };

                const bool match[4] { val16[0] == pc16[0], val16[1] == pc16[1], val16[2] == pc16[2], val16[3] == pc16[3] };
                const bool anyMatch = Span(match).includes(true);

                if (anyMatch) {
                    emit(adrExpr.arg(reg, pc).assemble(pc));
                }

                for (const auto [i, match] : util::iterateWithIdx(Span(match)))
                    if (!match) {
                        constexpr auto movkExpr = assemble<"movk (), {}, lsl #{}">();
                        emit(movkExpr.arg(reg, val16[i], i * 16).assemble((mOut.size() - startSize) * sizeof(Instr)));
                    }

                return mOut.size() - startSize;
            }
        }

        size /* max when CanUseInstrAsData: 3 or 2 + 2 at the end, when !CanUseInstrAsData: 5 */ emitBranch(ptr to, bool link) {
            const ptr from = calcCurRxAddr();
            const s64 gap = to - from;
            if (abs(gap) <= cMaxBranchDistance) {
                mOut.add(link ? makeBL(from, to) : makeB(from, to));
                return 1;
            }

            constexpr Instr cBr = assemble<"br ip">().assembleOne(0, 0);
            constexpr Instr cBlr = assemble<"blr ip">().assembleOne(0, 0);

            size movSize = emitMovImmediate64(IP0, to);
            mOut.add(link ? cBlr : cBr);
            return movSize + 1;
        }

        size emitB(ptr to) { return emitBranch(to, false); }
        size emitBL(ptr to) { return emitBranch(to, true); }
    };

} // namespace hk::hook::a64
