#pragma once

#include "hk/ValueOrResult.h"
#include "hk/sf/sf.h"
#include "hk/sf/utils.h"
#include "hk/util/Singleton.h"
#include <optional>

namespace hk::am::detail {
    class CommonStateGetter : public sf::Service {
        HK_SINGLETON(CommonStateGetter);

    public:
        CommonStateGetter(sf::Service&& service)
            : sf::Service(forward<sf::Service>(service)) { }

        ValueOrResult<Handle> getMessageEventHandle() {
            return sf::invokeSimple<Handle>(*this, 0);
        }

        ValueOrResult<std::optional<u32>> receiveMessage() {
            auto result = sf::invokeSimple<u32>(*this, 1);
            if (Result(result).getValue() == 0x680)
                return std::optional<u32>(std::nullopt);
            return result.map([](u32 value) { return std::optional(value); });
        }
    };
}
