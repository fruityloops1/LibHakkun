#pragma once

#include "hk/diag/diag.h"
#include "hk/sf/sf.h"
#include "hk/types.h"
#include "hk/util/TypeName.h"
#include <array>
#include <type_traits>

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
    constexpr auto inlineDataExtractor() {
        if constexpr (std::is_same_v<T, void>)
            return [](sf::Response& response) -> void { };
        else
            return [](sf::Response& response) -> T {
                HK_ABORT_UNLESS(response.data.size_bytes() >= sizeof(T), "hk::sf::inlineDataExtractor: response too small (%zu < sizeof(%s)==%zu)", response.data.size_bytes(), util::getTypeName<T>(), sizeof(T));
                return *cast<T*>(response.data.data());
            };
    }

    constexpr auto subserviceExtractor(Service* currentService) {
        return [currentService](sf::Response& response) -> sf::Service { return response.nextSubservice(currentService); };
    }

    constexpr auto handleExtractor() {
        return [](sf::Response& response) -> Handle { return response.nextCopyHandle(); };
    }

    template <typename T = void, typename... Args>
    ValueOrResult<T> invokeSimple(sf::Service& service, s32 id, const Args&... args) {
        auto input = packInput(args...);
        if constexpr (std::is_same_v<T, sf::Service>) {
            return service.invokeRequest(sf::Request(&service, id, &input), subserviceExtractor(&service));
        } else {
            return service.invokeRequest(sf::Request(&service, id, &input), inlineDataExtractor<T>());
        };
    }

} // namespace hk::sf
