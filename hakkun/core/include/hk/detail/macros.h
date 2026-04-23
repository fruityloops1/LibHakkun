#ifndef INCLUDE_HK_DETAIL_MACROS
#error "do not include"
#endif

#pragma once

#define hk_alwaysinline __attribute__((always_inline))
#define hk_noinline __attribute__((noinline))
#define hk_noreturn __attribute__((noreturn))
#define section(SECTION) __attribute__((section(#SECTION)))
#define STRINGIFY(X) #X
#define STR(X) STRINGIFY(X)
#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)

#define NON_COPYABLE(CLASS)                 \
    constexpr CLASS(const CLASS&) = delete; \
    constexpr CLASS& operator=(const CLASS&) = delete

#define NON_MOVABLE(CLASS)             \
    constexpr CLASS(CLASS&&) = delete; \
    constexpr CLASS& operator=(CLASS&&) = delete

#ifdef HK_RELEASE
#define ralwaysinline hk_alwaysinline
#else
#define ralwaysinline
#endif

#undef INCLUDE_HK_DETAIL_MACROS