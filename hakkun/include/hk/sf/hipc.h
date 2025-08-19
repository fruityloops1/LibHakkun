#pragma once

#include "hk/types.h"

namespace hk::sf::hipc {

    struct Header {
        u16 tag;
        u8 sendStaticCount : 4;
        u8 sendBufferCount : 4;
        u8 recvBufferCount : 4;
        u8 exchBufferCount : 4;
        u16 dataWords : 10;
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
        u8 index : 6;
        u8 addressHigh : 6;
        u8 addressMid : 4;
        u16 size;
        u32 addressLow;

        Static(u8 index, u64 address, u16 size)
            : index(index)
            , size(size) {
            addressLow = u32(address & hk::bits(32));
            addressMid = u8((address >> 32) & hk::bits(4));
            addressHigh = u8((address >> 36) & hk::bits(6));
        }

        u64 address() const {
            return u64(addressLow) | (u64(addressMid) << 32) | (u64(addressHigh) << 36);
        }
    };

    enum class BufferMode : u8
    {
        Normal = 0,
        NonSecure = 1,
        NonDevice = 3,
    };

    struct Buffer {
        u32 sizeLow;
        u32 addressLow;
        BufferMode mode : 2;
        u32 addressHigh : 22;
        u8 sizeHigh : 4;
        u8 addressMid : 4;

        Buffer(BufferMode mode, u64 address, u64 size)
            : mode(mode) {
            addressLow = u32(address & hk::bits(32));
            addressMid = u8((address >> 32) & hk::bits(4));
            addressHigh = u8((address >> 36) & hk::bits(22));
            sizeLow = u32(address & hk::bits(32));
            sizeHigh = u8((address >> 32) & hk::bits(4));
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
        u16 addressHigh;
        u16 size;

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

}
