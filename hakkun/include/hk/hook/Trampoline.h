#pragma once

#include "hk/container/VecSpan.h"
#include "hk/hook/InstrUtil.h"
#include "hk/hook/Replace.h"
#include "hk/hook/a64/Assembler.h"
#include "hk/svc/api.h"
#include "hk/util/PoolAllocator.h"

namespace hk::hook {

    namespace detail {

        extern "C" struct _ __static_trampolines_begin__;
        extern "C" struct _ __static_trampolines_end__;

        static const ptr cTrampolinesBeginRx = ptr(&__static_trampolines_begin__);
        static const ptr cTrampolinesEndRx = ptr(&__static_trampolines_end__);
        static const size cTrampolinePoolSize = cTrampolinesEndRx - cTrampolinesBeginRx;
        const extern size cTrampolineRwMap;

        size makeTrampolineBackup(Span<const Instr> orig, ptr origAddr, ptr origReturnAddr, VecSpan<Instr> out, ptr trampolineRx);

        template <size MaxNumInstrs>
        struct TrampolineBackupBase {
            Instr instrs[MaxNumInstrs] { 0 };

            void make(Span<const Instr> orig, ptr origAddr, ptr origReturnAddr) { makeTrampolineBackup(orig, origAddr, origReturnAddr, this->instrs, getRx()); }
            void clear() {
                util::fill(this->instrs, MaxNumInstrs, Instr(0));
                svc::clearCache(getRx(), sizeof(*this));
            }

            ptr getRx() const { return cTrampolinesBeginRx + (ptr(this) - cTrampolineRwMap); }
            TrampolineBackupBase* getRw() const { return cast<TrampolineBackupBase*>(cTrampolineRwMap + (ptr(this) - cTrampolinesBeginRx)); }
        } __attribute__((packed));

        /**
         * HK_HOOK_TRAMPOLINE_LEVEL
         * 0: only basic backups (no b.cond, cbz/cbnz, tbz/tbnz), +-128MB branch range
         * 1: all backups, +-128MB branch range
         * 2: all backups, infinite branch range
         */

        using TrampolineBackup = TrampolineBackupBase<
#if HK_HOOK_TRAMPOLINE_LEVEL == 0 or !__aarch64__
            2
#elif HK_HOOK_TRAMPOLINE_LEVEL == 1
            5
#elif HK_HOOK_TRAMPOLINE_LEVEL == 2
            5 * 5
#else
#error
#endif
            >;

    } // namespace detail

    namespace detail {

        template <typename...>
        struct TrampolineStaticBackup {
            static constinit inline section(.text.trampolines_static) hk::hook::detail::TrampolineBackup orig;
        };

        enum _TrampolineTypeFlag { };

        template <size N>
        using TrampolineTypeFlag = _TrampolineTypeFlag;

        using TrampolineStaticFlag = TrampolineTypeFlag<1>;
        using TrampolineDynamicFlag = TrampolineTypeFlag<2>;

        template <size Flag, typename...>
        struct IsTrampolineLambdaWithFlag {
            constexpr static bool cValue = false;
        };

        template <size Flag, LambdaNoCaptureDeduceThisType L>
        struct IsTrampolineLambdaWithFlag<Flag, L> : IsTrampolineLambdaWithFlag<Flag, L, decltype(&L::template operator()<L>)> { };

        template <size Flag, LambdaNoCaptureDeduceThisType L, typename Return, typename... Args>
        struct IsTrampolineLambdaWithFlag<Flag, L, Return (*)(Args...)> {
            constexpr static bool cValue = util::ctIsSame<TrampolineTypeFlag<Flag>, util::tIndex<2, Args...>>;
        };

        template </* hk::LambdaNoCaptureDeduceThisType */ typename L, typename...>
        class TrampolineBackupInvoker : public TrampolineBackupInvoker<L, decltype(&L::template operator()<L>)> {
        private:
            using Super = TrampolineBackupInvoker<L, decltype(&L::template operator()<L>)>;

        public:
            using Super::Super;
        };

        template </*hk::LambdaNoCaptureDeduceThisType*/ typename L, typename Return, typename Self, typename Orig, typename Flag, typename... Args>
            requires(util::ctIsSame<Flag, TrampolineStaticFlag>)
        class TrampolineBackupInvoker<L, Return (*)(Self, Orig, Flag, Args...)> : hk::util::FunctionTraits<Return (*)(Args...)> {
            using Super = hk::util::FunctionTraits<Return (*)(Args...)>;
            using FuncPtr = typename Super::FuncPtrTypeStatic;

        public:
            TrampolineBackupInvoker(const L&) { }

            hk_alwaysinline static Return invoke(Args... args) {
                FuncPtr func = pun<FuncPtr>(&TrampolineStaticBackup<L>::orig);
                return func(forward<Args>(args)...);
            }

            hk_alwaysinline Return operator()(Args... args) const { return invoke(forward<Args>(args)...); }
        };

        template <hk::LambdaNoCaptureDeduceThisType L, typename...>
        struct TrampolineFunc : TrampolineFunc<L, decltype(&L::template operator()<L>)> { };

        template <hk::LambdaNoCaptureDeduceThisType L, typename Return, typename Self, typename Orig, typename Flag, typename... Args>
            requires(util::ctIsSame<Flag, TrampolineStaticFlag>)
        struct TrampolineFunc<L, Return (*)(Self, Orig, Flag, Args...)> {
            static Return invoke(Args... args) {
                const ptr pc = ptr(invoke);

                return L()({ L() }, TrampolineStaticFlag(), forward<Args>(args)...);
            }
        };

    } // namespace detail

    template <typename L>
    concept TrampolineStaticLambdaType = detail::IsTrampolineLambdaWithFlag<1, L>::cValue;

    class TrampolineStaticBase : public HookNInstr<detail::cHookMaxOverwriteSize> {
    protected:
        constexpr static size N = detail::cHookMaxOverwriteSize;

        Result installAtOffset(const ro::RoModule* module, ptr offset, ptr to, detail::TrampolineBackup* backup);
    };

    template <typename...>
    class Trampoline;

    template <TrampolineStaticLambdaType L>
    class Trampoline<L> : public TrampolineStaticBase, public HookOperations<Trampoline<L>> {
        using TrampolineStaticBase::installAtOffset;

    public:
        const static inline auto orig = detail::TrampolineBackupInvoker<L>::invoke;

        Trampoline(const L& func) { }

        hk_alwaysinline Result installAtOffset(const ro::RoModule* module, ptr offset) {
            return installAtOffset(module, offset, ptr(detail::TrampolineFunc<L>::invoke), detail::TrampolineStaticBackup<L>::orig.getRw());
        }
    };

    template <typename Return, typename... Args>
    class Trampoline<Return, Args...> : public Trampoline<Return (*)(Args...)> {
        using Super = Trampoline<Return (*)(Args...)>;

    public:
        using Super::Super;
    };

    template <typename Return, typename... Args>
    class Trampoline<Return(Args...)> : public Trampoline<Return (*)(Args...)> {
        using Super = Trampoline<Return (*)(Args...)>;

    public:
        using Super::Super;
    };

    template <typename Return, typename... Args>
    class Trampoline<Return(Args..., ...)> : public Trampoline<Return (*)(Args..., ...)> {
        using Super = Trampoline<Return (*)(Args..., ...)>;

    public:
        using Super::Super;
    };

    template <FunctionPointerType FuncPtr>
    class Trampoline<FuncPtr> : public TrampolineStaticBase, public HookOperations<Trampoline<FuncPtr>> {
        using TrampolineStaticBase::installAtOffset;

        const FuncPtr mFunc;

        detail::TrampolineBackup* getBackup() { return cast<detail::TrampolineBackup*>(orig)->getRw(); }

    public:
        const FuncPtr orig;

        template <LambdaNoCaptureType L>
        [[deprecated("see README.md for new static trampoline syntax")]] hk_alwaysinline Trampoline(L func)
            : mFunc(util::FunctionTraits<L>::fromLambda(forward<L>(func)))
            , orig(pun<FuncPtr>(&detail::TrampolineStaticBackup<L>::orig)) { }

        template <typename... Unique>
        hk_alwaysinline Trampoline(FuncPtr func, Unique...)
            : mFunc(func)
            , orig(pun<FuncPtr>(&detail::TrampolineStaticBackup<Unique...>::orig)) { }

        hk_alwaysinline Result installAtOffset(const ro::RoModule* module, ptr offset) {
            return installAtOffset(module, offset, ptr(mFunc), getBackup());
        }
    };

    template <TrampolineStaticLambdaType L>
    Trampoline(const L&) -> Trampoline<L>;

    namespace detail {

        template <typename>
        struct LegacyTrampolineFromLambda;

        template <LambdaNoCaptureType L>
        struct LegacyTrampolineFromLambda<L> : LegacyTrampolineFromLambda<decltype(&L::operator())> { };

        template <typename Class, typename Return, typename... Args>
        struct LegacyTrampolineFromLambda<Return (Class::*)(Args...) const> {
            using Trampoline = Trampoline<Return, Args...>;
        };

    } // namespace detail

    template <LambdaNoCaptureType L>
    [[deprecated("see README.md for new static trampoline syntax")]] typename detail::LegacyTrampolineFromLambda<L>::Trampoline trampoline(L func) {
        using Traits = util::FunctionTraits<L>;
        return { forward<L>(func) };
    }

} // namespace hk::hook

#define TrampolineStatic(...) this auto &&_hk_trampoline_func_self, ::hk::hook::detail::TrampolineBackupInvoker<::hk::util::tRemoveQualifiers<decltype(_hk_trampoline_func_self)>> orig, ::hk::hook::detail::TrampolineStaticFlag
#define AllocateTrampolineStatic(FUNC) ::hk::hook::Trampoline<decltype(FUNC)>(FUNC, ::hk::util::TemplateString(__FILE__), __LINE__, ::hk::util::TemplateString(#FUNC))

template <typename... Args>
using HkTrampoline = hk::hook::Trampoline<Args...>;
