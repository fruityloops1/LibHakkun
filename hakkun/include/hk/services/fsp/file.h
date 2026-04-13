#pragma once

#include "hk/Result.h"
#include "hk/sf/hipc.h"
#include "hk/sf/sf.h"
#include "hk/sf/utils.h"
#include "hk/types.h"

namespace hk::fsp {
    class IFile : public sf::Service {
    public:
        IFile(sf::Service&& service)
            : sf::Service(forward<sf::Service>(service)) { }

        template <typename T>
        ValueOrResult<u64> read(Span<T> data, s64 offset) {
            auto input = sf::packInput(u32(0), offset, u64(data.size()));
            auto request = sf::Request(this, 0, &input);
            request.addOutMapAlias(data, sf::hipc::BufferMode::NonSecure);
            return invokeRequest(move(request), sf::inlineDataExtractor<u64>());
        }

        template <typename T>
        Result write(Span<const T> data, s64 offset, bool flush = false) {
            auto input = sf::packInput(u32(flush), offset, u64(data.size()));
            auto request = sf::Request(this, 1, &input);
            request.addInMapAlias(data, sf::hipc::BufferMode::NonSecure);
            return invokeRequest(move(request));
        }
    };
} // namespace hk::fsp
