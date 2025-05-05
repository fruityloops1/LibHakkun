#pragma once

#include "hk/types.h"

namespace nn::diag {

    struct LogMetaData {
        size line = 0;
        const char* file = nullptr;
        const char* function = nullptr;
        const char* _18 = "";
        u32 _20 = 1;
        u32 _24 = 0;
        void* _28 = nullptr;
        void* _30 = nullptr;
        void* _38 = nullptr;
    };

    namespace detail {

        void PutImpl(const LogMetaData& metaData, const char* msg, size len);

    } // namespace detail

} // namespace nn::diag
