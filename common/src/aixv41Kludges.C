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

#include "common/h/headers.h"

extern "C" {
//extern int accept(int, struct sockaddr *addr, int *);
//extern int accept(int, struct sockaddr *addr, size_t *);
extern int getrusage(int, struct rusage*);
//extern int getsockname(int, struct sockaddr*, int *);
//extern int getsockname(int, struct sockaddr*, size_t *);

extern int listen(int, int);
extern int rexec(char **ahost, u_short inport, char *user, char *passwd,
		 char *cmd, int *fd2p);
extern int socket(int, int, int);
extern int socketpair(int, int, int, int sv[2]);
extern int pipe(int fd[2]);
extern int vfork();
extern int fork();

/* 
 * Warning this next ifdef may not be correct on all platforms.  We have
 *   noticed that the xdrrec_endofrecord protoype is missing for gcc295.1 on
 *   the UMD AIX 4.2 systems, but seems fine on the Wisconsin gcc295.2 AIX
 *   4.3 systems.  The need for this prototype may be different elsewhere.
 *   jkh (hollings@cs.umd.edu) 3/21/00
 *
 */
#ifndef _AIX43
extern bool_t xdrrec_endofrecord (XDR *xdrs, bool_t sendnow);
#endif

};

/* Non standard (even from sunos4 to sunos5 -- blech */

/* POSIX */
int P_getopt(int argc, char *argv[], const char *optstring) { return getopt(argc, argv, optstring);}
void P_abort() { abort();}
int P_close (int FILEDES) { return (close(FILEDES));}
int P_dup2 (int OLD, int NEW) { return (dup2(OLD, NEW));}
// int P_execlp (const char *FILENAME, const char *ARG0) {
//  return (execlp(FILENAME, ARG0, NULL));}
int P_execvp (const char *FILENAME, char *const ARGV[]) {
  return (execvp(FILENAME, ARGV));}
void P__exit (int STATUS) { _exit(STATUS);}
int P_fcntl (int filedes, int command, int arg2) {
  return (fcntl(filedes, command, arg2));}
FILE * P_fdopen (int FILEDES, const char *OPENTYPE) {
  return (fdopen(FILEDES, OPENTYPE));}
FILE * P_fopen (const char *FILENAME, const char *OPENTYPE) {
  return (fopen(FILENAME, OPENTYPE));}
int P_fstat (int FILEDES, struct stat *BUF) { return (fstat(FILEDES, BUF));}
pid_t P_getpid () { return (getpid());}
int P_kill(pid_t PID, int SIGNUM) { return (kill(PID, SIGNUM));}
off_t P_lseek (int FILEDES, off_t OFFSET, int WHENCE) {
  return (lseek(FILEDES, OFFSET, WHENCE));}
int P_open(const char *FILENAME, int FLAGS, mode_t MODE) {
  return (open(FILENAME, FLAGS, MODE));}
int P_pclose (FILE *STREAM) { return (pclose(STREAM));}
FILE *P_popen (const char *COMMAND, const char *MODE) { return (popen(COMMAND, MODE));}
size_t P_read (int FILEDES, void *BUFFER, size_t SIZE) {
  return (read(FILEDES, BUFFER, SIZE));}
int P_uname(utsname *unm) { return (uname(unm));}
pid_t P_wait(int *status_ptr) { return (wait(status_ptr));}
pid_t P_waitpid(pid_t pid, int *statusp, int options) {
  return (waitpid(pid, statusp, options));}
size_t P_write (int FILEDES, const void *BUFFER, size_t SIZE) {
  return (write(FILEDES, BUFFER, SIZE));}

/* SYSTEM-V shared memory */
int P_shmget(key_t thekey, int size, int flags) { return shmget(thekey, size, flags); }
void *P_shmat(int shmid, void *addr, int flags) { return shmat(shmid, addr, flags); }
int P_shmdt(void *addr) { return shmdt(addr); }
int P_shmctl(int shmid, int cmd, struct shmid_ds *buf) { return shmctl(shmid, cmd, buf); }

/* ANSI */
void P_exit (int STATUS) { exit(STATUS);}
int P_fflush(FILE *stream) {return (fflush(stream));}
char *P_fgets (char *S, int COUNT, FILE *STREAM) { return (fgets(S, COUNT, STREAM));}
void * P_malloc (size_t SIZE) { return (malloc(SIZE));}
void * P_memcpy (void *A1, const void *A2, size_t SIZE) {
  return (memcpy(A1, A2, SIZE));}
void * P_memset (void *BLOCK, int C, unsigned SIZE) {
  return (memset(BLOCK, C, SIZE));}
void P_perror (const char *MESSAGE) { perror(MESSAGE);}
P_sig_handler P_signal (int SIGNUM, P_sig_handler ACTION) {
  return (signal(SIGNUM, ACTION));}
char *P_strcat(char *TO, const char *FROM) { return (strcat(TO, FROM));}
char *P_strchr(const char *S, int C) { return (strchr(S, C));}
int P_strcmp(const char *S1, const char *S2) { return (strcmp(S1, S2));}
char *P_strcpy(char *TO, const char *FROM) { return (strcpy(TO, FROM));}
char *P_strdup(const char *FROM) { return (strdup(FROM));}
size_t P_strlen (const char *S) { return (strlen(S));}
char * P_strncat (char *TO, const char *FROM, size_t SIZE) {
  return (strncat(TO, FROM, SIZE));}
int P_strncmp (const char *S1, const char *S2, size_t SIZE) {
  return (strncmp(S1, S2, SIZE));}
char * P_strncpy (char *TO, const char *FROM, size_t SIZE) {
  return (strncpy(TO, FROM, SIZE));}
char * P_strrchr (const char *STRING, int C) { return (strrchr(STRING, C));}
char * P_strstr (const char *HAYSTACK, const char *NEEDLE) {
  return (strstr(HAYSTACK, NEEDLE));}
double P_strtod (const char *STRING, char **TAILPTR) {
  return(strtod(STRING, TAILPTR));}
char * P_strtok (char *NEWSTRING, const char *DELIMITERS) {
  return (strtok(NEWSTRING, DELIMITERS));}
long int P_strtol (const char *STRING, char **TAILPTR, int BASE){
  return (strtol(STRING, TAILPTR, BASE));}
unsigned long int P_strtoul(const char *STRING, char **TAILPTR, int BASE){
  return (strtoul(STRING, TAILPTR, BASE));}

/* BSD */

int P_accept (int SOCK, struct sockaddr *ADDR, size_t *LENGTH_PTR) {
#if defined(CROSSCOMPILER)
  return (accept(SOCK, ADDR, (int *)LENGTH_PTR));
#else
  return (accept(SOCK, ADDR, (long unsigned int *)LENGTH_PTR));
#endif
}

int P_bind(int socket, struct sockaddr *addr, size_t len) {
  return (bind(socket, addr, len));}

// void P_bzero(void *block, size_t size) { bzero(block, size);}

int P_connect(int socket, struct sockaddr *addr, size_t len) {
  return (connect(socket, addr, len));}

struct hostent *P_gethostbyname (const char *NAME) { 
  return (gethostbyname(NAME));
}

int P_gethostname(char *name, size_t size) {
  return (gethostname(name, size));}

int P_getrusage(int i, struct rusage *r) {return (getrusage(i, r));}

struct servent *P_getservbyname (const char *NAME, const char *PROTO) {
  return (getservbyname(NAME, PROTO));}

int P_getsockname (int SOCKET, struct sockaddr *ADDR, size_t *LENGTH_PTR) {
#if defined(CROSSCOMPILER)
  return (getsockname(SOCKET, ADDR, (int *)LENGTH_PTR));
#else
  return (getsockname(SOCKET, ADDR, (long unsigned int *)LENGTH_PTR));
#endif
}

/* int P_gettimeofday (struct timeval *TP, struct timezone *TZP) {
  return (gettimeofday(TP, TZP));} */

int P_listen(int socket, unsigned int n) {
  return (listen(socket, n));}

caddr_t P_mmap(caddr_t addr, size_t len, int prot, int flags,
	       int fd, off_t off) {
  return ((caddr_t)mmap(addr, len, prot, flags, fd, off));}

int P_munmap(caddr_t ca, int i) {return (munmap(ca, i));}

int P_select (int wid, fd_set *rd, fd_set *wr, fd_set *ex, struct timeval *tm) {
  return (select((unsigned long)wid, rd, wr, ex, tm));}

int P_socket (int NAMESPACE, int STYLE, int PROTOCOL) {
  return (socket(NAMESPACE, STYLE, PROTOCOL));}

int P_socketpair(int NAMESPACE, int style, int protocol, int filedes[2]) {
  return (socketpair(NAMESPACE, style, protocol, filedes));}

int P_pipe(int fd[2]) { return (pipe(fd)); }

int P_strcasecmp(const char *s1, const char *s2) {
  return (strcasecmp((char*)s1, (char*)s2));}

int P_strncasecmp (const char *S1, const char *S2, size_t N) {
  return (strncasecmp((char*)S1, (char*)S2, N));}

int P_rexec(char **ahost, u_short inport, char *user,
	    char *passwd, char *cmd, int *fd2p){
  return (rexec(ahost, inport, user, passwd, cmd, fd2p));}

void P_endservent(void) { endservent(); }

void P_xdr_destroy(XDR *xdrs) { xdr_destroy(xdrs); }
bool_t P_xdr_u_char(XDR *x, u_char *u) { return (xdr_u_char(x, u));}
bool_t P_xdr_int(XDR *x, int *i) { return (xdr_int(x, i));}
bool_t P_xdr_double(XDR *x, double *d) { return (xdr_double(x, d));}
bool_t P_xdr_u_int(XDR *x, u_int *u) { return (xdr_u_int(x, u));}
bool_t P_xdr_float(XDR *x, float *f) { return (xdr_float(x, f));}
bool_t P_xdr_char(XDR *x, char *c) { return (xdr_char(x, c));}
bool_t P_xdr_string(XDR *x, char **c, const u_int maxsize) {
  return (xdr_string(x, c, maxsize));}
bool_t P_xdrrec_endofrecord(XDR *x, int now) {return (xdrrec_endofrecord(x, now));}
bool_t P_xdrrec_skiprecord(XDR*x) { return (xdrrec_skiprecord(x));}
void P_xdrrec_create(XDR *x, const u_int send_sz, const u_int rec_sz,
		     const caddr_t handle, 
		     xdr_rd_func readit, xdr_wr_func writeit) {
  xdrrec_create(x, send_sz, rec_sz, handle, 
		(int(*)(void *, char *, int))readit, 
		(int(*)(void *, char *, int))writeit);}






