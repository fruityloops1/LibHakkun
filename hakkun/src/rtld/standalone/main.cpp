#include "hk/ValueOrResult.h"
#include "hk/hook/InstrUtil.h"
#include "hk/init/module.h"
#include "hk/ro/ElfUtil.h"
#include "hk/ro/ModuleHeader.h"
#include "hk/ro/RoUtil.h"
#include "hk/svc/api.h"
#include "hk/svc/types.h"
#include "hk/types.h"
#include "rtld/RoModule.h"
#include "rtld/RoModuleList.h"
#include "rtld/types.h"

namespace hk::rtld {

    namespace {

        nn::ro::detail::RoModuleList g_AutoLoad;

    } // namespace

    static void relocateSelf(ptr moduleBase, const Elf_Dyn* dynamic) {
        const auto data = hk::ro::parseDynamic(moduleBase, dynamic);

        data.forEachPlt([&](const Elf_Rela& entry) {
            Elf_Addr* ptr = cast<Elf_Addr*>(moduleBase + entry.r_offset);
            *ptr = moduleBase + entry.r_addend;
        });
    }

    void initialize(ptr selfModuleBase, const Elf_Dyn* dynamic) {
        relocateSelf(selfModuleBase, dynamic);
        hk::init::callInitializers();

        HK_ABORT_UNLESS_R(hk::ro::forEachAutoLoadModule(selfModuleBase, [&](ptr textBase, ptr rodataBase, ptr bssBase) {
            nn::ro::detail::RoModule* module = cast<nn::ro::detail::RoModule*>(bssBase);

            module->Initialize(textBase, dynamic);
            module->Relocate();

            g_AutoLoad.pushBack(module);
        }));
    }

    void start() {
    }

    extern "C" void __module_entry__() {
        ptr moduleBase = HK_UNWRAP(([]() hk_alwaysinline -> ValueOrResult<ptr> {
            ptr addr;
            __asm("adr %[result], ." : [result] "=r"(addr));
            hk::svc::MemoryInfo info;
            u32 page;

            // us
            HK_TRY(hk::svc::QueryMemory(&info, &page, addr));
            return ptr(info.base_address);
        })());

        struct {
            hk::hook::Instr bInstr;
            u32 offset;
        } const* const offs = cast<decltype(offs)>(moduleBase);

        const hk::ro::ModuleHeader* header = cast<const hk::ro::ModuleHeader*>(moduleBase + offs->offset);
        const Elf_Dyn* dynamic = cast<const Elf_Dyn*>(moduleBase + header->dynamicOffset);

        initialize(moduleBase, dynamic);
        start();
        svc::ExitProcess();
    }

} // namespace hk::rtld
