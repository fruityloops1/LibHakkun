#pragma once

#include "hk/ValueOrResult.h"
#include "hk/sf/sf.h"
#include "hk/sf/utils.h"
#include <span>

namespace hk::am::detail {
    class Storage : public sf::Service {
    public:
        Storage(sf::Service&& service)
            : sf::Service(forward<sf::Service>(service)) { }

        ValueOrResult<u64> getSize() {
            return sf::invokeSimple<u64>(*this, 0);
        }

        template <typename T>
        Result write(u64 offset, std::span<const T> data) {
            auto request = sf::Request(this, 10, &offset);
            request.addInAutoselect(data.data(), data.size_bytes());
            return invokeRequest(move(request));
        }

        template <typename T>
        Result read(u64 offset, std::span<T> data) {
            auto request = sf::Request(this, 11, &offset);
            request.addOutAutoselect(data.data(), data.size_bytes());
            return invokeRequest(move(request));
        }
    };
}
