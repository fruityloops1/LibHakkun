#include "hk/mem/BssHeap.h"
#include <new>

extern "C" {
void* malloc(std::size_t size) {
    return hk::mem::sMainHeap.allocate(size);
}
void* aligned_alloc(std::size_t alignment, std::size_t size) {
    return hk::mem::sMainHeap.allocate(size, alignment);
}
void* realloc(void* ptr, std::size_t size) {
    return hk::mem::sMainHeap.reallocate(ptr, size);
}
void free(void* ptr) {
    hk::mem::sMainHeap.free(ptr);
}
void* calloc(std::size_t nmemb, std::size_t size) {
    void* ptr = malloc(nmemb * size);
    __builtin_memset(ptr, 0, nmemb * size);
    return ptr;
}
}

void* operator new(std::size_t size) {
    return malloc(size);
}

void* operator new(std::size_t size, const std::nothrow_t& t) noexcept {
    return malloc(size);
}

void* operator new[](std::size_t size) {
    return malloc(size);
}

void* operator new[](std::size_t size, const std::nothrow_t& t) noexcept {
    return malloc(size);
}

void operator delete(void* ptr) {
    return free(ptr);
}

void operator delete(void* ptr, const std::nothrow_t& t) {
    return free(ptr);
}

void operator delete(void* ptr, std::size_t size) {
    return free(ptr);
}

void operator delete[](void* ptr) {
    return free(ptr);
}

void operator delete[](void* ptr, const std::nothrow_t& t) {
    return free(ptr);
}

void operator delete[](void* ptr, std::size_t size) {
    return free(ptr);
}
