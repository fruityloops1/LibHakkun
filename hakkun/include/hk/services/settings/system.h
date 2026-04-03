#pragma once

#include "hk/Result.h"
#include "hk/ValueOrResult.h"
#include "hk/diag/diag.h"
#include "hk/services/settings/result.h"
#include "hk/services/settings/util.h"
#include "hk/services/sm.h"
#include "hk/sf/sf.h"
#include "hk/sf/utils.h"
#include "hk/types.h"
#include "hk/util/Array.h"
#include "hk/util/FixedBuffer.h"
#include "hk/util/FixedVec.h"
#include "hk/util/Singleton.h"
#include "hk/util/StringView.h"

namespace hk::settings {

    class SystemSettings : public sf::Service {
        HK_SINGLETON(SystemSettings)
    public:
        SystemSettings(sf::Service&& service)
            : sf::Service(forward<sf::Service>(service)) { }

        static ValueOrResult<SystemSettings*> initialize() {
            createInstance(HK_TRY(sm::ServiceManager::instance()->getServiceHandle<"set:sys">()));
            return instance();
        }

        template <size Capacity>
        ValueOrResult<util::FixedBuffer<u8, Capacity>> getSettingsItemValue(util::StringView key_namespace, util::StringView key) {
            ASSERT_KEY_LEN(key_namespace)
            ASSERT_KEY_LEN(key)

            util::Array<u8, Capacity> data;
            auto request = sf::Request(this, 38);
            request.addInPointer<char>(key_namespace);
            request.addInPointer<char>(key);
            request.addOutMapAlias<u8>(data);
            u64 finalSize = HK_TRY(invokeRequest(move(request), sf::inlineDataExtractor<u64>()));
            diag::logLine("actual size: %d vs %d  %d", finalSize, Capacity, data[0]);
            HK_UNLESS(finalSize <= Capacity, ResultValueTooLarge());

            return util::FixedBuffer<u8, Capacity>(data, finalSize);
        }

        template <typename T, bool RejectIfWrongSize = true>
        ValueOrResult<T> getSettingsItemValue(util::StringView key_namespace, util::StringView key) {
            util::FixedVec<u8, sizeof(T)> data = HK_TRY(getSettingsItemValue<sizeof(T)>(key_namespace, key));

            if (RejectIfWrongSize)
                HK_UNLESS(data.size() == sizeof(T), ResultValueIsWrongSize());

            return *(T*)data.begin();
        }
    };

} // namespace hk::settings
