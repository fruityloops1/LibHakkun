#include "hk/diag/diag.h"
#include "hk/ro/RoModule.h"
#include "hk/svc/api.h"
#include "hk/svc/types.h"

section(.bss.rtldmodule) nn::ro::detail::RoModule hkRtldModule;

#undef HK_ASSERT
#define HK_ASSERT(_EXPR)                    \
    {                                       \
        if ((_EXPR) == false) {             \
            *(int*)(0x1000 + __LINE__) = 0; \
            __builtin_trap();               \
        }                                   \
    }

extern "C" void __module_entry__(void* x0, void* x1) {
    ptr addr;
    __asm("adr %[result], ." : [result] "=r"(addr));
    hk::svc::MemoryInfo info;
    u32 page;

    // us
    HK_ASSERT(hk::svc::QueryMemory(&info, &page, addr).succeeded());
    HK_ASSERT(info.permission & hk::svc::MemoryPermission_ReadExecute);
    addr = info.base_address + info.size;

    HK_ASSERT(hk::svc::QueryMemory(&info, &page, addr).succeeded());
    HK_ASSERT(info.permission & hk::svc::MemoryPermission_Read);
    addr = info.base_address + info.size;

    HK_ASSERT(hk::svc::QueryMemory(&info, &page, addr).succeeded());
    HK_ASSERT(info.permission & hk::svc::MemoryPermission_ReadWrite);
    addr = info.base_address + info.size;

    // main
    HK_ASSERT(hk::svc::QueryMemory(&info, &page, addr).succeeded());
    HK_ASSERT(info.permission & hk::svc::MemoryPermission_ReadExecute);

    using Entry = void (*)(void*, void*);
    Entry entry = reinterpret_cast<Entry>(info.base_address);
    entry(x0, x1);
}
