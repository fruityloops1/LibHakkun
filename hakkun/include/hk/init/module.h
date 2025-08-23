#pragma once

#include "hk/types.h"
#include "hk/util/TemplateString.h"
#include "rtld/RoModule.h"

namespace hk::init {

    template <util::TemplateString Name>
    struct ModuleName {
        u32 attribType = 0;
        u32 nameLen = sizeof(Name) - 1;
        decltype(Name) name = Name;
    };

    extern "C" {
    extern nn::ro::detail::RoModule hkRtldModule;
    extern u8 __module_start__;
    extern u8 __module_end__;
    extern const Elf_Dyn _DYNAMIC[];
    extern const Elf_Rela __rela_start__[];
    extern const Elf_Rela __rela_end__[];

    using InitFuncPtr = void (*)();

    extern InitFuncPtr __preinit_array_start__[];
    extern InitFuncPtr __preinit_array_end__[];
    extern InitFuncPtr __init_array_start__[];
    extern InitFuncPtr __init_array_end__[];
    }

    hk_alwaysinline inline void callInitializers() {
        InitFuncPtr* current = __preinit_array_start__;
        while (current != __preinit_array_end__)
            (*current++)();
        current = __init_array_start__;
        while (current != __init_array_end__)
            (*current++)();
    }

    inline ptr getModuleStart() { return cast<ptr>(&__module_start__); }
    inline ptr getModuleEnd() { return cast<ptr>(&__module_end__); }

    inline nn::ro::detail::RoModule* getSelfRtldModule() { return &hkRtldModule; }

} // namespace hk::init
