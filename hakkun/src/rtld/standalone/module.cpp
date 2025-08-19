#include "hk/init/module.h"
#include "hk/ro/ModuleHeader.h"
#include "rtld/RoModule.h"

extern "C" {
section(.bss.rtldmodule) nn::ro::detail::RoModule hkRtldModule;
extern hk::ro::ModuleHeader __mod0;
section(.rodata.modulename) const hk::init::ModuleName<"rtld.nss"> hkModuleName;
}