/************************************************************************
 * lprintf.c: printf-like error functions.
************************************************************************/





/************************************************************************
 * header files.
************************************************************************/

#include <errno.h>
#include <stdio.h>
#include <stdarg.h>

#include "lprintf.h"





/************************************************************************
 * void log_msg(const char* msg)
 * void log_printf(void (*pfunc)(const char *), const char* fmt, ...)
 * void log_perror(void (*pfunc)(const char *), const char* msg)
 *
 * error printing functions.
************************************************************************/

static char log_buffer[8192];

void log_msg(const char* msg) {
    fprintf(stderr, "%s", msg);
}

void
log_printf(void (*pfunc)(const char *), const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsprintf(log_buffer, fmt, ap);
    va_end(ap);
    pfunc(log_buffer);
}

void
log_perror(void (*pfunc)(const char *), const char* msg) {
    extern char* sys_errlist[];
    sprintf(log_buffer, "%s: %s\n", msg, sys_errlist[errno]);
    pfunc(log_buffer);
}
