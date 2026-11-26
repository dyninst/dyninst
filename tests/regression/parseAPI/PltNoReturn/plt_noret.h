/*
 * Functions of libplt_noret.so.  Each is in its own file, so the compiler cannot inline one into
 * another, and all are exported with default visibility, so calls between them go through PLT stubs
 * whose jump slots bind, by default, to the definitions in the same library.
 */
#ifndef PLT_NORET_H
#define PLT_NORET_H

void fatal_inner(void); /* never returns: calls abort */
void fatal(void);       /* never returns: calls fatal_inner */
int ok(void);           /* returns */
int caller(void);       /* calls ok, then fatal */
void tail(void);        /* calls fatal, normally as a tail call */

extern int plt_noret_sink;

#endif
