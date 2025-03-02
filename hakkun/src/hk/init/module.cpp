#include "hk/init/module.h"
#include "hk/ro/RoUtil.h"
#include "hk/sail/init.h"
#include "hk/svc/api.h"
#include "rtld/ModuleHeader.h"
#include "rtld/RoModule.h"

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
        callInitializers();

        hkMain();
    }

} // namespace hk::init
