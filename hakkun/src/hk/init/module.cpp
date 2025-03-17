#include "hk/init/module.h"
#include "hk/ro/RoUtil.h"
#include "hk/sail/init.h"
#include "hk/svc/api.h"
#include "rtld/ModuleHeader.h"
#include "rtld/RoModule.h"

#ifdef HK_ADDON_HeapSourceBss
#include "hk/mem/BssHeap.h"
#endif

namespace hk::init {

    extern "C" {
    section(.bss.rtldmodule) nn::ro::detail::RoModule hkRtldModule;
    extern rtld::ModuleHeader __mod0;
    section(.rodata.modulename) const ModuleName hkModuleName;
    }

    static void callInitializers() {
        InitFuncPtr* current = __preinit_array_start__;
        while (current != __preinit_array_end__)
            (*current++)();
        current = __init_array_start__;
        while (current != __init_array_end__)
            (*current++)();
    }

    extern "C" void hkMain();

    extern "C" void __module_entry__(void* x0, void* x1) {
        constexpr char msg[] = "Hakkun __module_entry__";
        svc::OutputDebugString(msg, sizeof(msg));

        ro::initModuleList();
#ifndef HK_DISABLE_SAIL
        sail::loadSymbols();
#endif
#ifdef HK_ADDON_HeapSourceBss
        if (mem::initializeMainHeap)
            mem::initializeMainHeap();
#endif
        callInitializers();

        hkMain();
    }

} // namespace hk::init
