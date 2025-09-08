#pragma once

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

        Handle getMessageEventHandle() {
            return HK_UNWRAP(invokeRequest(sf::Request(this, 0), sf::simpleHandleHandler()));
        }

        std::optional<u32> receiveMessage() {
            auto result = sf::invokeSimple<u32>(*this, 1);
            if (Result(result).getValue() == 0x680) return std::nullopt;
            return HK_UNWRAP(result);
        }
    };
}
