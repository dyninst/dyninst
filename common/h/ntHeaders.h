/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: ntHeaders.h,v 1.9 2001/12/06 01:36:20 bernat Exp $

#if !defined(pd_nt_headers_h)
#define pd_nt_headers_h

#include <assert.h>
#include <stdio.h>

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <direct.h>
#include <strstrea.h>
#include <malloc.h>


#ifdef BPATCH_LIBRARY

#include <wtypes.h>
typedef void *caddr_t; 

#else

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef mips_unknown_ce2_11 //ccw 9 apr 2001
#include <rpc/rpc.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BPATCH_LIBRARY */

#ifdef mips_unknown_ce2_11 //ccw 29 mar 2001
#include <windows.h>
#endif

#include <winnt.h>
#include <winsock.h>
#include <imagehlp.h>

#include <process.h>
#include <sys/types.h>

#include <signal.h>
#include <stdarg.h>
#include <time.h>
#include <sys/stat.h>
#include <float.h>

#define isnan _isnan
#ifndef alloca
#define alloca _alloca
#endif

/* compatiblity typedefs */
typedef int pid_t;
typedef int key_t;

// Apparently this doesn't exist on NT? Weird.
int P_getopt(int argc, char *argv[], char *optstring);

/* POSIX */
inline void P_abort (void) { abort();}
inline int P_close (int FILEDES) { return (close(FILEDES));}
inline int P__dup2(int OLD, int NEW) { return (_dup2(OLD, NEW)); }
inline int P__execvp(const char *fname, const char *const argv[]) {
  return (_execvp(fname, argv)); }
inline void P__exit (int STATUS) { _exit(STATUS);}
inline FILE * P_fdopen (int FILEDES, const char *OPENTYPE) {
  return (fdopen(FILEDES, OPENTYPE));}
inline FILE * P_fopen (const char *FILENAME, const char *OPENTYPE) {
  return (fopen(FILENAME, OPENTYPE));}
inline int P_fstat (int FILEDES, struct stat *BUF) { return (fstat(FILEDES, BUF));}
inline int P_getpid () { return (getpid());}
inline off_t P_lseek (int FILEDES, off_t OFFSET, int WHENCE) {
  return (lseek(FILEDES, OFFSET, WHENCE));}
inline int P__open(const char *FILENAME, int FLAGS, int MODE) {
  return (open(FILENAME, FLAGS, MODE));}
inline int P__pclose (FILE *STREAM) { return (_pclose(STREAM));}
inline FILE *P__popen (const char *COMMAND, const char *MODE) {
  return (_popen(COMMAND, MODE));}
inline size_t P_read (int FILEDES, void *BUFFER, size_t SIZE) {
  return (read(FILEDES, BUFFER, SIZE));}
inline size_t P_write (int FILEDES, const void *BUFFER, size_t SIZE) {
  return (write(FILEDES, BUFFER, SIZE));}
inline int P_chdir(const char *path) { return (chdir(path)); }

/* ANSI */
inline void P_exit (int STATUS) { exit(STATUS);}
inline int P_fflush(FILE *stream) { return (fflush(stream));}
inline char * P_fgets (char *S, int COUNT, FILE *STREAM) {
  return (fgets(S, COUNT, STREAM));}
inline void * P_malloc (size_t SIZE) { return (malloc(SIZE));}
inline void * P_memcpy (void *A1, const void *A2, size_t SIZE) { return memcpy(A1, A2, SIZE); }
inline void * P_memset (void *BLOCK, int C, size_t SIZE) {
  return (memset(BLOCK, C, SIZE));}
inline void P_perror (const char *MESSAGE) { perror(MESSAGE);}
typedef void (*P_sig_handler)(int);
inline P_sig_handler P_signal (int SIGNUM, P_sig_handler ACTION) {
  return (signal(SIGNUM, ACTION));}
inline char * P_strcat (char *TO, const char *FROM) {
  return (strcat(TO, FROM));}

inline const char * P_strchr (const char *STRING, int C) {return (strchr(STRING, C));}
inline char * P_strchr (char *STRING, int C) {return (strchr(STRING, C));}

inline int P_strcmp (const char *S1, const char *S2) {
  return (strcmp(S1, S2));}
inline char * P_strcpy (char *TO, const char *FROM) {
  return (strcpy(TO, FROM));}
inline char *P_strdup(const char *S) { return (strdup(S));}
inline size_t P_strlen (const char *S) { return (strlen(S));}
inline char * P_strncat (char *TO, const char *FROM, size_t SIZE) {
  return (strncat(TO, FROM, SIZE)); }
inline int P_strncmp (const char *S1, const char *S2, size_t SIZE) {
  return (strncmp(S1, S2, SIZE));}
inline char * P_strncpy (char *TO, const char *FROM, size_t SIZE) {
  return (strncpy(TO, FROM, SIZE));}

inline const char * P_strrchr (const char *STRING, int C) {return (strrchr(STRING, C));}
inline char * P_strrchr (char *STRING, int C) {return (strrchr(STRING, C));}

inline const char * P_strstr (const char *HAYSTACK, const char *NEEDLE) {return (strstr(HAYSTACK, NEEDLE));}
inline char * P_strstr (char *HAYSTACK, const char *NEEDLE) {return (strstr(HAYSTACK, NEEDLE));}

inline double P_strtod (const char *STRING, char **TAILPTR) {
  return (strtod(STRING, TAILPTR));}
inline char * P_strtok (char *NEWSTRING, const char *DELIMITERS) {
  return (strtok(NEWSTRING, DELIMITERS));}
inline long int P_strtol (const char *STRING, char **TAILPTR, int BASE) {
  return (strtol(STRING, TAILPTR, BASE));}
inline unsigned long int P_strtoul(const char *STRING, char **TAILPTR, int BASE) { 
  return (strtoul(STRING, TAILPTR, BASE));}

/* BSD */
inline int P_accept (int SOCK, struct sockaddr *ADDR, size_t *LENGTH_PTR) {
  return (accept(SOCK, ADDR, (int*) LENGTH_PTR));}
inline int P_bind(int socket, struct sockaddr *addr, size_t len) {
  return (bind(socket, addr, len));}
inline int P_connect(int socket, struct sockaddr *addr, size_t len) {
  return (connect(socket, addr, len));}
inline struct hostent * P_gethostbyname (const char *NAME) {
  return (gethostbyname(NAME));}
inline struct servent * P_getservbyname (const char *NAME, const char *PROTO) {
  return (getservbyname(NAME, PROTO));}
inline int P_getsockname (int SOCKET, struct sockaddr *ADDR, size_t *LENGTH_PTR) {
  return (getsockname(SOCKET, ADDR, (int*) LENGTH_PTR));}
inline int P_listen (int socket, unsigned int n) { return (listen(socket, n));}
inline int P_socket (int NAMESPACE, int STYLE, int PROTOCOL) {
  return (socket(NAMESPACE, STYLE, PROTOCOL));}

inline int P_select(int wid, fd_set *rd, fd_set *wr, fd_set *ex,
		    struct timeval *tm) {
  return (select(wid, rd, wr, ex, tm));}

#ifndef BPATCH_LIBRARY
typedef int (*P_xdrproc_t)(XDR*, ...);
//extern const char *sys_errlist[];

inline void   P_xdr_destroy(XDR *x) { xdr_destroy(x);}
inline bool_t P_xdr_u_char(XDR *x, u_char *uc) { return (xdr_u_char(x, uc));}
inline bool_t P_xdr_int(XDR *x, int *i) { return (xdr_int(x, i));}
inline bool_t P_xdr_double(XDR *x, double *d) {
  return (xdr_double(x, d));}
inline bool_t P_xdr_u_int(XDR *x, u_int *u){
  return (xdr_u_int(x, u));}
inline bool_t P_xdr_float(XDR *x, float *f) {
  return (xdr_float(x, f));}
inline bool_t P_xdr_char(XDR *x, char *c) {
  return (xdr_char(x, c));}
inline bool_t P_xdr_string(XDR *x, char **h, const u_int maxsize) {
  return (xdr_string(x, h, maxsize));}

inline void P_xdrrec_create(XDR *x, const u_int send_sz, const u_int rec_sz,
			    const caddr_t handle, 
			    xdr_rd_func read_r, xdr_wr_func write_f) {
  xdrrec_create(x, send_sz, rec_sz, handle, read_r, write_f);}
inline bool_t P_xdrrec_endofrecord(XDR *x, int now) { 
  return (xdrrec_endofrecord(x, now));}
inline bool_t P_xdrrec_skiprecord(XDR *x) { return (xdrrec_skiprecord(x));}
inline bool_t P_xdrrec_eof(XDR *x) { return (xdrrec_eof(x)); }
#endif

#endif
