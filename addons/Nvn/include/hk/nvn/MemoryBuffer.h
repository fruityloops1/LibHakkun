#pragma once

#include "hk/diag/diag.h"
#include "hk/types.h"

#include "nvn/nvn_Cpp.h"

namespace hk::nvn {

    using namespace ::nvn;

    class MemoryBuffer {
        nvn::MemoryPool mPool;
        nvn::Buffer mBuffer;
        void* mMemory = nullptr;
        bool mInitialized = false;

    public:
        void initialize(void* buffer, size size, nvn::Device* device, nvn::MemoryPoolFlags flags) {
            HK_ASSERT(!mInitialized);
            HK_ASSERT(alignDownPage(buffer) == buffer);
            size = alignUpPage(size);
            mMemory = buffer;

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

            mInitialized = true;
        }

        bool isInitialized() const { return mInitialized; }

        void finalize() {
            HK_ASSERT(mInitialized);
            mMemory = nullptr;
            mPool.Finalize();
            mBuffer.Finalize();
            mInitialized = false;
        }

        void* getMemory() const { return mMemory; }
        nvn::BufferAddress getAddress() const { return mBuffer.GetAddress(); }
        size getSize() const { return mPool.GetSize(); }
        void* map() const { return mPool.Map(); }
    };

} // namespace hk::nvn
