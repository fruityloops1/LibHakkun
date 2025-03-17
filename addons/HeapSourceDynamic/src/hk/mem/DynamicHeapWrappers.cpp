#include <cstddef>
#include <new>

extern "C" {
void* hk_Znwm(std::size_t size);
void* hk_ZnwmRKSt9nothrow_t(std::size_t size, const std::nothrow_t&);
void* hk_Znam(std::size_t size);
void* hk_ZnamRKSt9nothrow_t(std::size_t size, const std::nothrow_t&);
void hk_ZdlPv(void* ptr);
void hk_ZdlPvRKSt9nothrow_t(void* ptr, const std::nothrow_t&);
void hk_ZdlPvm(void* ptr, std::size_t size);
void hk_ZdaPv(void* ptr);
void hk_ZdaPvRKSt9nothrow_t(void* ptr, const std::nothrow_t&);
void hk_ZdaPvm(void* ptr, std::size_t size);
void* hk_malloc(std::size_t size);
void* hk_realloc(void* ptr, std::size_t size);
void* hk_aligned_alloc(std::size_t alignment, std::size_t size);
void hk_free(void* ptr);
}
void* operator new(std::size_t size) {
    return hk_Znwm(size);
}

void* operator new(std::size_t size, const std::nothrow_t& t) noexcept {
    return hk_ZnwmRKSt9nothrow_t(size, t);
}

void* operator new[](std::size_t size) {
    return hk_Znam(size);
}

void* operator new[](std::size_t size, const std::nothrow_t& t) noexcept {
    return hk_ZnamRKSt9nothrow_t(size, t);
}

void operator delete(void* ptr) {
    return hk_ZdlPv(ptr);
}

void operator delete(void* ptr, const std::nothrow_t& t) {
    return hk_ZdlPvRKSt9nothrow_t(ptr, t);
}

void operator delete(void* ptr, std::size_t size) {
    return hk_ZdlPvm(ptr, size);
}

void operator delete[](void* ptr) {
    return hk_ZdaPv(ptr);
}

void operator delete[](void* ptr, const std::nothrow_t& t) {
    return hk_ZdaPvRKSt9nothrow_t(ptr, t);
}

void operator delete[](void* ptr, std::size_t size) {
    return hk_ZdaPvm(ptr, size);
}

extern "C" {
void* malloc(std::size_t size) {
    return hk_malloc(size);
}
void* aligned_alloc(std::size_t alignment, std::size_t size) {
    return hk_aligned_alloc(alignment, size);
}
void* realloc(void* ptr, std::size_t size) {
    return hk_realloc(ptr, size);
}
void free(void* ptr) {
    hk_free(ptr);
}
void* calloc(std::size_t nmemb, std::size_t size) {
    void* ptr = hk_malloc(nmemb * size);
    __builtin_memset(ptr, 0, nmemb * size);
    return ptr;
}
}
