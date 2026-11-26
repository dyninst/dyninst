#include "plt_noret.h"

void abort(void) __attribute__((noreturn)); /* left undefined: bound to libc at run time */

void fatal_inner(void) { abort(); }
