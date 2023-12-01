#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <mutex>

namespace {
  bool debug_decode = false;
  std::once_flag init_flag{};
  
  void init() {
    std::call_once(init_flag,
      []() noexcept {
        if(getenv("INSTRUCTIONAPI_DEBUG_DECODE")) {
            debug_decode = true;
        }
      }
    );
  }
}

namespace Dyninst { namespace InstructionAPI {

  void decode_printf(const char *format, ...) {
    if (NULL == format) return;

    init();
    if (!debug_decode) return;

    va_list va;
    va_start(va, format);
    vfprintf(stderr, format, va);
    va_end(va);
  }

}}
