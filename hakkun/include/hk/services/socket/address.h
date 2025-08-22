#pragma once

#include "hk/Result.h"
#include "hk/ValueOrResult.h"
#include "hk/diag/diag.h"
#include "hk/services/socket/result.h"
#include "hk/types.h"
#include "hk/util/TemplateString.h"
#include <bit>
namespace hk::socket {
    enum class AddressFamily : u8
    {
        Ipv4 = 2,
        Ipv6 = 28,
    };
    enum class Type : u8
    {
        Stream = 1,
        Datagram = 2,
        Raw = 3,
        SequencedPacketStream = 5,
    };
    enum class Protocol : u8
    {
        Tcp = 2,
        Udp = 17,
    };

    class SocketAddr {
        u8 mLength;
        AddressFamily mFamily;

        friend class SocketAddrIpv4;
        SocketAddr(u8 length, AddressFamily family)
            : mLength(length)
            , mFamily(family) { }
    };
    class SocketAddrIpv4 : public SocketAddr {
        constexpr SocketAddrIpv4(u16 port, u32 address)
            : SocketAddr(6, AddressFamily::Ipv4)
            , mPort(std::byteswap(port))
            , mAddress(address) { }

    public:
        u16 mPort;
        u32 mAddress;
        u64 _ = {};
        constexpr static ValueOrResult<u32> parseAddress(const char* text) {
            size len = __builtin_strlen(text);
            HK_UNLESS(len <= 15, ResultTooLong());

            constexpr const auto parseOctet = [](const char*& text, bool lastOctet) -> ValueOrResult<u8> {
                if (!text)
                    HK_ABORT("null text", 0);

                u16 value = 0;
                for (u8 i = 0; i < 4; i++) {
                    char c = *text++;
                    if (c == '.' && i == 0)
                        return ResultDelimiterBeforeDigit();
                    if (c == '.' || (c == '\0' && lastOctet && i != 0))
                        return value;
                    if (c == '\0')
                        return ResultEarlyEndOfString();
                    if (c < '0' || c > '9')
                        return ResultInvalidCharacter();
                    value *= 10;
                    value += c - '0';
                    if (value > 0xFF)
                        return ResultOctetTooLarge();
                }
                return ResultOctetTooWide();
            };

            u32 value2 = 0;
            u8 octets[4] = {};
            for (u8 i = 0; i < 4; i++) {
                auto result = parseOctet(text, i == 3);
                HK_TRY(result);
                octets[i] = result.value();
                value2 <<= 8;
                value2 |= octets[i];
            }

            return std::byteswap(value2);
        }
        template <util::TemplateString address>
        constexpr static SocketAddrIpv4 parse(u16 port) {
            static_assert(__builtin_strlen(address.value) <= 15, "IPv4 addresses are no longer than 15 characters");
            constexpr ValueOrResult<u32> result = parseAddress(address.value);
            static_assert(Result(result).getDescription() == 0);

            return SocketAddrIpv4(
                port,
                result.value());
        }

        static ValueOrResult<SocketAddrIpv4> parse(const char* address, u16 port) {
            return parseAddress(address).map([port](u32 addressValue) {
                return SocketAddrIpv4(
                    port,
                    addressValue);
            });
        }
    };
}
