#include "hk/hook/InstrUtil.h"
#include "hk/ro/ModuleHeader.h"
#include "hk/svc/api.h"
#include "hk/svc/types.h"
#include "hk/types.h"
#include "rtld/types.h"

static void relocateSelf(ptr moduleBase, const Elf_Dyn* dynamic) {

    Elf_Addr rela = 0;
    Elf_Addr jmprel = 0;
    const char* dynstr = 0;
    const Elf_Sym* dynsym = 0;

    Elf_Xword rela_entry_size = sizeof(Elf_Rela);

    Elf_Xword rela_entry_count = 0;

    Elf_Xword rela_size = 0;

    for (; dynamic->d_tag != DT_NULL; dynamic++) {
        switch (dynamic->d_tag) {
        case DT_RELA:
            rela = (Elf_Addr(moduleBase) + dynamic->d_un.d_ptr);
            continue;

        case DT_JMPREL:
            jmprel = (Elf_Addr(moduleBase) + dynamic->d_un.d_ptr);
            continue;

        case DT_RELAENT:
            rela_entry_size = dynamic->d_un.d_val;
            continue;

        case DT_RELASZ:
            rela_size = dynamic->d_un.d_val;
            continue;

        case DT_SYMTAB:
            dynsym = (Elf_Sym*)(moduleBase + dynamic->d_un.d_val);
            continue;

        case DT_STRTAB:
            dynstr = (const char*)(moduleBase + dynamic->d_un.d_val);
            continue;

        case DT_RELACOUNT:
        case DT_RELCOUNT:
        case DT_REL:
        case DT_RELENT:
        case DT_RELSZ:
        case DT_NEEDED:
        case DT_PLTRELSZ:
        case DT_PLTGOT:
        case DT_HASH:
        case DT_STRSZ:
        case DT_SYMENT:
        case DT_INIT:
        case DT_FINI:
        case DT_SONAME:
        case DT_RPATH:
        case DT_SYMBOLIC:
        default:
            continue;
        }
    }

    if (rela_entry_count == 0)
        rela_entry_count = rela_size / rela_entry_size;

    if (rela_entry_count) {
        Elf_Xword i = 0;

        while (i < rela_entry_count) {
            Elf_Rela* entry = (Elf_Rela*)(rela + (i * rela_entry_size));

            switch (ELF_R_TYPE(entry->r_info)) {
            case ARCH_ABS32:
            case ARCH_ABS64:
            case ARCH_JUMP_SLOT:
            case ARCH_RELATIVE:
            case ARCH_GLOB_DAT: {
                Elf_Addr* ptr = (Elf_Addr*)(moduleBase + entry->r_offset);
                *ptr = moduleBase + entry->r_addend;
                break;
            }
            }
            i++;
        }
    }

    if (jmprel) {
        Elf_Xword i = 0;
        while (true) {
            Elf_Rela* entry = (Elf_Rela*)(jmprel + (i * rela_entry_size));
            if (entry->r_offset == 0)
                break;

            switch (ELF_R_TYPE(entry->r_info)) {
            case ARCH_ABS32:
            case ARCH_ABS64:
            case ARCH_JUMP_SLOT:
            case ARCH_RELATIVE:
            case ARCH_GLOB_DAT: {
                Elf_Addr* ptr = (Elf_Addr*)(moduleBase + entry->r_offset);
                *ptr = moduleBase + entry->r_addend;
                break;
            }
            }
            i++;
        }
    }
}

extern "C" void __module_entry__() {
    ptr moduleBase = ([]() -> ptr {
        ptr addr;
        __asm("adr %[result], ." : [result] "=r"(addr));
        hk::svc::MemoryInfo info;
        u32 page;

        // us
        HK_ASSERT(hk::svc::QueryMemory(&info, &page, addr).succeeded());
        return info.base_address;
    })();

    struct {
        hk::hook::Instr bInstr;
        u32 offset;
    } const* const offs = cast<decltype(offs)>(moduleBase);

    const hk::ro::ModuleHeader* header = cast<const hk::ro::ModuleHeader*>(moduleBase + offs->offset);
    const Elf_Dyn* dynamic = cast<const Elf_Dyn*>(moduleBase + header->dynamicOffset);

    relocateSelf(moduleBase, dynamic);
}