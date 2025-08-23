#pragma once

#include "hk/Result.h"
#include "hk/diag/diag.h"
#include "hk/hook/InstrUtil.h"
#include "hk/hook/MapUtil.h"
#include "hk/hook/results.h"
#include "hk/ro/RoUtil.h"
#include "hk/svc/api.h"
#include "hk/svc/types.h"
#include "hk/types.h"
#include "hk/util/Context.h"
#include "hk/util/Lambda.h"
#include "hk/util/PoolAllocator.h"
#include <cstdlib>
#include <type_traits>

#ifdef __aarch64__
#include "hk/hook/a64/Assembler.h"
#endif

namespace hk::hook {

    namespace detail {

        struct FarBackup {
            struct BranchToAddress {
#ifdef __aarch64__
                Instr ldrAddrInstr;
                Instr brInstr;
                ptr addr;
#else
#error
#endif
            };

            BranchToAddress branchToAddressForReplace;
            Instr origInstrForTrampoline;
            Instr bRetInstrForTrampoline;
        };

        extern util::PoolAllocator<FarBackup, HK_HOOK_TRAMPOLINE_POOL_SIZE> sFarBackupPool;
    } // namespace detail

    template <typename Func>
    class ReplaceHook {
        ptr getOffsetIntoFarMap() const {
            ptr rwMap = getFarRwMap();
            ptr offsetIntoMap = ptr(mFarBackup) - rwMap;

            return offsetIntoMap;
        }

    protected:
        const Func mFunc = nullptr;
        const ro::RoModule* mModule = nullptr;
        ptr mOffset = 0;
        Instr mOrigInstr = 0;
        detail::FarBackup* mFarBackup = nullptr;
        ptr mFarRxMap = 0;

        static s64 checkBranchDistanceExceeded(ptr from, ptr to) {
            const s64 gap = abs(s64(to - from));
            return gap <= cMaxBranchDistance ? 0 : gap;
        }

        ptr getAt() const { return mModule->range().start() + mOffset; }
        ptr getFarRwMap() const { return alignDownPage(ptr(mFarBackup)); }
        ptr getFarRx() const {
            return mFarRxMap + getOffsetIntoFarMap();
        }
        void clearFarBackupCache() {
            ptr branchTarget = mFarRxMap + getOffsetIntoFarMap();
            svc::clearCache(branchTarget, sizeof(detail::FarBackup));
        }

#define _HK_REPLACEHOOK_TRY(RESULT)               \
    {                                             \
        const ::hk::Result _result_temp = RESULT; \
        if (_result_temp.failed()) {              \
            mModule = nullptr;                    \
            mOffset = 0;                          \
            mOrigInstr = 0;                       \
            mFarBackup = nullptr;                 \
            mFarRxMap = 0;                        \
            return _result_temp;                  \
        }                                         \
    }

        Result writeBranch() {
            const ptr func = ptr(mFunc);
            const ptr hookAddr = mModule->range().start() + mOffset;

            mOrigInstr = *cast<Instr*>(getAt());

            if (checkBranchDistanceExceeded(hookAddr, func) or true) {
                mFarBackup = detail::sFarBackupPool.allocate();
                HK_ABORT_UNLESS(mFarBackup != nullptr, "FarBackupPool full! Current size: 0x%x", HK_HOOK_TRAMPOLINE_POOL_SIZE);

#if __aarch64__
                mFarBackup->branchToAddressForReplace.ldrAddrInstr = 0x58000050; // ldr x16, [pc, #8]
                mFarBackup->branchToAddressForReplace.brInstr = a64::assemble<"br x16">().assembleOne(0, 0);
                mFarBackup->branchToAddressForReplace.addr = func;
#endif

                const auto findRxPage = [](ptr hookAddr, bool direction) -> ptr {
                    while (true) {
                        svc::MemoryInfo info;
                        u32 page;
                        HK_ABORT_UNLESS_R(svc::QueryMemory(&info, &page, hookAddr));

                        diag::debugLog("%zx %zx %zx %d %d", hookAddr, info.base_address, info.size, info.state, info.permission);

                        if (info.state == svc::MemoryState_Free && info.size >= cPageSize)
                            return info.base_address + info.size - cPageSize;

                        if (direction == true)
                            hookAddr += info.size;
                        else
                            hookAddr -= info.size + 1;
                    }
                };

                ptr rxMapUnder = findRxPage(hookAddr, false);
                ptr rxMapAbove = findRxPage(hookAddr, true);

                ptr rwMap = getFarRwMap();
                ptr offsetIntoMap = getOffsetIntoFarMap();

                mFarRxMap = abs(s64(rxMapUnder + offsetIntoMap) - s64(rwMap + offsetIntoMap)) < abs(s64(rxMapAbove + offsetIntoMap) - s64(rwMap + offsetIntoMap)) ? rxMapUnder : rxMapAbove;

                s64 distance = checkBranchDistanceExceeded(hookAddr, mFarRxMap + offsetIntoMap);
                HK_ABORT_UNLESS(distance == 0, "FarBackup: Branch exceeded max branch distance (%zd > %zu)", distance, cMaxBranchDistance);

                Result rc = mapRwToRx(rwMap, cPageSize, &mFarRxMap);

                svc::MemoryInfo info;
                u32 page;
                svc::QueryMemory(&info, &page, mFarRxMap);
                diag::debugLog("penisd %x %zx %zx %zx %d %d", rc, mFarRxMap, info.base_address, info.size, info.state, info.permission);
                defer {
                    if (rc.failed())
                        detail::sFarBackupPool.free(mFarBackup);
                };
                HK_TRY(rc);

                ptr branchTarget = mFarRxMap + offsetIntoMap;
                rc = hook::writeBranch(mModule, mOffset, Func(branchTarget));
                clearFarBackupCache();
                svc::clearCache(hookAddr, sizeof(Instr));
                HK_TRY(rc);

                return ResultSuccess();
            } else
                return hook::writeBranch(mModule, mOffset, mFunc);
        }

        Result restoreBranch() {
            if (mFarBackup != nullptr) {
                ptr rwMap = getFarRwMap();

                HK_TRY(unmapRwToRx(rwMap, cPageSize, mFarRxMap));

                detail::sFarBackupPool.free(mFarBackup);
                mFarBackup = nullptr;
                mFarRxMap = 0;
            }

            HK_TRY(mModule->writeRo(mOffset, mOrigInstr));
            svc::clearCache(mModule->range().start() + mOffset, sizeof(Instr));

            mModule = nullptr;
            mOffset = 0;
            mOrigInstr = 0;

            return ResultSuccess();
        }

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

            _HK_REPLACEHOOK_TRY(writeBranch());

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
            ptr addr = util::lookupSymbol<Symbol>();

            return installAtPtr(cast<void*>(addr));
        }

        virtual Result uninstall() {
            HK_UNLESS(isInstalled(), ResultNotInstalled());

            return restoreBranch();
        }

        Result installAtMainOffset(ptr offset) {
            return installAtOffset(ro::getMainModule(), offset);
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
template <typename Ret, typename... Args>
using HkReplaceVarArgs = hk::hook::ReplaceHook<Ret (*)(Args..., ...)>;
