#pragma once

#include "hk/svc/api.h"
#include "hk/svc/cpu.h"
#include "hk/svc/results.h"
#include "hk/svc/types.h"
#include "hk/types.h"
#include <limits>

namespace hk::os {

    template <bool AutoClear = true>
    class alignas(sizeof(u32)) EventImpl {
        constexpr static u32 cSignaled = 0xFEFEFEFE;
        constexpr static u32 cEmpty = 0xBABABABA;

        u32 mValue = cEmpty;

        NON_COPYABLE(EventImpl);
        NON_MOVABLE(EventImpl);

    public:
        EventImpl() {
        }

        Result clear() {
            if (not svc::storeExclusive(&mValue, cEmpty))
                return svc::SignalToAddress(&mValue, svc::SignalType::Signal, cEmpty, std::numeric_limits<s64>::max());
            return ResultSuccess();
        }

        Result signal() {
            if (not svc::storeExclusive(&mValue, cSignaled))
                return svc::SignalToAddress(&mValue, svc::SignalType::Signal, cSignaled, std::numeric_limits<s64>::max());
            return ResultSuccess();
        }

        Result wait(s64 timeout = std::numeric_limits<s64>::max()) {
            if (svc::loadExclusive(&mValue) == cSignaled) {
                if constexpr (AutoClear)
                    return clear();
                return ResultSuccess();
            }

            HK_TRY(svc::WaitForAddress(&mValue, svc::ArbitrationType::WaitIfEqual, cEmpty, timeout));
            if constexpr (AutoClear)
                return clear();

            return ResultSuccess();
        }

        ~EventImpl() {
            while (not svc::storeExclusive(&mValue, -1))
                ;
        }
    };

    using Event = EventImpl<true>;
    using EventNoAutoClear = EventImpl<false>;

} // namespace hk::os
