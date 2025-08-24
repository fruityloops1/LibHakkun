#pragma once

#include "hk/types.h"

namespace hk::socket {

    struct ServiceConfig {
        u32 version = 0xA;

        u32 tcpTxBufSize = 0x8000;
        u32 tcpRxBufSize = 0x10000;
        u32 tcpTxBufMaxSize = 0x30000;
        u32 tcpRxBufMaxSize = 0x30000;

        u32 udpTxBufSize = 0x2400;
        u32 udpRxBufSize = 0xa500;

        u32 sbEfficiency = 4;

        constexpr size_t calcTransferMemorySize() const {
            u32 tcp_tx_buf_max_size = tcpTxBufMaxSize != 0 ? tcpTxBufMaxSize : tcpTxBufSize;
            u32 tcp_rx_buf_max_size = tcpRxBufMaxSize != 0 ? tcpRxBufMaxSize : tcpRxBufSize;
            u32 sum = tcp_tx_buf_max_size + tcp_rx_buf_max_size + udpTxBufSize + udpRxBufSize;

            sum = alignUpPage(sum);
            return size(sbEfficiency * sum);
        }
    };

} // namespace hk::socket
