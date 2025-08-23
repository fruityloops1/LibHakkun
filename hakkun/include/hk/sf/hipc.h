#pragma once

#include "hk/types.h"

namespace hk::sf::hipc {

    struct Header {
        u32 tag : 16;
        u32 sendStaticCount : 4;
        u32 sendBufferCount : 4;
        u32 recvBufferCount : 4;
        u32 exchBufferCount : 4;
        u32 dataWords : 10;
        u32 : 21;
        bool hasSpecialHeader : 1;
    };

    struct SpecialHeader {
        bool sendPid : 1;
        u32 copyHandleCount : 4;
        u32 moveHandleCount : 4;
        u32 pad : 23;
    };

    struct Static {
        u32 index : 6;
        u32 addressHigh : 6;
        u32 addressMid : 4;
        u32 size : 16;
        u32 addressLow;

        Static(u8 index, u64 address, u16 size)
            : index(index)
            , size(size) {
            addressLow = u32(address & hk::bits(32));
            addressMid = u32((address >> 32) & hk::bits(4));
            addressHigh = u32((address >> 36) & hk::bits(6));
        }

        u64 address() const {
            return u64(addressLow) | (u64(addressMid) << 32) | (u64(addressHigh) << 36);
        }
    };

    enum class BufferMode : u32 {
        Normal = 0,
        NonSecure = 1,
        NonDevice = 3,
    };

    struct Buffer {
        u32 sizeLow;
        u32 addressLow;
        BufferMode mode : 2;
        u32 addressHigh : 22;
        u32 sizeHigh : 4;
        u32 addressMid : 4;

        Buffer(BufferMode mode, u64 address, u64 size)
            : mode(mode) {
            addressLow = u32(address & hk::bits(32));
            addressMid = u32((address >> 32) & hk::bits(4));
            addressHigh = u32((address >> 36) & hk::bits(22));
            sizeLow = u32(size & hk::bits(32));
            sizeHigh = u32((size >> 32) & hk::bits(4));
        }

        u64 address() const {
            return u64(addressLow) | (u64(addressMid) << 32) | (u64(addressHigh) << 36);
        }

        u64 size() const {
            return u64(sizeLow) | (u64(sizeHigh) << 32);
        }
    };

    struct ReceiveStatic {
        u32 addressLow;
        u32 addressHigh : 16;
        u32 size : 16;

        ReceiveStatic()
            : ReceiveStatic(0, 0) { };
        ReceiveStatic(u64 address, u16 size)
            : size(size) {
            addressLow = u32(address & hk::bits(32));
            addressHigh = u16((address >> 32) & hk::bits(32));
        }

        u64 address() const {
            return u64(addressLow) | (u64(addressHigh) << 32);
        }
    };

} // namespace hk::sf::hipc
