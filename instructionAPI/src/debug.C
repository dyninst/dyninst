#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

namespace Dyninst { namespace InstructionAPI {

  static bool debug_decode = false;

  void init() {
    static bool initialized = false;
    if (initialized) return;
    initialized = true;
    if(getenv("INSTRUCTIONAPI_DEBUG_DECODE")) {
        debug_decode = true;
    }
  }

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
