#pragma once

#include "hk/diag/diag.h"
#include "hk/sf/sf.h"
#include "hk/types.h"
#include <array>

namespace hk::sf {
    template <typename... Args>
    constexpr size calcParamsSize() {
        u32 totalSize = 0;
        ([&] {
            totalSize += sizeof(Args);
        }(),
            ...);
        return totalSize;
    }

    template <typename... Args>
    std::array<u8, calcParamsSize<Args...>()> packInput(const Args&... args) {
        std::array<u8, calcParamsSize<Args...>()> array = {};
        ptr offset = 0;
        ([&] {
            *cast<Args*>(&array[offset]) = args;
            offset += sizeof(Args);
        }(),
            ...);
        return array;
    }

    template <typename T>
    auto simpleDataHandler() {
        return [](sf::Response& response) -> T {
            HK_ASSERT(response.data.size_bytes() >= sizeof(T));
            return *cast<T*>(response.data.data());
        };
    }
}
