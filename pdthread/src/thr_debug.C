#include "thr_debug.h"
#include "hashtbl.C"
#include <string.h>

#if DO_DEBUG_LIBPDTHREAD == 1
static hashtbl<char, unsigned> do_traces_for;

void thr_debug_msg(const char* func_name, const char* format, ...) {
    va_list ap;
    va_start(ap, format);

    if (do_traces_for.get((char*)func_name, strlen(func_name)) == 0) return;

    static const char* preamble_format = "[id: (%d,%d,%d,\"%s\"); func: %s] ";
    const char* name = thr_name(NULL);

    name = name ? name : "no name";
    
    unsigned len = 2048 + strlen(preamble_format) + (func_name ? strlen(func_name) : 0) + strlen(format) + strlen(name);
    char* real_format = 
        new char[len];
    unsigned ct = snprintf(real_format, 256, preamble_format, getpid(), thr_self(), pthread_self(), name, func_name);
    strncat(real_format, format, len - ct - 1);
    vfprintf(stderr, real_format, ap);
    delete [] real_format;
}

void enable_tracing_for(const char* func) {
    char* func_name = new char[strlen(func)];
    strcpy(func_name, func);
    do_traces_for.put(func_name, 1, strlen(func));
    delete [] func_name;
}

void disable_tracing_for(const char* func) {
    char* func_name = new char[strlen(func)];
    strcpy(func_name, func);
    do_traces_for.put(func_name, 0, strlen(func));
    delete [] func_name;
}

#define DEBUGGING_CURRENT_MODULE
#else
#if DO_DEBUG_LIBPDTHREAD == 0
void thr_debug_msg(const char* func_name, const char* format, ...) { }
void enable_tracing_for(const char* func) { }
void disable_tracing_for(const char* func) { }
#else
#error Do not compile thr_debug.C without checking to ensure that debugging is either enabled or disabled
#endif
#endif

