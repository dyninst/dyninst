/*
 * $Log: rpcUtil.C,v $
 * Revision 1.35  1994/11/11 06:59:23  markc
 * Added additional argument to RPC_make_arg_list and RPC_undo_arg_list to
 * support remote executition for paradyndPVM.
 * Added additional argument to RPC_make_arg_list and RPC_undo_arg_list to
 * support remote executition for paradyndPVM.
 *
 * Revision 1.34  1994/11/06  09:51:03  newhall
 * added error checking, especially the handshaking when paradyn starts up
 * paradynd.
 *
 * Revision 1.33  1994/09/27  22:28:46  jcargill
 * Moved the rexec prototype inside the extern C part
 *
 * Revision 1.32  1994/09/27  19:23:05  jcargill
 * Warning cleanup: prototyped rexec, pushed consts further down
 *
 * Revision 1.31  1994/09/22  03:19:52  markc
 * Changes to remove compiler warnings
 *
 * Revision 1.30  1994/09/20  18:23:58  hollings
 * added option to use rexec as well as fork and rsh to start processes.
 *
 * Revision 1.29  1994/08/18  19:55:04  markc
 * Added ifdef for solaris.
 *
 * Revision 1.28  1994/08/17  18:25:25  markc
 * Added RPCgetArg
 * Change RPC_make_arg_list to avoid leaving a hole at the head of the arg list
 * Changed RPCProcessCreate to use the new version of arg list
 * Changed the execl to execlp
 *
 * Revision 1.27  1994/07/28  22:22:04  krisna
 * changed definitions of ReadFunc and WriteFunc to conform to prototypes
 *
 * Revision 1.26  1994/07/19  18:30:27  markc
 * Made machineName default to zero as last parameter to RPC_make_arg_list.
 * Added typecast to malloc call in RPC_make_arg_list.
 *
 * Added typecast to malloc call in RPC_make_arg_list.
 *
 * Revision 1.25  1994/07/18  19:08:25  hollings
 * added extra arg to RPC_make_arg_list.
 *
 * Revision 1.24  1994/06/22  00:37:13  markc
 * Fixed code to remove warnings.
 *
 * Revision 1.23  1994/06/02  23:36:58  markc
 * Added support for igen error checking.
 *
 * Revision 1.22  1994/05/17  00:14:45  hollings
 * added rcs log entry.
 *
 * Revision 1.21  1994/05/16  04:27:47  hollings
 * Added inlcude of vfork.h on SUNS to prevent problem with optimizer.
 *
 * Revision 1.20  1994/05/12  18:47:51  jcargill
 * Changed make args function to leave room for program name in arg_list[0],
 * and added code to RPCprocessCreate to poke it in there before execv'ing.
 *
 * Revision 1.19  1994/04/21  23:23:49  hollings
 * removed paradynd name from make args function.
 *
 * Revision 1.18  1994/04/06  22:46:12  markc
 * Fixed bug in XDRrpc constructor that clobbered the fd value.  Added feature
 * to RPC_readReady to do blocking select.
 *
 * Revision 1.17  1994/04/01  20:05:27  hollings
 * Removed kill of rsh process (not needed and it causes a race condition).
 *
 * Revision 1.16  1994/04/01  04:59:13  markc
 * Put in support to encode NULL ptrs to strings in xdr_String.
 *
 * Revision 1.15  1994/03/31  22:59:08  hollings
 * added well known port as a paramter to xdrRPC constructor.
 *
 * Revision 1.14  1994/03/31  22:45:04  markc
 * Added Log for rcs.
 *
*/

//
// This file defines a set of utility routines for RPC services.
//
//

// overcome malloc redefinition due to /usr/include/rpc/types.h declaring 
// malloc 

#include <signal.h>
#include <sys/wait.h>

/* prevents malloc from being redefined */
#ifdef MIPS
#define MALLOC_DEFINED_AS_VOID
#endif

#include "util/h/rpcUtil.h"
#include "util/h/tunableConst.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <memory.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/file.h>

extern "C" {
#include <rpc/types.h>
#include <rpc/xdr.h>
}

// functions that g++-fixincludes missed
#ifdef MIPS
extern "C" {
void bzero (char*, int);
int select (int, fd_set*, fd_set*, fd_set*, struct timeval*);
char *strdup (char*);
int gethostname(char*, int);
int socket(int, int, int);
int bind(int s, struct sockaddr *, int);
int getsockname(int, struct sockaddr*, int *);
int listen(int, int);
int connect(int s, struct sockaddr*, int);
int socketpair(int, int, int, int sv[2]);
int vfork();
int accept(int, struct sockaddr *addr, int *); 
}
#elif SPARC
#include <sys/socket.h>
#include <vfork.h>
extern "C" {
void bzero (char*, int);
int select (int, fd_set*, fd_set*, fd_set*, struct timeval*);
int socket(int, int, int);
int gethostname(char*, int);
int bind(int s, struct sockaddr *, int);
int getsockname(int, struct sockaddr*, int *);
int listen(int, int);
int connect(int s, struct sockaddr*, int);
int socketpair(int, int, int, int sv[2]);
int accept(int, struct sockaddr *addr, int *); 
int rexec(char **ahost, u_short inport, const char *user, const char *passwd, 
	  const char *cmd, int *fd2p);
}
#elif SOLARIS
#endif


const char *RSH_COMMAND="rsh";

tunableBooleanConstant useRexec(FALSE, NULL, userConstant, "useRexec",
    "Use rsedc instead of rsh to establish connection to daemon");

int RPCdefaultXDRRead(const void* handle, char *buf, u_int len)
{
    int fd = (int) handle;
    int ret;

    do {
	ret = read(fd, buf, len);
    } while (ret < 0 && errno == EINTR);

    if (ret <= 0) return(-1);
    return (ret);
}

int RPCdefaultXDRWrite(const void* handle, char *buf, u_int len)
{
    int fd = (int) handle;
    int ret;

    do {
	ret = write(fd, buf, len);
    } while (ret < 0 && errno == EINTR);

    if (ret != len) 
	return(-1);
    else
	return (ret);
}

XDRrpc::~XDRrpc()
{
  if (fd >= 0)
    {
      fcntl (fd, F_SETFL, FNDELAY);
      close(fd);
    }
  if (__xdrs__) 
    {
      xdr_destroy (__xdrs__);
      delete (__xdrs__);
    }
}

//
// prepare for RPC's to be done/received on the passed fd.
//
XDRrpc::XDRrpc(int f, xdrIOFunc readRoutine, xdrIOFunc writeRoutine, int nblock)
{
    fd = f;
    __xdrs__ = new XDR;
    if (!readRoutine) readRoutine = (xdrIOFunc) RPCdefaultXDRRead;
    if (!writeRoutine) writeRoutine = (xdrIOFunc) RPCdefaultXDRWrite;
    (void) xdrrec_create(__xdrs__, 0, 0, (char *) fd, readRoutine,writeRoutine);
    if (nblock)
      fcntl (fd, F_SETFL, FNDELAY);
}

//
// prepare for RPC's to be done/received on the passed fd.
//
XDRrpc::XDRrpc(char *machine,
	       char *user,
	       char *program,
	       xdrIOFunc readRoutine, 
	       xdrIOFunc writeRoutine,
	       char **arg_list,
	       int nblock,
	       int wellKnownPortFd)
{
    fd = RPCprocessCreate(pid, machine, user, program, arg_list, 
	wellKnownPortFd);
    if (fd >= 0) {
        __xdrs__ = new XDR;
	if (!readRoutine) readRoutine = (xdrIOFunc) RPCdefaultXDRRead;
	if (!writeRoutine) writeRoutine = (xdrIOFunc) RPCdefaultXDRWrite;
	(void) xdrrec_create(__xdrs__, 0, 0, (char *) fd, 
		readRoutine, writeRoutine);
	if (nblock)
	  fcntl (fd, F_SETFL, FNDELAY);
    } else {
	__xdrs__ = NULL;
	fd = -1;
    }
}

int
RPC_readReady (int fd, int timeout)
{
  fd_set readfds;
  struct timeval tvptr, *the_tv;

  tvptr.tv_sec = timeout; tvptr.tv_usec = 0;
  if (fd < 0) return -1;
  FD_ZERO(&readfds);
  FD_SET (fd, &readfds);

  // -1 timeout = blocking select
  if (timeout == -1)
     the_tv = 0;
  else
     the_tv = &tvptr;
 
  if (select (fd+1, &readfds, NULL, NULL, the_tv) == -1)
    {
      // if (errno == EBADF)
	return -1;
    }
  return (FD_ISSET (fd, &readfds));
}

// TODO
// mdc - I need to clean this up
int 
RPC_undo_arg_list (int argc, char **arg_list, char **machine, int &family,
		   int &type, int &well_known_socket, int &flag, int &firstPVM)
{
  int loop;
  char *ptr;
  int sum = 0;

  for (loop=0; loop < argc; ++loop)
    {
      if (!strncmp(arg_list[loop], "-p", 2))
	{
	  well_known_socket = (int) strtol (arg_list[loop] + 2, &ptr, 10);
	  if (ptr == (arg_list[loop] + 2))
	    return(-1);
	  sum |= 1;
	}
      else if (!strncmp(arg_list[loop], "-f", 2))
	{
	  family = (int) strtol (arg_list[loop] + 2, &ptr, 10);
	  if (ptr == (arg_list[loop] + 2))
	    return(-1);
          sum |= 2;
	}
      else if (!strncmp(arg_list[loop], "-v", 2))
	{
	  firstPVM = (int) strtol (arg_list[loop] + 2, &ptr, 10);
	  if (ptr == (arg_list[loop] + 2))
	    return(-1);
          sum |= 4;
	}
      else if (!strncmp(arg_list[loop], "-t", 2))
	{
	  type = (int) strtol (arg_list[loop] + 2, &ptr, 10);
	  if (ptr == (arg_list[loop] + 2))
	    return(-1);
          sum |= 8;
	}
      else if (!strncmp(arg_list[loop], "-m", 2))
	{
	  *machine = strdup (arg_list[loop] + 2);
	  if (!(*machine)) return -1;
	  sum |= 16;
	}
      else if (!strncmp(arg_list[loop], "-l", 2))
	{
	  flag = (int) strtol (arg_list[loop] + 2, &ptr, 10);
	  if (ptr == (arg_list[loop] + 2))
	    return(-1);
          sum |= 32;
	}
    }
  if (sum == (32 + 16 + 8 + 4 + 2 + 1))
	return 0;
  else
	return -1;
}

/*
 * Build an argument list starting at position 0.
 * Note, this arg list will be used in an exec system call
 * AND, the command name will have to be inserted at the head of the list
 * But, a NULL space will NOT be left at the head of the list
 */
char **RPC_make_arg_list(int family, int type, int well_known_socket,
			 int flag, int firstPVM, char *machine_name)
{
  char arg_str[100];
  int arg_count = 0;
  char **arg_list;

  arg_list = new char*[9];
  sprintf(arg_str, "%s%d", "-p", well_known_socket);  
  arg_list[arg_count++] = strdup (arg_str);  // 0
  sprintf(arg_str, "%s%d", "-f", family);
  arg_list[arg_count++] = strdup (arg_str);  // 1
  sprintf(arg_str, "%s%d", "-t", type);
  arg_list[arg_count++] = strdup (arg_str);  // 2
  if (!machine_name) {
      machine_name = (char *) malloc(50);
      gethostname (machine_name, 49);
  }
  sprintf(arg_str, "%s%s", "-m", machine_name);
  arg_list[arg_count++] = strdup (arg_str); // 3
  sprintf(arg_str, "%s%d", "-l", flag);
  arg_list[arg_count++] = strdup (arg_str);  // 4
  sprintf(arg_str, "%s%d", "-v", firstPVM);
  arg_list[arg_count++] = strdup (arg_str);  // 5 
  arg_list[arg_count++] = 0;                 // 6
  return arg_list;
}

// returns fd of socket that is listened on, or -1
int
RPC_setup_socket (int *sfd,   // return file descriptor
		  int family, // AF_INET ...
		  int type)   // SOCK_STREAM ...
{
  struct sockaddr_in serv_addr;
  int length;
  char machine[50];

  if (gethostname(machine, 49) != 0)
    return -1;

  if ((*sfd = socket(family, type, 0)) < 0)
    return -1;

  memset ((char*) &serv_addr, 0, sizeof(serv_addr));
  /* bzero ((char *) &serv_addr, sizeof(servaddr)); */
  serv_addr.sin_family = (short) family;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(0);
  
  length = sizeof(serv_addr);

  if (bind(*sfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    return -1;

  if (getsockname (*sfd, (struct sockaddr *) &serv_addr, &length) < 0)
    return -1;

  if (listen(*sfd, 5) < 0)
    return -1;

  return (ntohs (serv_addr.sin_port));
}

//
// connect to well known socket
//
XDRrpc::XDRrpc(int family,            
	       int req_port,             
	       int type,
	       char *machine, 
	       xdrIOFunc readRoutine,
	       xdrIOFunc writeRoutine,
	       int nblock)
     // socket, connect using machine
{
  struct sockaddr_in serv_addr;
  struct hostent *hostptr = 0;
  struct in_addr *inadr = 0;

  __xdrs__ = 0;

  if ( (hostptr = gethostbyname(machine)) == 0)
    { fd = -1; return; }

  inadr = (struct in_addr *) hostptr->h_addr_list[0];
  memset ((char*) &serv_addr, 0, sizeof(serv_addr));
  /* bzero ((char *) &serv_addr, sizeof(serv_addr)); */
  serv_addr.sin_family = family;
  serv_addr.sin_addr = *inadr;
  serv_addr.sin_port = htons(req_port);

  if ( (fd = socket(family, type, 0)) < 0)
    { fd = -1; return; }

  if (connect(fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    { fd = -1; return; }

    __xdrs__ = new XDR;
    if (!readRoutine) readRoutine = (xdrIOFunc) RPCdefaultXDRRead;
    if (!writeRoutine) writeRoutine = (xdrIOFunc) RPCdefaultXDRWrite;
    (void) xdrrec_create(__xdrs__, 0, 0, (char *) fd, readRoutine,writeRoutine);
    if (nblock)
      fcntl (fd, F_SETFL, FNDELAY);

}

//
// our version of string encoding that does malloc as needed.
//
bool_t xdr_char_PTR(XDR *xdrs, char **str)
{
    int len;
    unsigned char isNull=0;

	// if XDR_FREE, str's memory is freed
    switch (xdrs->x_op) {
	case XDR_ENCODE:
		if (*str) {
		    len = strlen(*str)+1;
                    if (!xdr_u_char(xdrs, &isNull))
			return FALSE;
                } else {
		    isNull = (unsigned char) 1;
                    if (!xdr_u_char(xdrs, &isNull))
			return FALSE;
                    else
			return TRUE;
                }
                return (xdr_string (xdrs, str, 65536));
	case XDR_DECODE:
		*str = NULL;
		if (!xdr_u_char(xdrs, &isNull))
		    return FALSE;
                if (isNull)
		    return TRUE;
                else
                    return (xdr_string (xdrs, str, 65536));
	case XDR_FREE:
		// xdr_free (xdr_string, str);
		if (*str)
		  free (*str);
		*str = NULL;
		return (TRUE);
		// return(TRUE);
		// free the memory
	default:
                return (FALSE);
		// this should never occur	
    }
}

//
// directly exec the command (local).
//
int execCmd(int &pid, const char *command, char **arg_list, int portFd)
{
    int ret;
    int sv[2];
    char **new_al;
    int al_len, i;
    int execlERROR;

    ret = socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
    if (ret) return(ret);
    execlERROR = 0;
    pid = vfork();
    if (pid == 0) {
	close(sv[0]);
	dup2(sv[1], 0);
	if (!arg_list)
	  execlp(command, command);
	else {
	  // how long is the arg_list?
	  for (al_len=0; arg_list[al_len]; al_len++) 
	    ; // not a typo

	  new_al = new char*[al_len+2];
	  new_al[0] = strdup (command);
	  new_al[al_len+1] = 0;
	  for (i=0; i<al_len; ++i) {
	    new_al[i+1] = arg_list[i];
	  }
	  execvp(command, new_al);
	  delete(new_al[0]);
	  delete(new_al);
	}
	execlERROR = errno;
	_exit(-1);
    } else if (pid > 0 && !execlERROR) {
	close(sv[1]);
	return(sv[0]);
    } else {
	return(-1);
    }
    return(-1);
}

int handleRemoteConnect(int &pid, int fd, int portFd)
{
    char *ret;
    int retFd;
    FILE *pfp;
    char line[256];

    pfp = fdopen(fd, "r");
    if (pfp == NULL) {
       cerr << "handleRemoteConnect: fdopen of fd " << fd << " failed." << endl;
       return -1;
    }

    do {
	ret = fgets(line, sizeof(line)-1, pfp);
	if (ret && !strncmp(line, "PARADYND", strlen("PARADYND"))) {
	    // got the good stuff
	    sscanf(line, "PARADYND %d", &pid);

	    retFd = RPC_getConnect(portFd);
	    return(retFd);
	} else if (ret) {
	    printf("%s", line);
	    return (-1);
	}
    }  while (ret);

    return(-1);
}

//
// use rsh to get a remote process started.
//
int rshCommand(int &pid, const char *hostName, const char *userName, 
	       const char *command, char **arg_list, int portFd)
{
    int total;
    int fd[2];
    char **curr;
    int shellPid;
    char *paradyndCommand;
    int ret;

    total = strlen(command) + 2;
    for (curr = arg_list; *curr; curr++) {
	total += strlen(*curr) + 2;
    }
    paradyndCommand = (char *) malloc(total+2);

    sprintf(paradyndCommand, "%s ", command);
    for (curr = arg_list; *curr; curr++) {
	strcat(paradyndCommand, *curr);
	strcat(paradyndCommand, " ");
    }

    // need to rsh to machine and setup io path.

    if (pipe(fd)) {
	perror("pipe");
	return (-1);
    }

    shellPid = vfork();
    if (shellPid == 0) {
	/* child */
	assert(-1 != dup2(fd[1], 1)); /* copy it onto stdout */
	assert(-1 != close(fd[0]));
	assert(-1 != close(fd[1]));
	if (userName) {
	    ret = execlp(RSH_COMMAND, RSH_COMMAND, hostName, "-l", 
			 userName, "-n", paradyndCommand, "-l0", NULL);
            fprintf(stderr,"rshCommand: execlp failed (ret = %d)\n",ret);
	} else {
	    ret = execlp(RSH_COMMAND, RSH_COMMAND, hostName, "-n", 
			 paradyndCommand, "-l0", NULL);
            fprintf(stderr,"rshCommand: execlp failed (ret = %d)\n",ret);
	}
	_exit(-1);
    } else if (shellPid > 0) {
	close(fd[1]);
    } else {
	// error situation
    }
    return(handleRemoteConnect(pid, fd[0], portFd));
}

int rexecCommand(int &pid, const char *hostName, const char *userName, 
		 const char *command, char **arg_list, int portFd)
{
    int fd;
    int total;
    char **curr;
    char *paradyndCommand;
    struct servent *inport;

    total = strlen(command) + 2;
    for (curr = arg_list; *curr; curr++) {
	total += strlen(*curr) + 2;
    }
    paradyndCommand = (char *) malloc(total+2);

    sprintf(paradyndCommand, "%s ", command);
    for (curr = arg_list; *curr; curr++) {
	strcat(paradyndCommand, *curr);
	strcat(paradyndCommand, " ");
    }

    inport = getservbyname("exec",  "tcp");

    fd = rexec(&hostName, inport->s_port, userName,NULL, paradyndCommand, NULL);
    if (fd < 0) {
	perror("rexec");
	printf("rexec failed\n");
    }
    return(handleRemoteConnect(pid, fd, portFd));
}

/*
 * 
 * "command" will be appended to the front of the arg list
 *
 * if arg_list == NULL, command is the only argument used
 */
int RPCprocessCreate(int &pid, const char *hostName, const char *userName,
		     const char *command, char **arg_list, int portFd)
{
    int ret;
    char local[50];

    if (gethostname(local, 49))
	strcpy (local, " ");

    if (!hostName || 
	!strcmp(hostName, "") || 
	!strcmp(hostName, "localhost") ||
	!strcmp(hostName, local)) {
      ret = execCmd(pid, command, arg_list, portFd);
    } else if (useRexec.getValue()) {
      ret = rexecCommand(pid, hostName, userName, command, arg_list, portFd);
    } else {
      ret = rshCommand(pid, hostName, userName, command, arg_list, portFd);
    }
    return(ret);
  }

int RPC_getConnect(int fd)
{
  int clilen;
  struct in_addr cli_addr;
  int new_fd;

  if (fd == -1)
    return -1;

  clilen = sizeof(cli_addr);

  if ((new_fd = accept (fd, (struct sockaddr *) &cli_addr, &clilen)) < 0)
    return -1;
  else
    return new_fd;
}

/*
 *  RPCgetArg - break a string into blank separated words
 *  Used to parse a command line.
 *  
 *  input --> a null terminated string, the command line
 *  argc --> returns the number of args found
 *  returns --> words (separated by blanks on command line)
 *  Note --> this allocates memory 
 */
char **RPCgetArg(int &argc, const char *input)
{
#define BLANK ' '

  char **temp, **result;
  const char *word_start;
  int word_count = 0, temp_max, index, length, word_len;

  argc = 0;
  if (!input) 
    return ((char **) 0);

  length = strlen(input);
  index = 0;

  /* advance past blanks in input */
  while ((index < length) && (input[index] == BLANK))
    index++;

  /* input is all blanks, or NULL */
  if (index >= length) {
    result = new char*[1];
    if (!result) return ((char**) 0);
    result[0] = 0;
    return result;
  }

  temp = new char*[30];
  if (!temp) return temp;
  temp_max = 29;

  do {
    /* the start of each string */
    word_len = 0; word_start = input + index;

    /* find the next BLANK and the WORD size */
    while ((index < length) && (input[index] != BLANK)) {
      index++; word_len++;
    }

    /* copy the word */
    temp[word_count] = new char[word_len+1];
    strncpy(temp[word_count], word_start, word_len);
    temp[word_count][word_len] = (char) 0;
    word_count++;

    /* skip past consecutive blanks */
    while ((index < length) && (input[index] == BLANK))
      index++;

    /* no more room in temp, copy it to new_temp */
    if (word_count > temp_max) {
      char **new_temp;
      int nt_len, i;
      
      /* new temp_max size */
      nt_len = (temp_max+1) >> 1;
      temp_max = nt_len - 1;

      /* copy temp to new_temp */
      new_temp = new char*[nt_len];
      if (!new_temp) return ((char**) 0);
      for (i=0; i<word_count; ++i)
	new_temp[i] = temp[i];
      
      delete temp;
      temp = new_temp;
    }
  } while (index < length);

  argc = word_count;
  /* null terminate the word list */
  temp[word_count] = (char*) 0;
  return temp;
}
