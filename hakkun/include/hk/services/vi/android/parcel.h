#pragma once

#include "hk/types.h"
#include "hk/util/Stream.h"
#include <array>
#include <span>
#include <string_view>
#include <type_traits>

namespace hk::vi::detail {

    struct ParcelHeader {
        u32 payloadOffset = sizeof(ParcelHeader);
        u32 payloadSize = 0;
        u32 objectsOffset = sizeof(ParcelHeader);
        u32 objectsSize = 0;

        template<typename T = u8>
        std::span<const T> payload() const {
            return std::span(cast<const T*>(this) + payloadOffset, payloadSize / sizeof(T));
        }

        template<typename T = u8>
        std::span<const T> objects() const {
            return std::span(cast<const T*>(this) + objectsOffset, objectsSize / sizeof(T));
        }
    };

    struct OutParcel {
        std::array<u8, 0x400> data;

        util::Stream<u8> reader() {
            return util::Stream<u8>(data.data(), data.size());
        }
    };

    inline void writeString(util::Stream<u8>& stream, const char* text) {
        std::string_view view(text);
        stream.write(u32(view.length()));
        for (char c : view)
            stream.write(u16(c));
        stream.align(4);
    }

    inline void writeInterfaceToken(util::Stream<u8>& stream, const char* text, u32 size) {
        stream.write(u32(0x100));
        writeString(stream, text);
    }

    inline void writeFlattenedObject(util::Stream<u8>& stream, std::span<const u8> data) {
        stream.write(u32(data.size()));
        stream.write(u32(0));
        stream.writeIterator<const u8>(data);
    }

    template <size size>
    class InParcel {
        ParcelHeader header;
        std::array<u8, size> data;

    public:
        template <typename P, typename O>
        void write(P&& payloadWriter, O&& objectWriter) {
            util::Stream<u8> stream(data.data(), data.size());
            auto& streamRef = stream;
            payloadWriter(streamRef);
            header.payloadSize = stream.tell();
            header.objectsOffset = sizeof(ParcelHeader) + stream.tell();
            objectWriter(streamRef);
            header.objectsSize = stream.tell() - header.payloadSize;
        }

        template <typename F>
        void write(F&& payloadWriter) {
            write(payloadWriter, [](auto&) { });
        }
    };

} // namespace hk::vi::detail
