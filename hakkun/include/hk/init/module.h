#pragma once

#include "hk/hook/InstrUtil.h"
#include "hk/ro/ElfUtil.h"
#include "hk/ro/ModuleHeader.h"
#include "hk/svc/api.h"
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

    hk_alwaysinline inline nn::ro::detail::RoModule* initializeRtldModule(ptr moduleBase, bool ignoreSelf = false) {
        struct {
            hk::hook::Instr bInstr;
            u32 offset;
        } const* const offs = cast<decltype(offs)>(moduleBase);

        const hk::ro::ModuleHeader* header = cast<const hk::ro::ModuleHeader*>(moduleBase + offs->offset);
        const Elf_Dyn* dynamic = cast<const Elf_Dyn*>(ptr(header) + header->dynamicOffset);
        nn::ro::detail::RoModule* module = cast<nn::ro::detail::RoModule*>(ptr(header) + header->moduleOffset);

        u8* bssStart = cast<u8*>(ptr(header) + header->bssOffset);
        const size bssSize = header->bssEndOffset - header->bssOffset;

        memset(bssStart, 0, bssSize);

        new (module) nn::ro::detail::RoModule;

        const bool skip = ignoreSelf && module == init::getSelfRtldModule();

        if (!skip) {
            module->Initialize(moduleBase, dynamic);
            module->Relocate();
        }

        return module;
    }

    hk_alwaysinline inline ptr initializeSelfModule() {
        ptr moduleBase;
        __asm("adr %[result], __module_start__" : [result] "=r"(moduleBase));

        initializeRtldModule(moduleBase);

        return moduleBase;
    }

} // namespace hk::init
