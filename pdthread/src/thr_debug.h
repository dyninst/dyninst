#include <stdarg.h>
#include "../h/thread.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef CURRENT_FILE
#error Do not include thr_debug.h without defining CURRENT_FILE
#else
/* This indirection is necessary so that CURRENT_FILE will be expanded. */
#define __pastetoks(x,y) x ## y
#define pastetoks(x,y) __pastetoks(x,y)
#define thr_debug_msg pastetoks(thr_debug_msg_for_, CURRENT_FILE)
#endif

#ifndef __GNUC__
#define CURRENT_FUNCTION ((const char*)0)
#else
#define CURRENT_FUNCTION __PRETTY_FUNCTION__
#endif

#ifndef DO_DEBUG_LIBPDTHREAD
#define DO_DEBUG_LIBPDTHREAD 0
#endif

static void thr_debug_msg(const char* func_name, const char* format, ...);

#if DO_DEBUG_LIBPDTHREAD == 1

static void thr_debug_msg(const char* func_name, const char* format, ...) {
    va_list ap;
    va_start(ap, format);

    if (do_traces_for->get((char*)func_name, strlen(func_name)) != 0) return;

    static const char* preamble_format = "[id: (%d,%d,%d,\"%s\"); func: %s] ";
    const char* name = thr_name(NULL);

    name = name ? name : "no name";
    
    unsigned len = 2048 + strlen(preamble_format) + (func_name ? strlen(func_name) : 0) + strlen(format) + strlen(name);
    char* real_format = 
        new char[len];
    unsigned ct = snprintf(real_format, 256,
                    preamble_format, getpid(), thr_self(),
                    Thread::GetSelfId(),
                    name, func_name);
    strncat(real_format, format, len - ct - 1);
    vfprintf(stderr, real_format, ap);
    delete [] real_format;
}

#define DEBUGGING_CURRENT_MODULE
#else
#if DO_DEBUG_LIBPDTHREAD == 0
inline static void thr_debug_msg(const char* func_name, const char* format, ...) { }
#else
#error Do not include thr_debug.h without checking to ensure that debugging is either enabled or disabled
#endif
#endif

