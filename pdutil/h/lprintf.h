/************************************************************************
 * lprintf.h: interface to printf-like error functions.
************************************************************************/





#if !defined(_lprintf_h_)
#define _lprintf_h_





/************************************************************************
 * function prototypes.
************************************************************************/

extern void log_msg(const char *);
extern void log_printf(void (*)(const char *), const char *, ...);
extern void log_perror(void (*)(const char *), const char *);





#endif /* !defined(_lprintf_h_) */
