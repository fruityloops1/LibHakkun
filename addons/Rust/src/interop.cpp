#include "hk/diag/diag.h"

extern "C" void rust_log(const u8* data, size len) {
  hk::diag::logBuffer(cast<const char*>(data), len);
}

extern "C" void rust_panic() {
  HK_ABORT("rust panicked", 0);
}
