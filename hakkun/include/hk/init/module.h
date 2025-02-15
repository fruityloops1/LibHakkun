#pragma once

#include "hk/types.h"
#include "rtld/RoModule.h"

namespace hk::init {

#define MODULE_NAME_STR STR(MODULE_NAME) ".nss"

    struct ModuleName {
        u32 _0 = 0;
        u32 nameLen = sizeof(MODULE_NAME_STR) - 1;
        char name[sizeof(MODULE_NAME_STR)] = MODULE_NAME_STR;
    };

    extern "C" {
    extern nn::ro::detail::RoModule hkRtldModule;
    extern u8 __module_start__;
    extern const Elf_Dyn _DYNAMIC[];
    extern const Elf_Rela __rela_start__[];
    extern const Elf_Rela __rela_end__[];

    using InitFuncPtr = void (*)();

    extern InitFuncPtr __preinit_array_start__[];
    extern InitFuncPtr __preinit_array_end__[];
    extern InitFuncPtr __init_array_start__[];
    extern InitFuncPtr __init_array_end__[];

    extern const ModuleName hkModuleName;
    }

    inline ptr getModuleStart() { return cast<ptr>(&__module_start__); }

} // namespace hk::init
