
/* Note: this is meant to be included in every file that enables debugging,
   since thr_debug_msg is declared with static linkage */

#include <stdarg.h>

#ifndef __GNUC__
#define CURRENT_FUNCTION ((const char*)0)
#else
#define CURRENT_FUNCTION __PRETTY_FUNCTION__
#endif

#if DO_DEBUG_LIBPDTHREAD == 1
inline static void thr_debug_msg(const char* func_name, const char* format, ...) {
    va_list ap;
    va_start(ap, format);

    static const char* preamble_format = "[tid: %d; func: %s] ";
    unsigned len = 2048 + strlen(preamble_format) + (func_name ? strlen(func_name) : 0) + strlen(format);
    char* real_format = 
        new char[len];
    unsigned ct = snprintf(real_format, 256, preamble_format, thr_self(), func_name);
    strncat(real_format, format, len - ct - 1);
    vfprintf(stderr, real_format, ap);
    delete [] real_format;
}
#define DEBUGGING_CURRENT_MODULE
#else
#if DO_DEBUG_LIBPDTHREAD == 0
inline static void thr_debug_msg(const char* func_name, const char* format, ...) { }
#else
#error Do not include thr_debug.h without checking to ensure that debugging is enabled
#endif
#endif
