# HeapSourceBss

This addon defines a global ExpHeap to use for malloc/new allocations. The size can be changed in the config with HAKKUN_BSS_HEAP_SIZE, 256KB by default. If HAKKUN_MAIN_HEAP_USER_ARENA is enabled, the user can override hk::mem::initializeMainHeap to create the heap (hk::mem::sMainHeap) with a custom arena.
Depends on ExpHeap.