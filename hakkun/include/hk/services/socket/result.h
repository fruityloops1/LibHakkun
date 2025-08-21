#include "hk/Result.h"
namespace hk::socket {
    HK_RESULT_MODULE(412)
    HK_DEFINE_RESULT_RANGE(Socket, 30, 50)

    HK_DEFINE_RESULT(Errno, 30)
    HK_DEFINE_RESULT(DelimiterBeforeDigit, 31)
    HK_DEFINE_RESULT(InvalidCharacter, 32)
    HK_DEFINE_RESULT(OctetTooWide, 33)
    HK_DEFINE_RESULT(OctetTooLarge, 34)
    HK_DEFINE_RESULT(EarlyEndOfString, 35)
    HK_DEFINE_RESULT(TooLong, 36)
}
