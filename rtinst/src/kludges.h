/*
 * Copyright (c) 1993, 1994 Barton P. Miller, Jeff Hollingsworth,
 *     Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, Karen
 *     Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.
 * 
 * This software is furnished under the condition that it may not be
 * provided or otherwise made available to, or used by, any other
 * person, except as provided for by the terms of applicable license
 * agreements.  No title to or ownership of the software is hereby
 * transferred.  The name of the principals may not be used in any
 * advertising or publicity related to this software without specific,
 * written prior authorization.  Any use of this software must include
 * the above copyright notice.
 *
 */





/************************************************************************
 * kludges.h: the name says it all. stop compiler warnings.
************************************************************************/





#if !defined(_rtinst_src_kludges_h_)
#define _rtinst_src_kludges_h_





/************************************************************************
 * header files.
************************************************************************/

#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>





/************************************************************************
 * common definitions and declarations.
************************************************************************/

typedef void (*Sa_Handler)();





/************************************************************************
 * architecture-os definitions and declarations.
************************************************************************/

#if defined(i386_unknown_bsd)

#endif /* defined(i386_unknown_bsd) */



#if defined(hppa1_1_hp_hpux)

#endif /* defined(hppa1_1_hp_hpux) */



#if defined(mips_dec_ultrix4_3)

#endif /* defined(mips_dec_ultrix4_3) */



#if defined(sparc_sun_solaris2_4)

#endif /* defined(sparc_sun_solaris2_4) */



#if defined(sparc_sun_sunos4_1_3)

extern int    gettimeofday(struct timeval *, struct timezone *);
extern int    fclose(FILE *);
extern int    fflush(FILE *);
extern int    fprintf(FILE *, const char *, ...);
extern int    printf(const char *, ...);
extern int    setitimer(int, struct itimerval *, struct itimerval *);
extern int    sigpause(int);
/* extern size_t fread(void *, size_t, size_t, FILE *); */
/* extern size_t fwrite(void *, size_t, size_t, FILE *); */

#endif /* defined(sparc_sun_sunos4_1_3) */





/************************************************************************
 * for c-compilers without proper include files.
************************************************************************/

#if !defined(__cplusplus)

extern int  fflush(FILE *);
extern int  fprintf(FILE *, const char *, ...);
extern int  printf(const char *, ...);

extern void perror(const char *);


#endif /* !defined(__cplusplus) */





#endif /* !defined(_rtinst_src_kludges_h_) */
