
#include <stdarg.h>

#ifndef __GNUC__
#define CURRENT_FUNCTION ((const char*)0)
#else
#define CURRENT_FUNCTION __PRETTY_FUNCTION__
#endif

#ifndef DO_DEBUG_LIBPDTHREAD
#define DO_DEBUG_LIBPDTHREAD 0
#endif

void thr_debug_msg(const char* func_name, const char* format, ...);
void enable_tracing_for(const char* func_name);
void disable_tracing_for(const char* func_name);

