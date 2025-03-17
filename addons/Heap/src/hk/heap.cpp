#include "hk/heap/heap.h"
#include "hk/util/Context.h"

#ifndef FAKE_HEAP_SIZE
#define FAKE_HEAP_SIZE 0x5000
#endif

namespace hk::heap {
    char __fake_heap[FAKE_HEAP_SIZE];
    char* fake_heap_start = __fake_heap;
    char* fake_heap_end = __fake_heap + FAKE_HEAP_SIZE;

    HeapHandle MainHeap = nullptr;

    void initHeap() {
        using create_signature = HeapHandle (*)(void*, size_t, int);
        create_signature create_heap = (create_signature)hk::util::lookupSymbol<"_ZN2nn4lmem13CreateExpHeapEPvmi">(); 
        MainHeap = create_heap(__fake_heap, FAKE_HEAP_SIZE, 4);
    }
}

extern "C" { 
    void* malloc(size_t size) { 
        using alloc_signature = void* (*)(nn::lmem::detail::HeapHead*, size_t, int);
        alloc_signature alloc_heap = (alloc_signature)hk::util::lookupSymbol<"_ZN2nn4lmem19AllocateFromExpHeapEPNS0_6detail8HeapHeadEm">();
        void* allocated = alloc_heap(hk::heap::MainHeap, size, 8); 
        return allocated;
    }
    void free(void* ptr) {
        using free_signature = void* (*)(nn::lmem::detail::HeapHead*, void*);
        free_signature free_heap = (free_signature)hk::util::lookupSymbol<"_ZN2nn4lmem13FreeToExpHeapEPNS0_6detail8HeapHeadEPv">();
        free_heap(hk::heap::MainHeap, ptr); 
    } 
    void* aligned_alloc(size_t alignment, size_t size) { 
        using alloc_signature = void* (*)(nn::lmem::detail::HeapHead*, size_t, int);
        alloc_signature alloc_heap = (alloc_signature)hk::util::lookupSymbol<"_ZN2nn4lmem19AllocateFromExpHeapEPNS0_6detail8HeapHeadEm">();
        void* allocated = alloc_heap(hk::heap::MainHeap, size, alignment); 
        return allocated; 
    }
}; 

//main heap 
void* operator new(size_t size) { return malloc(size); }
void operator delete(void* ptr) { free(ptr); }