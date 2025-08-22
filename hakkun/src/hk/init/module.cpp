#include "hk/init/module.h"
#include "hk/diag/diag.h"
#include "hk/ro/ModuleHeader.h"
#include "hk/ro/RoUtil.h"
#include "hk/sail/init.h"
#include "hk/svc/api.h"
#include "rtld/RoModule.h"

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

    extern "C" void __module_entry__(void* x0, void* x1) {
        diag::debugLog("Hakkun __module_entry__");

        ro::RoUtil::initModuleList();
#ifdef HK_DISABLE_SAIL
        diag::debugLog("hk::sail: disabled");
#else
        sail::loadSymbols();
#endif
#ifdef HK_ADDON_HeapSourceBss
        mem::initializeMainHeap();
#endif
        callInitializers();

#ifndef HK_STANDALONE
        hkMain();
#endif
    }

} // namespace hk::init

#ifdef HK_STANDALONE
extern "C" void hkMain();

namespace nn::init {

    using FuncPtr = void (*)();

    extern void Start(size threadHandle, size argumentAddr, FuncPtr notifyExceptionHandlerReady, FuncPtr callInitializers);
    void Start(size threadHandle, size argumentAddr, FuncPtr notifyExceptionHandlerReady, FuncPtr callInitializers) {
        // if (notifyExceptionHandlerReady)
        //     notifyExceptionHandlerReady();

        if (callInitializers)
            callInitializers();

        hkMain();
    }

} // namespace nn::init
#endif