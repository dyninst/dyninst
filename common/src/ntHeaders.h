/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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


// $Id: ntHeaders.h,v 1.27 2008/05/09 00:25:37 jaw Exp $

#if !defined(pd_nt_headers_h)
#define pd_nt_headers_h

#include <windows.h>
#include <winsock2.h>

#if !defined(__out_ecount_opt)
#define __out_ecount_opt(x) //Working around dbhelp.h bugs
#endif

#include <dbghelp.h>

#include <assert.h>
#include <stdio.h>

#include <stddef.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <direct.h>
#include <malloc.h>

#ifdef BPATCH_LIBRARY

#include <wtypes.h>
typedef void *caddr_t; 

#else
#if defined(PARADYND)
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
#endif /* BPATCH_LIBRARY */


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

#define PDSOCKET_ERROR SOCKET_ERROR
typedef SOCKET PDSOCKET;

/* compatiblity typedefs */
typedef int pid_t;
typedef int key_t;
typedef unsigned int socklen_t;

// Apparently this doesn't exist on NT? Weird.
int P_getopt(int argc, char *argv[], const char *optstring);

/* POSIX */
inline void P_abort (void) { abort();}
inline int P_close (int FILEDES) { return (_close(FILEDES));}
inline int P__dup2(int OLD, int NEW) { return (_dup2(OLD, NEW)); }
inline void P__exit (int STATUS) { _exit(STATUS);}
inline FILE * P_fdopen (int FILEDES, const char *OPENTYPE) {
  return (_fdopen(FILEDES, OPENTYPE));}
inline FILE * P_fopen (const char *FILENAME, const char *OPENTYPE) {
  return fopen(FILENAME, OPENTYPE);
}
inline int P_fstat (int FILEDES, struct stat *BUF) { return (fstat(FILEDES, BUF));}
inline int P_getpid () { return (_getpid());}
inline off_t P_lseek (int FILEDES, off_t OFFSET, int WHENCE) {
  return (_lseek(FILEDES, OFFSET, WHENCE));}
inline int P__open(const char *FILENAME, int FLAGS, int MODE) {
  return (_open(FILENAME, FLAGS, MODE));}
inline int P__pclose (FILE *STREAM) { return (_pclose(STREAM));}
inline FILE *P__popen (const char *COMMAND, const char *MODE) {
  return (_popen(COMMAND, MODE));}
inline size_t P_write (int FILEDES, const void *BUFFER, size_t SIZE) {
  return (_write(FILEDES, BUFFER, SIZE));}
inline int P_chdir(const char *path) { return (_chdir(path)); }
inline int P_putenv(char *str) { return _putenv(str); }

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
inline int P_getpagesize() { 
	SYSTEM_INFO info;
    static int page_size = 0;
    if (page_size)
        return page_size;
    GetSystemInfo(&info);
    page_size = info.dwPageSize;
    return page_size;
}

inline int P_strcmp (const char *S1, const char *S2) {
  return (strcmp(S1, S2));}
inline char * P_strcpy (char *TO, const char *FROM) {
  return (strcpy(TO, FROM));}
inline char *P_strdup(const char *S) { return (_strdup(S));}
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
  return (::bind(socket, addr, len));}
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

inline int P_recv(int s, void *buf, size_t len, int flags) {
   return (recv(s, (char*)buf, len, flags));
}

inline int P_mkdir(const char *pathname, int) {
	return _mkdir(pathname);
}

inline int P_unlink(const char *pathname) {
	return _unlink(pathname);
}
extern char *cplus_demangle(const char *, int, bool);
/* We can't export this, it's inline. */
inline std::string P_cplus_demangle( const std::string &symbol, bool includeTypes = false ) {
   char *demangled = cplus_demangle( symbol.c_str(), 0, includeTypes );
   if (demangled)  {
      std::string s = std::string(demangled);
      free(demangled)
      return s;
   }  else  {
      return symbol;
   }

#ifndef BPATCH_LIBRARY
#if defined(PARADYND)
//extern const char *sys_errlist[];

extern "C" int snprintf(char *, size_t, const char *, ...);
#endif
#endif
#endif
