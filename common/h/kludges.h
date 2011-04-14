/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef KLUDGES_H
#define KLUDGES_H

/*
 * Kludges to handle broken system includes and such...
 *
 * $Log: kludges.h,v $
 * Revision 1.2  1995/02/10 22:35:06  jcargill
 * Fixed bzero prototype
 *
 * Revision 1.1  1994/11/01  16:07:28  markc
 * Added Object classes that provide os independent symbol tables.
 * Added stl-like container classes with iterators.
 *
 * Revision 1.2  1994/09/22  02:09:49  markc
 * Added sparc specific ifdefs
 *
 * Revision 1.1  1994/07/14  17:41:56  jcargill
 * Added kludges file to clean up compilation warnings.
 *
 */

#if defined(sparc_sun_sunos4_1_3)

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/ptrace.h>
#include <sys/socket.h>
#include <vfork.h>
#include <string.h>
#include <sys/wait.h> 
#include <assert.h>
#include <sys/signal.h>
#include <errno.h>

extern int wait3(int *statusp, int options, struct rusage *rusage);
extern int fflush(FILE *stream);
extern int vfork();
extern int getrusage(int, struct rusage*);
extern int munmap(caddr_t, int);
extern char *sys_errlist[];
extern int ptrace(enum ptracereq, int, char*, int, char*);
extern int select (int wid, fd_set *rd, fd_set *wr, fd_set *ex, struct timeval *tm);
extern int socketpair(int d, int type, int protocol, int sv[2]);
// extern char *strcat(char *s1, char *s2);
// extern char *strncat(char *s1, char *s2, int n);
// extern char *strdup(char *s1);
// extern int strcmp(char *s1, char *s2);
// extern int strncmp(char *s1, char *s2, int n);
extern int strcasecmp(char *s1, char *s2);
extern int strncasecmp(char *s1, char *s2, int n);
// extern char *strcpy(char *s1, char *s2);
// extern char *strncpy(char *s1, char *s2, int n);
// extern int strlen(char *s);
// extern char *strchr(char *s, char c);
// extern char *strrchr(char *s, char c);
FILE *fdopen(int, const char*);
int gethostname(char*, int);
int gettimeofday(struct timeval *tp, struct timezone *tzp);

#if defined(__cplusplus)
};
#endif /* defined(__cplusplus) */

#endif /* sparc_sun_sunos4_1_3 */


#ifdef sparc_tmc_cmost7_3

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/ptrace.h>
#include <sys/socket.h>
#include <vfork.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/signal.h>
#include <assert.h>

extern int wait3(int *statusp, int options, struct rusage *rusage);
extern int fflush(FILE *stream);
extern int vfork();
extern int getrusage(int, struct rusage*);
extern int munmap(caddr_t, int);
extern char *sys_errlist[];
extern int ptrace(enum ptracereq, int, char*, int, char*);
extern int select (int wid, fd_set *rd, fd_set *wr, fd_set *ex, struct timeval *tm);
extern int socketpair(int d, int type, int protocol, int sv[2]);
extern char *strcat(char *s1, char *s2);
extern char *strncat(char *s1, char *s2, int n);
extern char *strdup(char *s1);
// extern int strcmp(char *s1, char *s2);
extern int strncmp(const char *s1, const char *s2, int n);
extern int strcasecmp(char *s1, char *s2);
extern int strncasecmp(char *s1, char *s2, int n);
// extern char *strcpy(char *s1, char *s2);
// extern char *strncpy(char *s1, char *s2, int n);
// extern int strlen(char *s);
extern char *strchr(char *s, char c);
extern char *strrchr(char *s, char c);
extern int gettimeofday(struct timeval *tp, struct timezone *tzp);
extern char *memset(char *, int, int);
extern void bzero(void *, long unsigned int);
int gethostname(char*, int);

#if defined(__cplusplus)
};
#endif /* defined(__cplusplus) */

#endif /* sparc_tmc_cmost7_3 */

#if defined(sparc_sun_solaris2_3)
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/socket.h>
#include <errno.h>
// TODO - get this out of here
extern "C" {
extern int gettimeofday( struct timeval *tp, struct timezone *tzp);
extern int gethostname(char*, int);
}
extern char *sys_errlist[];
#endif  /* defined(sparc_sun_solaris2_3) */


#endif /* KLUDGES_H */
