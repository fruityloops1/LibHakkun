#include "hk/diag/ipclogger.h"
#include "hk/ValueOrResult.h"
#include "hk/diag/results.h"
#include "hk/ro/RoUtil.h"
#include "hk/services/sm.h"
#include "hk/sf/sf.h"

namespace hk::diag::ipclogger {
    IpcLogger IpcLogger::sInstance = {};

    Result IpcLogger::initialize() {
        if (!sInstance.isDisconnected())
            return ResultSuccess();
        bool failed = true;

        auto symbol = hk::ro::lookupSymbol("_ZN2nn2sf4hipc20ConnectToHipcServiceEPNS_3svc6HandleEPKc");
        Handle sessionHandle;
        if (symbol) {
            auto func = cast<hk::Result (*)(svc::Handle*, const char*)>(symbol);
            HK_TRY(func(&sessionHandle, "hk:log"));
        } else {
            if (!hk::sm::ServiceManager::instance())
                return MAKE_RESULT(ResultMissingServiceManager());

            sessionHandle = HK_TRY(hk::sm::ServiceManager::instance()->getServiceHandle<"hk:log">().map([](sf::Service service) {
                return service.toHandle();
            }));
        }

        failed = false;
        sInstance.mSession.store(sessionHandle, std::memory_order_release);

        return ResultSuccess();
    }

    void IpcLogger::logImpl(Span<const u8> buffer, u16 tag) {
        if (isDisconnected())
            return;

        util::Stream stream(svc::getTLS()->ipcMessageBuffer, sf::cTlsBufferSize);
        stream.write(sf::hipc::Header {
            .tag = tag,
            .sendBufferCount = 1,
            .dataWords = 0,
        });
        stream.write(sf::hipc::Buffer(sf::hipc::BufferMode::Normal, u64(buffer.data()), buffer.size()));
        auto res = svc::SendSyncRequest(mSession);
        if (res.failed())
            svc::Break(svc::BreakReason_Assert, (void*)0x20001, res.getValue());
    }

} // namespace hk::diag::ipclogger
