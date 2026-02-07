#include "hk/types.h"

namespace hk::socket {
    enum PollEvents : unsigned short {
        Default = 0,
        CanRead = bit(0),
        CanWrite = bit(2),
        Error = bit(3),
        ExceptionalCondition = bit(1),
        HangUp = bit(4),
    };

    struct PollFd {
        s32 fd;
        PollEvents requestedEvents = PollEvents::Default;
        PollEvents returnedEvents = PollEvents::Default;
    };

    static_assert(sizeof(PollFd) == 8);
}
