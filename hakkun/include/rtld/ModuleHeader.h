#pragma once

#include <stdint.h>

namespace rtld {

    struct ModuleHeader {
        constexpr static uint32_t MOD0_MAGIC = 0x30444F4D;

        uint32_t magic;
        uint32_t dynamic_offset;
        uint32_t bss_start_offset;
        uint32_t bss_end_offset;
        uint32_t unwind_start_offset;
        uint32_t unwind_end_offset;
        uint32_t module_object_offset;

        bool isValidMagic() const { return magic == MOD0_MAGIC; }
    };

} // namespace rtld
