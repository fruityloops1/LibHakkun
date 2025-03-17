#pragma once

#include "hk/heap/lmem.h"

namespace hk::heap {
    using HeapHandle = nn::lmem::detail::HeapHead*;
    extern HeapHandle MainHeap;
    void initHeap();
}

extern "C" { 
    void* malloc(size_t size); 
    void free(void* ptr); 
    void* aligned_alloc(size_t alignment, size_t size);
};