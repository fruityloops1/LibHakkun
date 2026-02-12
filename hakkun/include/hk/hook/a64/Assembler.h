#pragma once

#include "hk/diag/diag.h"
#include "hk/hook/InstrUtil.h"
#include "hk/hook/results.h"
#include "hk/ro/RoModule.h"
#include "hk/types.h"
#include "hk/util/Context.h"
#include "hk/util/Math.h"
#include "hk/util/TemplateString.h"
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
        enum Type : u8 {
            None,
            Imm,
            Arg,
        };

        u64 valueImm = 0;
        Type type = None;

        constexpr u64 resolve(const ArgGetter* args) const {
            switch (type) {
            case Imm:
                return valueImm;
            case Arg:
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

        constexpr IImm(_CreateArg, size argIdx)
            : type(Arg)
            , valueImm(argIdx) {
        }

        constexpr bool isValid() const { return type != None; }
        constexpr bool isImm() const { return type == Imm; }
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
        if (util::isRepresentable<Width, NumShift>(value))
            return true;

        if (Neg && util::isRepresentable<Width, NumShift>(~value))
            return true;
        return false;
    }

    constexpr auto canBeImm16Shift4xNeg = canBeImm<true, 16, 4>;
    constexpr auto canBeImm12Shift2x = canBeImm<false, 12, 2>;
    constexpr auto canBeImm6 = canBeImm<false, 6, 1>;

    struct IInstr {
        IInstrType type;
        IRegType rd = IRegType_Max;
        IRegType rn = IRegType_Max;
        IRegType rm = IRegType_Max;
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

        constexpr static u32 cOpInvalid = 0xFFFFFFFF;

        constexpr IInstr()
            : type(IInstrType::AddImm) {
        }

    private:
    public:
        constexpr Instr assemble(ptr addr, const ArgGetter* args) const {
            auto rd = this->rd;
            auto rn = this->rn;
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
                return InstrBuilder()
                    .set(31, 10, cOpRet)
                    .set(9, 5, reg(rn))
                    .set(4, 0, 0);
            }
            case IInstrType::Br: {
                return InstrBuilder()
                    .set(31, 10, cOpBr)
                    .set(9, 5, reg(rn))
                    .set(4, 0, 0);
            }
            case IInstrType::Blr: {
                return InstrBuilder()
                    .set(31, 10, cOpBlr)
                    .set(9, 5, reg(rn))
                    .set(4, 0, 0);
            }
            case IInstrType::Svc: {
                u16 immV = imm.resolve(args);
                return InstrBuilder()
                    .set(31, 0, cOpSvc)
                    .set(20, 5, immV);
            }
            case IInstrType::AddImm:
            case IInstrType::SubImm: {
                u32 immV = imm.resolve(args);

                if (!canBeImm12Shift2x(immV)) {
                    if (__builtin_is_constant_evaluated()) {
                        return cOpInvalid;
                    } else {
                        HK_ABORT("AddImm: Immediate out of range (%llx)", immV);
                    }
                }

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

                if (!canBeImm16Shift4xNeg(immV)) {
                    if (__builtin_is_constant_evaluated()) {
                        return cOpInvalid;
                    } else {
                        HK_ABORT("MovImm: Immediate out of range (%llx)", immV);
                    }
                }

                bool useMovn = false;
                u64 invImmV = ~immV;
                if (util::countSetBits(immV) > util::countSetBits(invImmV)) {
                    useMovn = true;
                    immV = invImmV;
                }

                u32 hw = (immV > bits(48)) ? 48
                    : (immV > bits(32))    ? 32
                    : (immV > bits(16))    ? 16
                                           : 0;

                return InstrBuilder()
                    .set(31, 31, isX(rd))
                    .set(30, 23, useMovn ? cOpMovn : cOpMovz)
                    .set(22, 21, hw / 16)
                    .set(20, 5, (immV >> hw) & bits(16))
                    .set(4, 0, reg(rd));
            }
            case IInstrType::Movk: {
                u64 immV = imm.resolve(args);
                u64 hw = shiftImm.resolve(args);

                if (!canBeImm16Shift4xNeg(immV)) {
                    if (__builtin_is_constant_evaluated()) {
                        return cOpInvalid;
                    } else {
                        HK_ABORT("Movk: Immediate out of range (%llx)", immV);
                    }
                }

                if (!(hw == 0 or hw == 16 or hw == 32 or hw == 48)) {
                    if (__builtin_is_constant_evaluated()) {
                        return cOpInvalid;
                    } else {
                        HK_ABORT("Movk: Shift out of range (%llx)", hw);
                    }
                }

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

        using Mnemonic = std::tuple<ParseMnemonic, const char*, std::array<u8, cMaxOperands + 1>>;

        constexpr static Mnemonic cMnemonics[] {
            { ParseMnemonic::None, "\0", {} },
            { ParseMnemonic::Nop, "nop", {} },
            { ParseMnemonic::Ret, "ret", { Rn | Optional } },
            { ParseMnemonic::Br, "br", { Rn } },
            { ParseMnemonic::Blr, "blr", { Rn } },
            { ParseMnemonic::Svc, "svc", { Imm } },
            { ParseMnemonic::Mov, "mov", { Rd, Rn | Imm } },
            { ParseMnemonic::Movk, "movk", { Rd, Imm, ShiftType | Optional, Shift | Optional } },
            { ParseMnemonic::Orr, "orr", { Rd, Rn, Rm, ShiftType | Optional, Shift | Optional } },
            { ParseMnemonic::Add, "add", { Rd, Rn, Rm | Imm, ShiftType | Optional, Shift | Optional } },
            { ParseMnemonic::Sub, "sub", { Rd, Rn, Rm | Imm, ShiftType | Optional, Shift | Optional } },
            { ParseMnemonic::And, "and", { Rd, Rn, Rm | Imm, ShiftType | Optional, Shift | Optional } },
        };

        constexpr int countOptionalOperands(const Mnemonic& m) const {
            int count = 0;
            for (size i = 0; i < cMaxOperands; i++) {
                if (std::get<2>(m)[i] & Optional)
                    count++;
            }
            return count;
        }

        constexpr static const Mnemonic& findMnemonic(const char* opcodeStr) {
            for (const auto& m : cMnemonics) {
                if (isEqualStringIgnoreCase(std::get<1>(m), opcodeStr))
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
        constexpr void processInstrs(std::array<IInstr, BufferSize>& instrs, int* outSize) const {
            *outSize = 0;
            size argIndex = 0;
            executeCallbackBySplit<"\n">(this->value, N, [&](const char* line, size lenLine) {
                constexpr static char cOperandDelims[] = " ,#\t\v\r";

                const Mnemonic* m = &cMnemonics[0];
                IRegType rd = IRegType_Max, rn = IRegType_Max, rm = IRegType_Max;
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
                    const auto type = std::get<0>(*m);
                    switch (type) {
                    case ParseMnemonic::Nop: {
                        instr.type = IInstrType::Nop;
                        break;
                    }
                    case ParseMnemonic::Ret: {
                        instr.type = IInstrType::Ret;
                        if (rn & W) {
                            throwErr<Throw>("X register required");
                            return {};
                        }
                        instr.rn = rn == IRegType_Max ? LR : rn;
                        break;
                    }
                    case ParseMnemonic::Br:
                    case ParseMnemonic::Blr: {
                        if (rn & W) {
                            throwErr<Throw>("X register required");
                            return {};
                        }
                        instr.type = std::get<0>(*m) == ParseMnemonic::Br ? IInstrType::Br : IInstrType::Blr;
                        instr.rn = rn;
                        break;
                    }
                    case ParseMnemonic::Svc: {
                        if (imm.isImm() && imm.resolve(nullptr) > bits(16)) {
                            throwErr<Throw>("SVC imm16 out of range", imm.resolve(nullptr));
                            return {};
                        }

                        instr.type = IInstrType::Svc;
                        instr.imm = imm;
                        break;
                    }
                    case ParseMnemonic::Mov: {
                        instr.rd = rd;
                        if (imm.isValid()) {
                            instr.type = IInstrType::MovImm;
                            instr.imm = imm;

                            checkImmRange(imm, canBeImm16Shift4xNeg);
                        } else {
                            if ((rd & W) != (rn & W)) {
                                throwErr<Throw>("Mismatching register widths");
                                return {};
                            }

                            if (rd == SP || rn == SP) {
                                instr.type = IInstrType::MovToFromSp;
                                instr.rn = rn;
                            } else {
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
                        instr.rd = rd;

                        instr.imm = imm;
                        checkImmRange(imm, canBeImm16Shift4xNeg);

                        if (shift != IShiftType::LSL) {
                            throwErr<Throw>("Shift type not allowed", shift);
                            return {};
                        }

                        if (shiftImm.isImm()) {
                            u32 hw = shiftImm.resolve(nullptr);
                            if (!(hw == 0 or hw == 16 or ((rd & W) == 0) and (hw == 32 or hw == 48))) {
                                throwErr<Throw>("Shift out of range", hw);
                                return {};
                            }
                        }

                        instr.shiftImm = shiftImm;
                        break;
                    }
                    case ParseMnemonic::Sub:
                    case ParseMnemonic::Add: {
                        instr.rd = rd;
                        instr.rn = rn;

                        if (imm.isValid()) {
                            instr.type = type == ParseMnemonic::Add ? IInstrType::AddImm : IInstrType::SubImm;
                            instr.imm = imm;

                            checkImmRange(imm, canBeImm12Shift2x);
                        } else {
                            instr.type = type == ParseMnemonic::Add ? IInstrType::AddReg : IInstrType::SubReg;
                            instr.rm = rm;

                            if (shift == IShiftType::ROR) {
                                throwErr<Throw>("Shift type ROR not allowed");
                                return {};
                            }
                            instr.shift = shift;

                            checkImmRange(shiftImm, canBeImm6);
                            instr.shiftImm = shiftImm;
                        }
                        break;
                    }
                    case ParseMnemonic::And: {
                        instr.rd = rd;
                        instr.rn = rn;

                        if ((rd & W) != (rn & W)) {
                            throwErr<Throw>("Mismatching register widths");
                            return {};
                        }

                        if (imm.isValid()) {
                            instr.imm = imm;
                        } else {
                            instr.type = IInstrType::AndReg;
                            instr.rm = rm;

                            if ((rd & W) != (rm & W)) {
                                throwErr<Throw>("Mismatching register widths");
                                return {};
                            }
                        }
                        break;
                    }
                    case ParseMnemonic::Orr: {
                        instr.rd = rd;

                        if ((rd & W) != (rn & W) || (rd & W) != (rm & W)) {
                            throwErr<Throw>("Mismatching register widths");
                            return {};
                        }

                        if (rd == SP || rn == SP || rm == SP) {
                            throwErr<Throw>("Register SP not allowed");
                            return {};
                        }

                        instr.type = IInstrType::OrrReg;
                        instr.rn = rn;
                        instr.rm = rm;

                        if (shiftImm.isValid()) {
                            instr.shift = shift;
                            checkImmRange(shiftImm, canBeImm6);
                            instr.shiftImm = shiftImm;
                        }

                        break;
                    }
                    default: {
                        throwErr<Throw>("Shouldn't happen");
                        return {};
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

                        if (std::get<0>(*m) == ParseMnemonic::None)
                            throwErr<Throw>("Invalid or unimplemented mnemonic");

                        numOptionalsToFind = numOptionalsToResolve = countOptionalOperands(*m);

                        operandIdx++;
                        return;
                    }

                    int op = std::get<2>(*m)[operandIdx];

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
                        IRegType& dst = op & Rd
                            ? rd
                            : op & Rn
                            ? rn
                            : rm;

                        IRegType res = parseRegisterOperand(token, lenToken);
                        if (res != IRegType_Max) {
                            dst = res;
                            operandIdx++;
                            resolved = true;
                            return;
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
                    int op = std::get<2>(*m)[operandIdx];

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
            std::array<IInstr, 1024> instrs;
            processInstrs<false>(instrs, &numInstrs);
            return numInstrs;
        }
    };

    template <bool Uninstallable, size NumInstrs, size MaxArgs = 16>
    class AsmBlock {
        std::array<IInstr, NumInstrs> mInstrs {};
        std::array<u64, MaxArgs> mArgs {};
        size mNumArgs = 0;

        ptr mOffset = 0;
        Instr mOrigInstrs[NumInstrs] {};
        const ro::RoModule* mModule = nullptr;

    public:
        constexpr AsmBlock(const std::array<IInstr, NumInstrs>& instrs)
            : mInstrs(instrs) {
        }

        constexpr AsmBlock(const AsmBlock& other) = default;

        constexpr void assemble(ptr startAddr, Instr* out) const {
            const ArgGetter args(mArgs.data());
            for (const auto& instr : mInstrs) {
                *(out++) = instr.assemble(startAddr, &args);
                startAddr += sizeof(Instr);
            }
        }

        constexpr Instr assembleOne(size idx, ptr addr) const {
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
        constexpr AsmBlock arg(Args... pArgs) {
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

        std::array<IInstr, N> instrs;
        int numInstrs;
        E.template processInstrs<true>(instrs, &numInstrs);

        std::array<IInstr, N> result;
        for (size i = 0; i < N; i++)
            result[i] = instrs[i];
        return AsmBlock<Uninstallable, N>(result);
    }

} // namespace hk::hook::asm
