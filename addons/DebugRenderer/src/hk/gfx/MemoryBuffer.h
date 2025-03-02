#pragma once

#include "hk/diag/diag.h"
#include "hk/types.h"
#include "nvn/nvn_Cpp.h"

namespace hk::gfx {

    class MemoryBuffer {
        nvn::MemoryPool mPool;
        nvn::Buffer mBuffer;

    public:
        void initialize(void* buffer, size size, nvn::Device* device, nvn::MemoryPoolFlags flags) {
            HK_ASSERT(alignDownPage(buffer) == buffer);
            size = alignUpPage(size);

            {
                nvn::MemoryPoolBuilder builder;
                builder.SetDefaults()
                    .SetDevice(device)
                    .SetFlags(flags)
                    .SetStorage(buffer, size);

                HK_ASSERT(mPool.Initialize(&builder));
            }

            {
                nvn::BufferBuilder builder;
                builder.SetDevice(device)
                    .SetDefaults()
                    .SetStorage(&mPool, 0x0, size);
                HK_ASSERT(mBuffer.Initialize(&builder));
            }
        }

        nvn::BufferAddress getAddress() const { return mBuffer.GetAddress(); }
        void* map() const { return mPool.Map(); }
    };

} // namespace hk::gfx
