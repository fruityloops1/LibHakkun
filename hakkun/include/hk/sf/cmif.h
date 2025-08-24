#pragma once

#include "hk/types.h"

namespace hk::sf::cmif {

    static constexpr u32 cInHeaderMagic = 0x49434653;

    struct InHeader {
        u32 magic = 0;
        u32 version;
        u32 command;
        u32 token;
    };

    struct OutHeader {
        u32 magic = 0;
        u32 version;
        Result result;
        u32 token;
    };

    enum class DomainTag : u8;
    struct DomainInHeader {
        DomainTag tag;
        u8 objectCount;
        u16 dataSize;
        u32 objectId;
        u32 _padding;
        u32 token;
    };

    struct DomainOutHeader {
        u32 objectCount;
        u32 _padding[3];
    };

    enum class MessageTag : u16 {
        Close = 2,
        Request = 4,
        Control = 5,
    };

    enum class DomainTag : u8 {
        Request = 0,
        Close = 1,
    };

} // namespace hk::sf::cmif
