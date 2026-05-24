#pragma once

#include "hk/hook/InstrUtil.h"
#include "hk/hook/results.h"
#include "hk/prim/traits/Function.h"
#include "hk/ro/RoUtil.h"
#include "hk/util/Context.h"

namespace hk::hook {

    namespace detail {

        constexpr size cHookMaxOverwriteSize =
#if HK_HOOK_TRAMPOLINE_LEVEL == 0 or !__aarch64__
            1
#elif HK_HOOK_TRAMPOLINE_LEVEL == 1
            2
#elif HK_HOOK_TRAMPOLINE_LEVEL == 2
            5
#else
#error
#endif
            ;

    } // namespace detail

    template <typename Parent>
    struct HookOperations {
        Result installAtPtr(ptr addr) {
            HK_UNLESS(addr != 0, hk::ResultInvalidAddress());
            auto* module = ro::getModuleContaining(addr);
            HK_UNLESS(module != nullptr, ResultOutOfBounds());

            return static_cast<Parent*>(this)->installAtOffset(module, ptr(addr) - module->range().start());
        }

        [[deprecated("use installAtPtr(ptr), installAtPtr(R(*)(Args...)), installAtPtr(R (T::*)(Args...))")]] Result installAtPtr(void* addr) {
            return installAtPtr(ptr(addr));
        }

        template <AnyFunctionPointerType T>
        Result installAtPtr(T func) {
            using Traits = util::FunctionTraits<T>;
            return installAtPtr(Traits::getAddress(func));
        }

        template <util::TemplateString Symbol>
        hk_alwaysinline Result installAtSym() { return installAtPtr(util::lookupSymbol<Symbol>()); }

        Result installAtMainOffset(ptr offset) {
            return static_cast<Parent*>(this)->installAtOffset(ro::getMainModule(), offset);
        }
    };

    class HookBase {
    protected:
        const ro::RoModule* mModule = nullptr;
        ptr mOffset = 0;

        ptr getAt() const { return mModule->range().start() + mOffset; }
    };

    template <size N>
    class HookNInstr : protected HookBase {
    protected:
        Instr mOrigInstrs[N] { 0 };

        void clear() {
            mModule = 0;
            mOffset = 0;
            util::fill(mOrigInstrs, N, Instr(0));
        }

    public:
        bool isInstalled() const { return mOrigInstrs[0] != 0; }

        Result installAtOffset(const ro::RoModule* module, ptr offset, Span<const Instr> instrs) {

            HK_UNLESS(!isInstalled(), ResultAlreadyInstalled());
            mModule = module;
            mOffset = offset;

            util::copy(mOrigInstrs, cast<Instr*>(getAt()), N);
            Result rc = mModule->writeRo(mOffset, instrs.data(), instrs.size() * sizeof(Instr));
            if (rc.failed()) {
                clear();
                return MAKE_RESULT(rc);
            }

            return ResultSuccess();
        }

        Result uninstall() {
            HK_UNLESS(isInstalled(), ResultNotInstalled());

            HK_TRY(mModule->writeRo(mOffset, mOrigInstrs, N * sizeof(Instr)));
            clear();

            return ResultSuccess();
        }
    };

    class HookOneInstr : public HookNInstr<1> {
    public:
        Result installAtOffset(const ro::RoModule* module, ptr offset, Instr instr) { return HookNInstr::installAtOffset(module, offset, { &instr, 1 }); }
    };

} // namespace hk::hook
