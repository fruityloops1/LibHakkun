
#pragma once

#include "hk/ValueOrResult.h"
#include "hk/services/sm.h"
#include "hk/sf/sf.h"
#include "hk/types.h"
#include "hk/util/FixedCapacityArray.h"
#include "hk/util/Singleton.h"
#include "hk/util/Stream.h"
#include <alloca.h>
#include <cstring>
#include <span>

namespace hk::lm {

    class Logger {
        friend class LogManager;
        sf::Service service;

        Logger(sf::Service&& service)
            : service(std::forward<sf::Service>(service)) { };

        util::FixedCapacityArray<u8, 4> encodeUleb128(u32 value) {
            util::FixedCapacityArray<u8, 4> result;

            do {
                u8 byte = value & 0x7F;
                value >>= 7;

                if (value != 0)
                    byte |= 0x80;

                result.add(byte);
            } while (value != 0);

            return result;
        }

    public:
        void log(const char* text) {
            struct LogPacketHeader {
                u64 processId;
                u64 threadId;
                u8 flags;
                u8 padding;
                u8 severity;
                u8 verbosity;
                u32 payloadSize;
            };

            size len = strlen(text);
            auto sizeBytes = encodeUleb128(len);
            auto logData = cast<u8*>(alloca(sizeof(LogPacketHeader) + 1 + sizeBytes.size() + len));
            util::Stream stream(logData);
            stream.write(LogPacketHeader {
                .processId = 0,
                .threadId = 0,
                .flags = 0,
                .padding = 0,
                .severity = 4, // "fatal"
                .verbosity = 0,
                .payloadSize = u32(1 + sizeBytes.size() + len),
            });
            // chunk key text log
            stream.write(u8(2));
            stream.writeIterator<u8>(sizeBytes);
            stream.writeIterator<char>(std::span<const char>(text, len));

            auto request = sf::Request(0);
            request.addInAutoselect(logData, stream.tell());
            HK_UNWRAP(service.invokeRequest(std::move(request), [](sf::Response& response) {
                return 0;
            }));
        }
    };

    class LogManager : public sf::Service {
        HK_SINGLETON(LogManager);

    public:
        LogManager(sf::Service&& service)
            : sf::Service(std::move(service)) { }

        static LogManager* connect() {
            createInstance(HK_UNWRAP(sm::ServiceManager::instance()->getServiceHandle<"lm">()));
            return instance();
        }

        ValueOrResult<Logger> getLogger() {
            u64 pidReserved = 0;
            auto request = sf::Request(0, &pidReserved, sizeof(pidReserved));
            request.setSendPid();
            return invokeRequest(std::move(request), [this](sf::Response& response) {
                sf::Service subservice = response.nextSubservice(this);
                return Logger(std::move(subservice));
            });
        }
    };

} // namespace hk::lm
