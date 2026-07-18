#include "hk/init/module.h"
#include "hk/diag/diag.h"
#include "hk/ro/ModuleHeader.h"
#include "hk/ro/RoUtil.h"
#include "hk/sail/init.h"
#include "hk/svc/cpu.h"
#include "hk/svc/types.h"
#include "rtld/RoModule.h"
#include "rtld/RoModuleList.h"

#ifdef HK_ADDON_HeapSourceBss
#include "hk/mem/BssHeap.h"
#endif

namespace hk::init {

    extern "C" {
    section(.bss.rtldmodule) nn::ro::detail::RoModule hkRtldModule;
    extern hk::ro::ModuleHeader __mod0;
    section(.rodata.modulename) extern const ModuleName<STR(MODULE_NAME) ".nss"> hkModuleName;
    section(.rodata.modulename) const ModuleName<STR(MODULE_NAME) ".nss"> hkModuleName;
    }

    extern "C" void hkMain();

#if HK_HOMEBREW_TYPE == HK_HOMEBREW_HBLOADER
    static nn::ro::detail::RoModuleList g_AutoLoadForHomebrew;
#endif

    extern "C" void __module_entry__(void* x0, void* x1) {
#if HK_HOMEBREW_TYPE == HK_HOMEBREW_HBLOADER
        initializeSelfModule();

        g_AutoLoadForHomebrew.pushFront(&hkRtldModule);
        nn::ro::detail::g_pAutoLoadList = &g_AutoLoadForHomebrew;
#endif

        diag::logLine("Hakkun __module_entry__");

        ro::RoUtil::initModuleList();
#ifdef HK_DISABLE_SAIL
        diag::logLine("hk::sail: disabled");
#else
        sail::loadSymbols();
#endif
#ifdef HK_ADDON_HeapSourceBss
        mem::initializeMainHeap();
#endif
        callInitializers();

#if HK_TARGET == HK_TARGET_MODULE or HK_TARGET == HK_TARGET_MODULE_DLL
        hkMain();
#endif
    }

} // namespace hk::init

#if HK_TARGET == HK_TARGET_MODULE_STANDALONE
extern "C" void hkMain();

namespace nn::init {

    using FuncPtr = void (*)();

    extern void Start(size threadHandle, size argumentAddr, FuncPtr notifyExceptionHandlerReady, FuncPtr callInitializers);
    void Start(size threadHandle, size argumentAddr, FuncPtr notifyExceptionHandlerReady, FuncPtr callInitializers) {
        // if (notifyExceptionHandlerReady)
        //     notifyExceptionHandlerReady();

        static hk::svc::ThreadType sMainThreadType;

        hk::svc::getTLS()->nnsdkThread = &sMainThreadType;
        hk::diag::setCurrentThreadName("HkMain");

        if (callInitializers)
            callInitializers();

        hkMain();
    }

} // namespace nn::init
#endif
