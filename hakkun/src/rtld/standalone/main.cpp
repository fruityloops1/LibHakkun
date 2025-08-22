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
#include <cstring>

namespace {

    nn::ro::detail::RoModuleList g_AutoLoad;
    nn::ro::detail::RoModuleList g_ManualLoad;
    // bool g_RoDebugFlag;

} // namespace

namespace nn {

    namespace ro::detail {

        RoModule::LookupGlobalManualFunctionPointer RoModule::g_LookupGlobalManualFunctionPointer = nullptr;

        ptr nn::ro::detail::RoModule::LookupGlobalAuto(const char* name) {
            for (auto* module : g_AutoLoad) {
                const Elf_Sym* sym = module->GetSymbolByName(name);
                if (sym) {
                    ptr value = module->m_Base + sym->st_value;
                    return value;
                }
            }
            return 0;
        }

    } // namespace ro::detail

    namespace init {

        extern "C" void __nnDetailInitLibc0();
        extern "C" void nnosInitialize(hk::Handle threadHandle, ptr argumentAddr);
        extern "C" void __nnDetailInitLibc1();
        extern "C" void __nnDetailInitLibc2();
        extern "C" void nndiagStartup();
        extern "C" void nninitStartup();
        extern "C" void nnMain();
        extern "C" void nnosQuickExit();

        extern "C" __attribute__((weak)) void nninitInitializeSdkModule(void);
        extern "C" __attribute__((weak)) void nninitInitializeAbortObserver(void);
        extern "C" __attribute__((weak)) void nninitFinalizeSdkModule(void);

        using FuncPtr = void (*)();

        __attribute__((weak)) void Start(size threadHandle, size argumentAddr, FuncPtr notifyExceptionHandlerReady, FuncPtr callInitializers);

    } // namespace init

} // namespace nn

namespace hk::rtld {

    static void relocateSelf(ptr moduleBase, const Elf_Dyn* dynamic) {
        const auto data = hk::ro::parseDynamic(moduleBase, dynamic);

        data.forEachRel([&](const Elf_Rel& entry) {
            Elf_Addr* ptr = cast<Elf_Addr*>(moduleBase + entry.r_offset);
            *ptr += moduleBase;
        });

        data.forEachRela([&](const Elf_Rela& entry) {
            Elf_Addr* ptr = cast<Elf_Addr*>(moduleBase + entry.r_offset);
            *ptr = moduleBase + entry.r_addend;
        });
    }

    void initialize(ptr selfModuleBase, const Elf_Dyn* dynamic) {
        relocateSelf(selfModuleBase, dynamic);
        hk::init::callInitializers();

        HK_ABORT_UNLESS_R(hk::ro::forEachAutoLoadModule(selfModuleBase, [&](ptr textBase, ptr rodataBase, ptr dataBase) {
            struct {
                hk::hook::Instr bInstr;
                u32 offset;
            } const* const offs = cast<decltype(offs)>(textBase);

            const hk::ro::ModuleHeader* header = cast<const hk::ro::ModuleHeader*>(textBase + offs->offset);
            const Elf_Dyn* dynamic = cast<const Elf_Dyn*>(ptr(header) + header->dynamicOffset);
            nn::ro::detail::RoModule* module = cast<nn::ro::detail::RoModule*>(ptr(header) + header->moduleOffset);

            new (module) nn::ro::detail::RoModule;

            module->Initialize(textBase, dynamic);
            module->Relocate();

            g_AutoLoad.pushFront(module);
        }));

        for (auto* module : g_AutoLoad) {
            auto applyByReference = [&]<typename T>(const char* name, T value) {
                auto* sym = module->GetSymbolByName(name);
                if (sym != nullptr && ELF_ST_BIND(sym->st_info) != STB_LOCAL) {
                    T* dest = cast<T*>(module->m_Base + sym->st_value);
                    *dest = value;
                }
            };

            applyByReference("_ZN2nn2ro6detail15g_pAutoLoadListE", &g_AutoLoad);
            applyByReference("_ZN2nn2ro6detail17g_pManualLoadListE", &g_ManualLoad);
            // applyByReference("_ZN2nn2ro6detail14g_pRoDebugFlagE", &g_RoDebugFlag);
            applyByReference("_ZN2nn2ro6detail36g_pLookupGlobalManualFunctionPointerE", &nn::ro::detail::RoModule::g_LookupGlobalManualFunctionPointer);
        }

        for (auto* module : g_AutoLoad)
            module->ResolveSymbols();
    }

    static void initModules() {
        for (auto iter = g_AutoLoad.rbegin(); iter != g_AutoLoad.rend(); ++iter) {
            auto* module = *iter;
            if (module->m_pInit != nullptr)
                module->m_pInit();
        }
    }

    void start(Handle threadHandle) {
        ptr argumentAddr = alignUpPage(init::getModuleEnd());

        if (nn::init::Start != nullptr)
            nn::init::Start(threadHandle, argumentAddr, []() { /* blah */ }, initModules);
        else {
            using namespace nn::init;

            __nnDetailInitLibc0();

            nnosInitialize(threadHandle, argumentAddr);

            // <- exception handler ready

            __nnDetailInitLibc1();

            nndiagStartup();

            if (nninitInitializeSdkModule != nullptr)
                nninitInitializeSdkModule();

            if (nninitInitializeAbortObserver != nullptr)
                nninitInitializeAbortObserver();

            nninitStartup();

            __nnDetailInitLibc2();

            initModules();

            nnMain();

            if (nninitFinalizeSdkModule != nullptr)
                nninitFinalizeSdkModule();

            nnosQuickExit();
        }
    }

    extern "C" void rtldEntry(ptr x0, Handle threadHandle) {
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
        const Elf_Dyn* dynamic = cast<const Elf_Dyn*>(ptr(header) + header->dynamicOffset);

        u8* bssStart = cast<u8*>(ptr(header) + header->bssOffset);
        const size bssSize = header->bssEndOffset - header->bssOffset;

        memset(bssStart, 0, bssSize);

        initialize(moduleBase, dynamic);
        start(threadHandle);
        svc::ExitProcess();
    }

    extern "C" void rtldInit() { }
    extern "C" void rtldFini() { }

} // namespace hk::rtld
