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

#if !defined(_solaris_headers_h)
#define _solaris_headers_h

#include <assert.h>
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
#include <sys/resource.h>
#include <stdarg.h>
#include <time.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <sys/un.h>

#include <rpc/types.h>
#include <rpc/xdr.h>

#include <strstream.h>

typedef int (*P_xdrproc_t)(XDR*, ...);
extern const char *sys_errlist[];

extern int (*P_native_demangle)(const char *, char *, size_t);

extern "C" int rexec(char **, unsigned short, const char *,
		     const char *, const char *, int *);

/* POSIX */
inline int P_getopt(int argc, char *argv[], const char *optstring) { return getopt(argc, argv, optstring);}
inline void P_abort (void) { abort();}
inline int P_close (int FILEDES) { return (close(FILEDES));}
inline int P_dup2 (int OLD, int NEW) { return (dup2(OLD, NEW));}
inline int P_execvp (const char *FILENAME, char *const ARGV[]) {
  return (execvp(FILENAME, ARGV));}
inline void P__exit (int STATUS) { _exit(STATUS);}
inline int P_fcntl (int FILEDES, int COMMAND, int ARG2) {
  return (fcntl(FILEDES, COMMAND, ARG2));}
inline FILE * P_fdopen (int FILEDES, const char *OPENTYPE) {
  return (fdopen(FILEDES, OPENTYPE));}
inline FILE * P_fopen (const char *FILENAME, const char *OPENTYPE) {
  return (fopen(FILENAME, OPENTYPE));}
inline int P_fstat (int FILEDES, struct stat *BUF) { return (fstat(FILEDES, BUF));}
inline pid_t P_getpid () { return (getpid());}
inline int P_kill(pid_t PID, int SIGNUM) { return (kill(PID, SIGNUM));}
inline off_t P_lseek (int FILEDES, off_t OFFSET, int WHENCE) {
  return (lseek(FILEDES, OFFSET, WHENCE));}
inline int P_open(const char *FILENAME, int FLAGS, mode_t MODE) {
  return (open(FILENAME, FLAGS, MODE));}
inline int P_pclose (FILE *STREAM) { return (pclose(STREAM));}
inline FILE *P_popen (const char *COMMAND, const char *MODE) {
  return (popen(COMMAND, MODE));}
inline size_t P_read (int FILEDES, void *BUFFER, size_t SIZE) {
  return (read(FILEDES, BUFFER, SIZE));}
inline int P_uname(struct utsname *un) { return (uname(un));}
inline pid_t P_wait(int *status_ptr) { return (wait(status_ptr));}
inline int P_waitpid(pid_t pid, int *statusp, int options) {
  return (waitpid(pid, statusp, options));}
inline size_t P_write (int FILEDES, const void *BUFFER, size_t SIZE) {
  return (write(FILEDES, BUFFER, SIZE));}
inline int P_chdir(const char *path) { return (chdir(path)); }
inline int P_putenv(char *str) { return putenv(str); }

/* SYSTEM-V shared memory */
#include <sys/ipc.h>
#include <sys/shm.h> /* shmid_ds */
inline int P_shmget(key_t theKey, int size, int flags) {
   return shmget(theKey, size, flags);
}
inline void *P_shmat(int shmid, void *addr, int flags) {
   return shmat(shmid, (char *)addr, flags);
}
inline int P_shmdt(void *addr) {return shmdt((char*)addr);}
inline int P_shmctl(int shmid, int cmd, struct shmid_ds *buf) {
   return shmctl(shmid, cmd, buf);
}

/* ANSI */
inline void P_exit (int STATUS) { exit(STATUS);}
inline int P_fflush(FILE *stream) { return (fflush(stream));}
inline char * P_fgets (char *S, int COUNT, FILE *STREAM) {
  return (fgets(S, COUNT, STREAM));}
inline void * P_malloc (size_t SIZE) { return (malloc(SIZE));}
extern void * P_memcpy (void *A1, const void *A2, size_t SIZE);
inline void * P_memset (void *BLOCK, int C, size_t SIZE) {
  return (memset(BLOCK, C, SIZE));}
inline void P_perror (const char *MESSAGE) { perror(MESSAGE);}
extern "C" {
typedef void (*P_sig_handler)(int);
}
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
  return (accept(SOCK, ADDR, LENGTH_PTR));}
inline int P_bind(int socket, struct sockaddr *addr, size_t len) {
  return (bind(socket, addr, len));}
inline int P_connect(int socket, struct sockaddr *addr, size_t len) {
  return (connect(socket, addr, len));}
inline struct hostent * P_gethostbyname (const char *NAME) {
  return (gethostbyname(NAME));}
/* inline int P_gethostname(char *name, size_t size) {
   return (gethostname(name, size));} */
/* inline int P_getrusage(int i, struct rusage *ru) { 
   return (getrusage(i, ru));} */
inline struct servent * P_getservbyname (const char *NAME, const char *PROTO) {
  return (getservbyname(NAME, PROTO));}
inline int P_getsockname (int SOCKET, struct sockaddr *ADDR, size_t *LENGTH_PTR) {
  return (getsockname(SOCKET, ADDR, LENGTH_PTR));}
inline int P_getsockopt(int s, int level, int optname, void *optval, 
			unsigned int *optlen) {
   return getsockopt(s, level, optname, optval, 
		     static_cast<socklen_t*>(optlen));
}
inline int P_setsockopt(int s, int level, int optname, void *optval, int optlen) {
   return setsockopt(s, level, optname, (const char*)optval, optlen);
}

/* inline int P_gettimeofday (struct timeval *TP, struct timezone *TZP) {
  return (gettimeofday(TP, TZP));} */
inline int P_listen (int socket, unsigned int n) { return (listen(socket, n));}
inline caddr_t P_mmap(caddr_t addr, size_t len, int prot, int flags,
		      int fd, off_t off) {
  return (static_cast<caddr_t>(mmap(addr, len, prot, flags, fd, off)));}
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
inline void P_endservent(void) { endservent(); }

/* Ugly */

inline int P_ptrace(int req, pid_t pid, int addr, int data) {
  return (ptrace(req, pid, addr, data));}

inline int P_select(int wid, fd_set *rd, fd_set *wr, fd_set *ex,
		    struct timeval *tm) {
  return (select(wid, rd, wr, ex, tm));}

inline int P_rexec(char **ahost, u_short inport, char *user,
		   char *passwd, char *cmd, int *fd2p) {
  return (rexec(ahost, inport, user, passwd, cmd, fd2p));}

#if defined(__GNUC__)
extern "C" char *cplus_demangle(char *, int);
#else
#include <demangle.h>
#endif

/* symbol: is the mangled name
   prototype:  the unmangled name is saved in this buffer
   size: specifies the size of the buffer, prototype
   native: target was compiled using Sun compiler
   return 0 for success and non-zero for failure
*/
inline int P_cplus_demangle(const char *symbol, char *prototype, size_t size, 
			    bool nativeCompiler) {
  char *demangled_sym;
#if defined(__GNUC__)
  if (!nativeCompiler || P_native_demangle == NULL) {
   demangled_sym = cplus_demangle(const_cast<char*>(symbol), 0);
   if(demangled_sym==NULL || strlen(demangled_sym) >= size)
     return 1;
   else {
     strcpy(prototype, demangled_sym);
     free(demangled_sym);
     return 0;
   }
  }
#endif

  // Since the Sun demangler gives prototypes, we need to strip that away
  demangled_sym = (char *) malloc(size * sizeof(char));
#if defined(__GNUC__)
  if ((*P_native_demangle)(symbol, demangled_sym, size))
#else
  if (cplus_demangle(symbol, demangled_sym, size))
#endif
  {
    free(demangled_sym);
    return 1;
  }

  char *sym_begin, *ptr;
  if (demangled_sym[0] == '(' &&
      strstr(demangled_sym, "::") != NULL) {
    // Local variable
    sym_begin = strrchr(demangled_sym, ')') + 3;
    if ((ptr = strrchr(sym_begin, ' ')) != NULL)
      *ptr = '\0';
  } else if ((ptr = strchr(demangled_sym, '(')) != NULL) { 
    // Function prototype
    *ptr = '\0';
    if ((ptr = strrchr(demangled_sym, '*')) == NULL &&
	(ptr = strrchr(demangled_sym, ' ')) == NULL)
      // Correctly demangled
      sym_begin = demangled_sym;
    else
      sym_begin = ptr+1;
  } else
    // Correctly demangled
    sym_begin = demangled_sym;

  if (strlen(sym_begin) >= size) {
    free(demangled_sym);
    return 1;
  }
  strcpy(prototype, sym_begin);
  free(demangled_sym);
  return 0;
}

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
#if defined(sparc_sun_solaris2_4) && !defined(__GNUC__)
  xdrrec_create(x, send_sz, rec_sz, handle,
        read_r, write_f);
#else
  xdrrec_create(x, send_sz, rec_sz, handle, read_r, write_f);
#endif
}
inline bool_t P_xdrrec_endofrecord(XDR *x, int now) { 
  return (xdrrec_endofrecord(x, now));}
inline bool_t P_xdrrec_skiprecord(XDR *x) { return (xdrrec_skiprecord(x));}
inline bool_t P_xdrrec_eof(XDR *x) { return (xdrrec_eof(x)); }

#endif
