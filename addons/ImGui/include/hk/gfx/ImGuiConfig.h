#pragma once

#include "hk/diag/diag.h"
#include "hk/svc/api.h"
#include <cstdarg>
#include <cstddef>
#include <cstdio>

namespace hk::gfx {

    inline void imguiLog(const char* fmt, ...) {
        std::va_list arg;
        va_start(arg, fmt);

        size_t size = vsnprintf(nullptr, 0, fmt, arg);
        char* buf = (char*)__builtin_alloca(size + 1);
        vsnprintf(buf, size + 1, fmt, arg);
        va_end(arg);

        svc::OutputDebugString(buf, size);
    }

} // namespace hk::gfx

#define IMGUI_DEBUG_PRINTF(_FMT, ...) \
    ::hk::gfx::imguiLog(_FMT, __VA_ARGS__)

#define IM_ASSERT(_EXPR) HK_ASSERT(_EXPR)
