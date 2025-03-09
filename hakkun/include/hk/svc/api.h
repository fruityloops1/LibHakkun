#pragma once

#include "hk/diag/diag.h"
#include "hk/svc/types.h"

namespace hk::svc {

#ifdef __aarch64__
    inline hk_alwaysinline void* getTLSPtr() {
        void* volatile ptr;
        asm volatile("mrs %0, tpidrro_el0" : "=r"(ptr));
        return ptr;
    }
#else
    inline __attribute__((always_inline)) void* getTLSPtr() {
        void* volatile ptr;
        asm volatile("mrc p15, 0, %0, cr13, c0, 0x3" : "=r"(ptr));
        return ptr;
    }
#endif

    inline hk_alwaysinline ThreadLocalRegion* getTLS() {
        return cast<ThreadLocalRegion*>(getTLSPtr());
    }

#ifdef __aarch64__
    inline hk_alwaysinline u64 getSystemTick() {
        volatile u64 tick;
        asm volatile("mrs %x0, cntpct_el0" : "=r"(tick));
        return tick;
    }
#else
    inline hk_alwaysinline u64 getSystemTick() {
        volatile u64 tick;
        asm volatile("mrrc p15, 0x0, %Q0, %R0, cr14" : "=r"(tick));
        return tick;
    }
#endif

#ifdef __aarch64__
    inline hk_alwaysinline u64 getSystemTickFrequency() {
        volatile u64 freq;
        asm volatile("mrs %x0, cntfrq_el0" : "=r"(freq));
        return freq;
    }
#else
    inline hk_alwaysinline u64 getSystemTickFrequency() {
        volatile u32 freq;
        asm volatile("mrc p15, 0, %0, cr14, c0, 0" : "=r"(freq));
        return freq;
    }
#endif

    inline hk_alwaysinline void prefetchRegion(ptr addr, ptrdiff size) {
#pragma clang loop unroll(enable)
        while (size > 0) {
            __builtin_prefetch((void*)addr);
            size -= 64;
            addr += 64;
        }
    }

    inline hk_alwaysinline void prefetchRegionStream(ptr addr, ptrdiff size) {
#pragma clang loop unroll(enable)
        while (size > 0) {
            __builtin_prefetch((void*)addr, 0, 0);
            size -= 64;
            addr += 64;
        }
    }

    Result QueryMemory(MemoryInfo* outMemoryInfo, u32* outPageInfo, ptr address);
    hk_noreturn Result Break(BreakReason reason, void* arg, size argSize);
    Result OutputDebugString(const char* str, size_t len);
    Result GetInfo(u64* out, InfoType type, svc::Handle handle, u64 subType);
    Result InvalidateProcessDataCache(svc::Handle process, ptr addr, size size);
    Result FlushProcessDataCache(svc::Handle process, ptr addr, size size);
    Result MapProcessMemory(ptr dest, svc::Handle process, u64 source, size size);

    hk_noreturn Result hkBreakWithMessage(BreakReason reason, void* arg, size argSize, void* headerSym, void* msgSym);

    inline hk_alwaysinline Result getProcessHandleMesosphere(Handle* out) {
        u64 value = 0;
        Result rc = GetInfo(&value, InfoType_MesosphereCurrentProcess, 0, 0);
        if (rc.succeeded())
            *out = Handle(value);
        return rc;
    }

    inline hk_alwaysinline void clearCache(ptr addr, ptrdiff size) {
        auto* tls = getTLS();
        tls->cache_maintanence_flag = 1;
#ifdef __aarch64__
        __builtin___clear_cache((char*)addr, (char*)addr + size);
#else // ILP32 userland cannot flush by itself
        svc::Handle process;
        Result rc = getProcessHandleMesosphere(&process);
#ifndef HK_RELEASE
        HK_ABORT_UNLESS_R(rc);
#endif
        svc::FlushProcessDataCache(process, addr, size);
        svc::InvalidateProcessDataCache(process, addr, size);
#endif
        tls->cache_maintanence_flag = 0;
    }

} // namespace hk::svc
