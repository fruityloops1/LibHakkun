#pragma once

#include "hk/svc/types.h"

namespace hk::svc {

#ifdef __aarch64__
    inline hk_alwaysinline u64 getSystemTickFrequency() {
        volatile u64 freq;
        asm volatile("mrs %x0, cntfrq_el0" : "=r"(freq));
        return freq;
    }
    inline hk_alwaysinline u64 getSystemTick() {
        volatile u64 tick;
        asm volatile("mrs %x0, cntpct_el0" : "=r"(tick));
        return tick;
    }
    inline hk_alwaysinline void* getTLSPtr() {
        void* volatile ptr;
        asm volatile("mrs %0, tpidrro_el0" : "=r"(ptr));
        return ptr;
    }

    inline u32 loadExclusive(volatile const u32* ptr) {
        u32 value;
        asm volatile(
            "ldaxr %w[value], %[ptr]"
            : [value] "=&r"(value)
            : [ptr] "Q"(*ptr)
            : "memory");
        return value;
    }

    inline bool storeExclusive(volatile u32* ptr, u32 value) {
        int result;
        asm volatile(
            "stlxr %w[result], %w[value], %[ptr]"
            : [result] "=&r"(result)
            : [value] "r"(value), [ptr] "Q"(*ptr)
            : "memory");
        return result == 0;
    }

    inline void clearExclusive() {
        asm volatile("clrex" ::: "memory");
    }
#else
    inline hk_alwaysinline u64 getSystemTickFrequency() {
        volatile u32 freq;
        asm volatile("mrc p15, 0, %0, cr14, c0, 0" : "=r"(freq));
        return freq;
    }
    inline hk_alwaysinline u64 getSystemTick() {
        volatile u64 tick;
        asm volatile("mrrc p15, 0x0, %Q0, %R0, cr14" : "=r"(tick));
        return tick;
    }
    inline __attribute__((always_inline)) void* getTLSPtr() {
        void* volatile ptr;
        asm volatile("mrc p15, 0, %0, cr13, c0, 0x3" : "=r"(ptr));
        return ptr;
    }

    inline uint32_t loadExclusive(volatile const u32* ptr) {
        u32 value;
        asm volatile(
            "ldrex %0, [%1]"
            : "=r"(value)
            : "r"(ptr)
            : "memory");
        return value;
    }

    inline bool storeExclusive(volatile u32* ptr, u32 value) {
        u32 result;
        asm volatile(
            "strex %0, %1, [%2]"
            : "=r"(result)
            : "r"(value), "r"(ptr)
            : "memory");
        return result == 0;
    }

    inline void clearExclusive() {
        // ...
    }
#endif

    inline hk_alwaysinline ThreadLocalRegion* getTLS() {
        return cast<ThreadLocalRegion*>(getTLSPtr());
    }

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

} // namespace hk::svc
