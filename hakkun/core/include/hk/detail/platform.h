#ifndef INCLUDE_HK_DETAIL_PLATFORM
#error "do not include"
#endif

#if NNSDK
#define HAS_NNSDK(...) __VA_ARGS__
#define NOT_NNSDK(...)
#define HAS_NNSDK_TERNARY(DOES, DOESNT) DOES
#else
#define HAS_NNSDK(...)
#define NOT_NNSDK(...) __VA_ARGS__
#define HAS_NNSDK_TERNARY(DOES, DOESNT) DOESNT
#endif
