/*
$Log: rpcUtil.C,v $
Revision 1.15  1994/03/31 22:59:08  hollings
added well known port as a paramter to xdrRPC constructor.

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
#include "util/h/rpcUtil.h"

#define RSH_COMMAND	"rsh"

int RPCdefaultXDRRead(int handle, char *buf, u_int len)
{
    int ret;

    do {
	ret = read(handle, buf, len);
    } while (ret < 0 && errno == EINTR);

    if (ret <= 0) return(-1);
    return (ret);
}

int RPCdefaultXDRWrite(int handle, char *buf, u_int len)
{
    int ret;

    do {
	ret = write(handle, buf, len);
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
    if (!readRoutine) readRoutine = RPCdefaultXDRRead;
    if (!writeRoutine) writeRoutine = RPCdefaultXDRWrite;
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
    fd = RPCprocessCreate(&pid, machine, user, program, arg_list, 
	wellKnownPortFd);
    if (fd >= 0) {
        __xdrs__ = new XDR;
	if (!readRoutine) readRoutine = RPCdefaultXDRRead;
	if (!writeRoutine) writeRoutine = RPCdefaultXDRWrite;
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
  struct timeval tvptr;

  tvptr.tv_sec = timeout; tvptr.tv_usec = 0;
  if (fd < 0) return -1;
  FD_ZERO(&readfds);
  FD_SET (fd, &readfds);
  if (select (fd+1, &readfds, NULL, NULL, &tvptr) == -1)
    {
      // if (errno == EBADF)
	return -1;
    }
  return (FD_ISSET (fd, &readfds));
}

int 
RPC_undo_arg_list (int argc, char **arg_list, char **machine, int &family,
		   int &type, int &well_known_socket, int &flag)
{
  int loop;
  char *ptr;
  int sum = 0;

  for (loop=0; loop < argc; ++loop)
    {
      if (!strncmp(arg_list[loop], "-p", 2))
	{
	  well_known_socket = (int) strtol (arg_list[loop] + 2, &ptr, 10);
	  if (!ptr)
	    return(-1);
	  sum |= 1;
	}
      else if (!strncmp(arg_list[loop], "-f", 2))
	{
	  family = (int) strtol (arg_list[loop] + 2, &ptr, 10);
	  if (!ptr)
	    return(-1);
          sum |= 2;
	}
      else if (!strncmp(arg_list[loop], "-t", 2))
	{
	  type = (int) strtol (arg_list[loop] + 2, &ptr, 10);
	  if (!ptr)
	    return(-1);
          sum |= 4;
	}
      else if (!strncmp(arg_list[loop], "-m", 2))
	{
	  *machine = strdup (arg_list[loop] + 2);
	  if (!(*machine)) return -1;
	  sum |= 8;
	}
      else if (!strncmp(arg_list[loop], "-l", 2))
	{
	  flag = (int) strtol (arg_list[loop] + 2, &ptr, 10);
	  if (!ptr)
	    return(-1);
          sum |= 16;
	}
    }
  if (sum == (16 + 8 + 4 + 2 + 1))
	return 0;
  else
	return -1;
}

char **
RPC_make_arg_list (char *program, int family, int type, int well_known_socket,
		   int flag)
{
  char arg_str[100];
  int arg_count = 0;
  char **arg_list;
  char machine_name[50];

  arg_list = new char*[7];
  arg_list[arg_count++] = strdup (program);
  sprintf(arg_str, "%s%d", "-p", well_known_socket);
  arg_list[arg_count++] = strdup (arg_str);
  sprintf(arg_str, "%s%d", "-f", family);
  arg_list[arg_count++] = strdup (arg_str);
  sprintf(arg_str, "%s%d", "-t", type);
  arg_list[arg_count++] = strdup (arg_str);
  gethostname (machine_name, 49);
  sprintf(arg_str, "%s%s", "-m", machine_name);
  arg_list[arg_count++] = strdup (arg_str);
  sprintf(arg_str, "%s%d", "-l", flag);
  arg_list[arg_count++] = strdup (arg_str);
  arg_list[arg_count++] = 0;
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
  
  bzero ((char *) &serv_addr, sizeof(serv_addr));
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
  int fd = 0;

  struct sockaddr_in serv_addr;
  struct hostent *hostptr = 0;
  struct in_addr *inadr = 0;

  __xdrs__ = 0;

  if ( (hostptr = gethostbyname(machine)) == 0)
    { fd = -1; return; }

  inadr = (struct in_addr *) hostptr->h_addr_list[0];
  bzero ((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = family;
  serv_addr.sin_addr = *inadr;
  serv_addr.sin_port = htons(req_port);

  if ( (fd = socket(family, type, 0)) < 0)
    { fd = -1; return; }

  if (connect(fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    { fd = -1; return; }

    __xdrs__ = new XDR;
    if (!readRoutine) readRoutine = RPCdefaultXDRRead;
    if (!writeRoutine) writeRoutine = RPCdefaultXDRWrite;
    (void) xdrrec_create(__xdrs__, 0, 0, (char *) fd, readRoutine,writeRoutine);
    if (nblock)
      fcntl (fd, F_SETFL, FNDELAY);

}

//
// prepare for RPC's to be done/received on the passed thread id.
//
THREADrpc::THREADrpc(int thread)
{
    tid = thread;
}

//
// This should never be called, it should be replaced by a virtual function
//    from the derived class created by igen.
//
void RPCUser::verifyProtocolAndVersion()
{
    abort();
}

//
// our version of string encoding that does malloc as needed.
//
bool_t xdr_String(XDR *xdrs, String *str)
{
    int len;

	// if XDR_FREE, str's memory is freed
    switch (xdrs->x_op) {
	case XDR_ENCODE:
		len = strlen(*str)+1;
		break;
	case XDR_DECODE:
		*str = NULL;
		break;
	case XDR_FREE:
		// xdr_free (xdr_string, str);
		if (*str)
		  free (*str);
		*str = NULL;
		return (TRUE);
		// return(TRUE);
		// free the memory
	default:
		assert(0);
		// this should never occur	
    }
    // should we have a better max length ???. 
    // xdr_bytes(xdrs, str, &len, 65536*32768);
    // return(TRUE);
    return (xdr_string (xdrs, str, 65536));
}

int RPCprocessCreate(int *pid, char *hostName, char *userName,
		     char *command, char **arg_list, int portFd)
{
    int ret;
    int sv[2];
    int execlERROR;
    char local[50];

    if (gethostname(local, 49))
	strcpy (local, " ");

    if (!hostName || 
	!strcmp(hostName, "") || 
	!strcmp(hostName, "localhost") ||
	!strcmp(hostName, local)) {
	ret = socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
	if (ret) return(ret);
	execlERROR = 0;
	*pid = vfork();
	if (*pid == 0) {
	    close(sv[0]);
	    dup2(sv[1], 0);
	    if (!arg_list)
	      execl(command, command);
	    else
	      execv(command, arg_list);
	    execlERROR = errno;
	    _exit(-1);
	} else if (*pid > 0 && !execlERROR) {
	    close(sv[1]);
	    return(sv[0]);
	} else {
	    return(-1);
	}
    } else {
	int total;
	int fd[2];
	char *ret;
	FILE *pfp;
	char **curr;
	int shellPid;
	char line[256];
	char *paradyndCommand;

	total = strlen(command) + 2;
	for (curr = arg_list+1; *curr; curr++) {
	    total += strlen(*curr) + 2;
	}
	paradyndCommand = (char *) malloc(total+2);

	sprintf(paradyndCommand, "%s ", command);
	for (curr = arg_list+1; *curr; curr++) {
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
	    dup2(fd[1], 1);                         /* copy it onto stdout */
	    close(fd[0]);
	    close(fd[1]);
	    if (userName) {
		execlp(RSH_COMMAND, RSH_COMMAND, hostName, "-l", 
		    userName, "-n", paradyndCommand, "-l0", NULL);
	    } else {
		execlp(RSH_COMMAND, RSH_COMMAND, hostName, "-n", 
		    paradyndCommand, "-l0", NULL);
	    }
	    _exit(-1);
	} else if (shellPid > 0) {
	    close(fd[1]);
	} else {
	    // error situation
	}

	pfp = fdopen(fd[0], "r");
	do {
	    ret = fgets(line, sizeof(line)-1, pfp);
	    if (ret && !strncmp(line, "PARADYND", strlen("PARADYND"))) {
		// got the good stuff
		sscanf(line, "PARADYND %d", pid);

		// dump rsh process
		kill(shellPid, SIGTERM);
		while (wait(NULL) != shellPid);

		return(RPC_getConnect(portFd));
	    } else if (ret) {
		// some sort of error message from rsh.
		printf("%s", line);
	    }
	}  while (ret);

	return(-1);
    }
}

//
// wait for an expected RPC responce, but also handle upcalls while waiting.
//    Should not be called directly !!!
//
void RPCUser::awaitResponce(int tag)
{
    abort();
}

void
XDRrpc::setNonBlock()
{
  if (fd >= 0)
    fcntl (fd, F_SETFL, FNDELAY);
}

int
XDRrpc::readReady(int timeout)
{
  return RPC_readReady (fd, timeout);
}

int
RPC_getConnect(int fd)
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

