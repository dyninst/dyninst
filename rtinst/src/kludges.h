/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
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
#if !defined(i386_unknown_nt4_0)
#include <sys/time.h>
#include <sys/types.h>
#endif




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
