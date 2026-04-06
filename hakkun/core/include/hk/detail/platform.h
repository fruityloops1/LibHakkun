#ifndef INCLUDE_HK_DETAIL_PLATFORM
#error "do not include"
#endif

#if NNSDK
#define HAS_NNSDK(...) __VA_ARGS__
#else
#define HAS_NNSDK(...)
#endif
