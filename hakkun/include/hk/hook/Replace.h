#pragma once

#include "hk/Result.h"
#include "hk/hook/InstrUtil.h"
#include "hk/hook/results.h"
#include "hk/ro/RoUtil.h"
#include "hk/util/Lambda.h"
#include <type_traits>

namespace hk::hook {

    template <typename Func>
    class ReplaceHook {
    protected:
        const Func mFunc = nullptr;
        const ro::RoModule* mModule = nullptr;
        ptr mOffset = 0;
        u32 mOrigInstr = 0;

        ptr getAt() const { return mModule->range().start() + mOffset; }

    public:
        ReplaceHook(Func func)
            : mFunc(func) { }

        template <typename L>
        ReplaceHook(L func)
            : mFunc((typename util::FunctionTraits<L>::FuncPtrType)func) { }

        bool isInstalled() const { return mOrigInstr != 0; }

        virtual Result installAtOffset(const ro::RoModule* module, ptr offset) {
            HK_UNLESS(!isInstalled(), ResultAlreadyInstalled());

            mModule = module;
            mOffset = offset;

            mOrigInstr = *cast<u32*>(getAt());
            Result rc = writeBranch(mModule, mOffset, mFunc);
            if (rc.failed()) {
                mModule = nullptr;
                mOffset = 0;
                mOrigInstr = 0;
                return rc;
            }

            return ResultSuccess();
        }

        template <typename T>
        Result installAtPtr(T* addr) {
            auto* module = ro::getModuleContaining(ptr(addr));
            HK_UNLESS(module != nullptr, ResultOutOfBounds());

            return installAtOffset(module, ptr(addr) - module->range().start());
        }

        template <util::TemplateString Symbol>
        hk_alwaysinline Result installAtSym() {
            ptr addr;
            if constexpr (sail::sUsePrecalcHashes) {
                constexpr u32 symMurmur = util::hashMurmur(Symbol.value);
                addr = sail::lookupSymbolFromDb<true>(&symMurmur);
            } else {
                addr = sail::lookupSymbolFromDb<false>(Symbol.value);
            }

            return installAtPtr(cast<void*>(addr));
        }

        virtual Result uninstall() {
            HK_UNLESS(isInstalled(), ResultNotInstalled());

            HK_TRY(mModule->writeRo(mOffset, mOrigInstr));
            mModule = nullptr;
            mOffset = 0;
            mOrigInstr = 0;

            return ResultSuccess();
        }
    };

    /*template <typename Return, typename... Args>
    ReplaceHook<Return (*)(Args...)> replace(Return (*func)(Args...)) {
        return { func };
    }*/

    template <typename L>
    typename std::enable_if<!util::LambdaHasCapture<L>::value, ReplaceHook<typename util::FunctionTraits<L>::FuncPtrType>>::type replace(L func) {
        using Func = typename util::FunctionTraits<L>::FuncPtrType;
        return { (Func)func };
    }

} // namespace hk::hook

template <typename Ret, typename... Args>
using HkReplace = hk::hook::ReplaceHook<Ret (*)(Args...)>;
