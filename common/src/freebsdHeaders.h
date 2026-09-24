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


#if !defined(_freebsd_headers_h)
#define _freebsd_headers_h

#include <assert.h>
#include <stddef.h>
#include <string>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/ptrace.h>
#include <sys/resource.h>
#include <stdarg.h>
#include <time.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/syscall.h>


#define PDSOCKET_ERROR (-1)
typedef int PDSOCKET;
/* Not going to use on Linux Platform - already declared in /usr/include/errno.h
extern const char *sys_errlist[];
*/

/* POSIX */
inline pid_t P_getpid () { return (getpid());}

inline int P_getpagesize() { return getpagesize(); }

/* ANSI */
inline void * P_memcpy (void *A1, const void *A2, size_t SIZE)
    { return memcpy( A1, A2, SIZE ); }
inline int P_strcmp (const char *S1, const char *S2) {
  return (strcmp(S1, S2));}
inline char *P_strdup(const char *S) { return (strdup(S));}

/* BSD */

inline int P_listen (int socket, unsigned int n) { return (listen(socket, n));}
inline caddr_t P_mmap(caddr_t addr, size_t len, int prot, int flags,
		      int fd, off_t off) {
  return (caddr_t)(mmap(addr, len, prot, flags, fd, off));}
inline int P_munmap(caddr_t addr, int i) { return (munmap(addr, i));}
inline int P_socket (int NAMESPACE, int STYLE, int PROTOCOL) {
  return (socket(NAMESPACE, STYLE, PROTOCOL));}
inline int P_socketpair(int namesp, int style, int protocol, int filedes[2]) {
  return (socketpair(namesp, style, protocol, filedes));}
inline int P_pipe(int fds[2]) { return (pipe(fds)); }
inline int P_strcasecmp(const char *s1, const char *s2) {
  return (strcasecmp(s1, s2));}
inline int P_strncasecmp (const char *S1, const char *S2, size_t N) {
  return (strncasecmp(S1, S2,N));}
/* As endservent() on Linux Platform /usr/include/netdb.h has no return value, 
   so P_endservent() should be a void function */
inline void P_endservent(void) { endservent(); }

inline ssize_t P_recv(int s, void *buf, int len, int flags) {
   return (recv(s, buf, len, flags));
}

inline long int P_ptrace(int req, pid_t pid, Address addr, Address data, int = -1) {
    return ((long int)ptrace(req, pid, (caddr_t)addr, (int)data));
}

inline int P_select(int wid, fd_set *rd, fd_set *wr, fd_set *ex,
		    struct timeval *tm) {
  return (select(wid, rd, wr, ex, tm));}

extern std::string DYNINST_EXPORT P_cplus_demangle( const std::string &symbol,
				bool includeTypes = false );

inline int P_mkdir(const char *pathname, mode_t mode) {
	return mkdir(pathname, mode);
}
inline int P_unlink(const char *pathname) {
	return unlink(pathname);
}
#endif
