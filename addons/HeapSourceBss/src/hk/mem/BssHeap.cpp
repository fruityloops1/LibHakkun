#include "hk/mem/BssHeap.h"

namespace hk::mem {

    constexpr size cHeapSize = HAKKUN_BSS_HEAP_SIZE;

    ExpHeap sMainHeap;

#ifndef HAKKUN_MAIN_HEAP_USER_ARENA
    __attribute__((aligned(cPageSize))) static u8 sMainHeapMem[cHeapSize] { 0 };

    __attribute__((weak)) void initializeMainHeap() {
        sMainHeap.initialize(sMainHeapMem, cHeapSize);
    }
#endif

} // namespace hk::mem
