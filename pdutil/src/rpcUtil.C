/*
 * $Log: rpcUtil.C,v $
 * Revision 1.38  1995/05/18 11:12:15  markc
 * Added flavor arg to RPC_undo_g_list
 *
 * Revision 1.37  1995/02/16  09:28:10  markc
 * Removed compiler warnings.
 * Changed Boolean to bool
 *
 * Revision 1.35  1994/11/11  06:59:23  markc
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
// This is ugly, and I hope to get rid of it -- mdc 2/2/95
#if defined(notdef)
/* prevents malloc from being redefined */
#ifdef MIPS
#define MALLOC_DEFINED_AS_VOID
#endif
#endif

#include "util/h/rpcUtil.h"

const char *RSH_COMMAND="rsh";

int RPCdefaultXDRRead(const void* handle, char *buf, const u_int len)
{
    int fd = (int) handle;
    int ret;

    do {
	ret = P_read(fd, buf, len);
    } while (ret < 0 && errno == EINTR);

    if (ret <= 0) { return(-1); }
    return (ret);
}

int RPCdefaultXDRWrite(const void* handle, const char *buf, const u_int len)
{
    int fd = (int) handle;
    int ret;

    do {

	ret = P_write(fd, buf, len);
    } while (ret < 0 && errno == EINTR);

    errno = 0;
    if (ret != len)
      return(-1);
    else
      return (ret);
}

XDRrpc::~XDRrpc()
{
  if (fd >= 0)
    {
      P_fcntl (fd, F_SETFL, FNDELAY);
      P_close(fd);
      fd = -1;
    }
  if (xdrs) 
    {
      P_xdr_destroy (xdrs);
      delete (xdrs);
      xdrs = NULL;
    }
}

//
// prepare for RPC's to be done/received on the passed fd.
//
XDRrpc::XDRrpc(int f, xdr_rd_func readRoutine, xdr_wr_func writeRoutine, const bool nblock)
: xdrs(NULL), fd(f), pid(-1)
{
    assert(fd >= 0);
    xdrs = new XDR;
    assert(xdrs);
    if (!readRoutine) readRoutine = (xdr_rd_func) RPCdefaultXDRRead;
    if (!writeRoutine) writeRoutine = (xdr_wr_func) RPCdefaultXDRWrite;
    P_xdrrec_create(xdrs, 0, 0, (char *) fd, readRoutine, writeRoutine);
    if (nblock)
      P_fcntl (fd, F_SETFL, FNDELAY);
}

//
// prepare for RPC's to be done/received on the passed fd.
//
XDRrpc::XDRrpc(const string machine,
	       const string user,
	       const string program,
	       xdr_rd_func readRoutine, 
	       xdr_wr_func writeRoutine,
	       const vector<string> &arg_list,
	       const bool nblock,
	       int wellKnownPortFd)
: xdrs(NULL), fd(-1), pid(-1)
{
    fd = RPCprocessCreate(pid, machine, user, program, arg_list, wellKnownPortFd);
    if (fd >= 0) {
        xdrs = new XDR;
	if (!readRoutine) readRoutine = (xdr_rd_func) RPCdefaultXDRRead;
	if (!writeRoutine) writeRoutine = (xdr_wr_func) RPCdefaultXDRWrite;
	P_xdrrec_create(xdrs, 0, 0, (char *) fd, readRoutine, writeRoutine);
	if (nblock)
	  P_fcntl (fd, F_SETFL, FNDELAY);
    }
}

bool
RPC_readReady (int fd, int timeout)
{
  fd_set readfds;
  struct timeval tvptr, *the_tv;

  tvptr.tv_sec = timeout; tvptr.tv_usec = 0;
  if (fd < 0) return false;
  FD_ZERO(&readfds);
  FD_SET (fd, &readfds);

  // -1 timeout = blocking select
  if (timeout == -1)
     the_tv = 0;
  else
     the_tv = &tvptr;
 
  if (P_select (fd+1, &readfds, NULL, NULL, the_tv) == -1)
    {
      // if (errno == EBADF)
	return false;
    }
  return (FD_ISSET (fd, &readfds));
}

// TODO
// mdc - I need to clean this up
bool
RPC_undo_arg_list (string& flavor, int argc, char **arg_list, string &machine,
		   int &family, int &type, int &well_known_socket, int &flag,
		   int &firstPVM)
{
  int loop;
  char *ptr;
  bool b_well_known=false, b_family=false, b_first=false, b_type = false,
  b_machine = false, b_flag = false, b_flavor=false;

  for (loop=0; loop < argc; ++loop)
    {
      if (!P_strncmp(arg_list[loop], "-p", 2))
	{
	  well_known_socket = P_strtol (arg_list[loop] + 2, &ptr, 10);
	  if (ptr == (arg_list[loop] + 2))
	    return(false);
	  b_well_known = true;
	}
      else if (!P_strncmp(arg_list[loop], "-f", 2))
	{
	  family = P_strtol (arg_list[loop] + 2, &ptr, 10);
	  if (ptr == (arg_list[loop] + 2))
	    return(false);
	  b_family = true;
	}
      else if (!P_strncmp(arg_list[loop], "-v", 2))
	{
	  firstPVM = P_strtol (arg_list[loop] + 2, &ptr, 10);
	  if (ptr == (arg_list[loop] + 2))
	    return(false);
	  b_first = true;
	}
      else if (!P_strncmp(arg_list[loop], "-t", 2))
	{
	  type = P_strtol (arg_list[loop] + 2, &ptr, 10);
	  if (ptr == (arg_list[loop] + 2))
	    return(false);
	  b_type = true;
	}
      else if (!P_strncmp(arg_list[loop], "-m", 2))
	{
	  machine = (arg_list[loop] + 2);
	  if (!machine.length()) return false;
	  b_machine = true;
	}
      else if (!P_strncmp(arg_list[loop], "-l", 2))
	{
	  flag = P_strtol (arg_list[loop] + 2, &ptr, 10);
	  if (ptr == (arg_list[loop] + 2))
	    return(false);
	  b_flag = true;
	}
      else if (!P_strncmp(arg_list[loop], "-z", 2))
	{
	  flavor = (arg_list[loop]+2);
	  if (!flavor.length()) return false;
	  b_flavor = true;
	}
    }
  return (b_flag && b_family && b_first && b_machine &&
	  b_type && b_well_known && b_flavor);
}

/*
 * Build an argument list starting at position 0.
 * Note, this arg list will be used in an exec system call
 * AND, the command name will have to be inserted at the head of the list
 * But, a NULL space will NOT be left at the head of the list
 */
bool RPC_make_arg_list(vector<string> &list, const int family, const int type, 
		       const int well_known_socket, const int flag, const int firstPVM,
		       const string machine_name, const bool use_machine)
{
  char arg_str[100];

  list.resize(0);

  sprintf(arg_str, "%s%d", "-p", well_known_socket);  
  list += arg_str;
  // arg_list[arg_count++] = strdup (arg_str);  // 0

  sprintf(arg_str, "%s%d", "-f", family);
  list += arg_str;
  // arg_list[arg_count++] = strdup (arg_str);  // 1

  sprintf(arg_str, "%s%d", "-t", type);
  list += arg_str;
  // arg_list[arg_count++] = strdup (arg_str);  // 2

  if (!use_machine) {
    struct utsname unm;
    if (P_uname(&unm) == -1)
      assert(0);
    sprintf(arg_str, "%s%s", "-m", unm.nodename);
    list += arg_str;
  } else {
    sprintf(arg_str, "%s%s", "-m", machine_name.string_of());
    list += arg_str;
  }
  // arg_list[arg_count++] = strdup (arg_str); // 3

  sprintf(arg_str, "%s%d", "-l", flag);
  list += arg_str;
  //arg_list[arg_count++] = strdup (arg_str);  // 4

  sprintf(arg_str, "%s%d", "-v", firstPVM);
  list += arg_str;
  // arg_list[arg_count++] = strdup (arg_str);  // 5 

  return true;
}

// returns fd of socket that is listened on, or -1
int
RPC_setup_socket (int &sfd,   // return file descriptor
		  const int family, // AF_INET ...
		  const int type)   // SOCK_STREAM ...
{
  struct sockaddr_in serv_addr;

  if ((sfd = P_socket(family, type, 0)) < 0)
    return -1;

  P_memset ((void*) &serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = (short) family;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(0);
  
  size_t length = sizeof(serv_addr);

  if (P_bind(sfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    return -1;

  if (P_getsockname (sfd, (struct sockaddr *) &serv_addr, &length) < 0)
    return -1;

  if (P_listen(sfd, 5) < 0)
    return -1;

  return (ntohs (serv_addr.sin_port));
}

//
// connect to well known socket
//
XDRrpc::XDRrpc(int family,            
	       int req_port,             
	       int type,
	       const string machine, 
	       xdr_rd_func readRoutine,
	       xdr_wr_func writeRoutine,
	       const bool nblock) : xdrs(NULL), fd(-1), pid(-1)
     // socket, connect using machine
{
  struct sockaddr_in serv_addr;
  struct hostent *hostptr = 0;
  struct in_addr *inadr = 0;
  if (!(hostptr = P_gethostbyname(machine.string_of())))
    return;

  inadr = (struct in_addr *) ((void*) hostptr->h_addr_list[0]);
  P_memset ((void*) &serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = family;
  serv_addr.sin_addr = *inadr;
  serv_addr.sin_port = htons(req_port);

  if ( (fd = P_socket(family, type, 0)) < 0)
    { fd = -1; return; }

  if (P_connect(fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    { fd = -1; return; }

    xdrs = new XDR;
    assert(xdrs);
    if (!readRoutine) readRoutine = (xdr_rd_func) RPCdefaultXDRRead;
    if (!writeRoutine) writeRoutine = (xdr_wr_func) RPCdefaultXDRWrite;
    P_xdrrec_create(xdrs, 0, 0, (char *) fd, readRoutine,writeRoutine);
    if (nblock)
      P_fcntl (fd, F_SETFL, FNDELAY);

}

//
// These xdr functions are not to be called with XDR_FREE
//
bool_t xdr_Boolean(XDR *xdrs, bool *bool_val) {
   u_char i;
   bool_t res;
   switch (xdrs->x_op) {
   case XDR_ENCODE:
      i = (u_char) *bool_val;
      return (P_xdr_u_char(xdrs, &i));
   case XDR_DECODE:
      res = P_xdr_u_char(xdrs, &i);
      *bool_val = (bool) i;
      return res;
   case XDR_FREE:
   default:
      assert(0);
      return FALSE;
   }
}

// XDR_FREE not handled, free it yourself!
// our version of string encoding that does malloc as needed.
//
bool_t xdr_string_pd(XDR *xdrs, string *str)
{
  unsigned int length = 0;
  assert(str);

  // if XDR_FREE, str's memory is freed
  switch (xdrs->x_op) {
  case XDR_ENCODE:
    if (length = str->length()) {
      if (!P_xdr_u_int(xdrs, &length))
	return FALSE;
      else {
	char *buffer = (char*) str->string_of(); 
	bool_t res = P_xdr_string (xdrs, &buffer, str->length() + 1);
	return res;
      }
    } else {
      return (P_xdr_u_int(xdrs, &length));
    }
  case XDR_DECODE:
    if (!P_xdr_u_int(xdrs, &length))
      return FALSE;
     else if (!length) {
       *str = (char*) NULL;
       return TRUE;
     } else {
       char *temp; 
       bool newd = false;
       unsigned max_len = 511;
       char stat_buf[512];
       if (length < 512) 
          temp = (char*) stat_buf;
       else {
          temp = new char[length+1];
          max_len = length;
          newd = true;
       }      
       if (!P_xdr_string (xdrs, &temp, max_len)) {
          if (newd) 
            delete temp;
 	  return FALSE;
       } else {
	  *str = temp;
          if (newd)
            delete temp; 
	  return TRUE;
       }
     }
  case XDR_FREE:
  default:
    // this should never occur	
    assert(0);
    return (FALSE);
  }
}

//
// directly exec the command (local).
//
int execCmd(int &pid, const string command, const vector<string> &arg_list, int portFd)
{
  int ret;
  int sv[2];
  int execlERROR;

  errno = 0;
  ret = P_socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
  if (ret==-1) return(ret);
  execlERROR = 0;
  int al_len = arg_list.size();
  char **new_al = new char*[al_len+2];
  // TODO argv[0] should not include path
  new_al[0] = P_strdup(command.string_of());
  new_al[al_len+1] = NULL;
  for (int i=0; i<al_len; ++i)
    new_al[i+1] = P_strdup(arg_list[i].string_of());
  ret = -1;

  pid = vfork();

  if (pid == 0) {
    if (P_close(sv[0])) { execlERROR = errno; _exit(-1);}
    if (P_dup2(sv[1], 0)) { execlERROR = errno; _exit(-1);}
    P_execvp(new_al[0], new_al);
    execlERROR = errno;
    _exit(-1);
  } else if (pid > 0 && !execlERROR) {
    P_close(sv[1]);
    ret = sv[0];
  }

  al_len=0;
  while (new_al[al_len]) {
    delete (new_al[al_len]);
    al_len++;
  }
  delete(new_al);
  return ret;
}

int handleRemoteConnect(int &pid, int fd, int portFd)
{
    char *ret;
    int retFd;
    FILE *pfp;
    char line[256];

    pfp = P_fdopen(fd, "r");
    if (pfp == NULL) {
       cerr << "handleRemoteConnect: fdopen of fd " << fd << " failed." << endl;
       return -1;
    }

    do {
	ret = P_fgets(line, sizeof(line)-1, pfp);
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
int rshCommand(int &pid, const string hostName, const string userName, 
	       const string command, const vector<string> &arg_list, int portFd)
{
    int fd[2];
    int shellPid;
    int ret;

    int total = command.length() + 2;
    for (int i=0; i<arg_list.size(); i++) {
      total += arg_list[i].length() + 2;
    }

    char *paradyndCommand = (char *) P_malloc(total+2);

    sprintf(paradyndCommand, "%s ", command.string_of());
    for (int j=0; j<arg_list.size(); j++) {
	P_strcat(paradyndCommand, arg_list[j].string_of());
	P_strcat(paradyndCommand, " ");
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
	if (userName.length()) {
	    ret = execlp(RSH_COMMAND, RSH_COMMAND, hostName.string_of(), "-l", 
			 userName.string_of(), "-n", paradyndCommand, "-l0", NULL);
            fprintf(stderr,"rshCommand: execlp failed (ret = %d)\n",ret);
	} else {
	    ret = execlp(RSH_COMMAND, RSH_COMMAND, hostName.string_of(), "-n", 
			 paradyndCommand, "-l0", NULL);
            fprintf(stderr,"rshCommand: execlp failed (ret = %d)\n",ret);
	}
	_exit(-1);
    } else if (shellPid > 0) {
	close(fd[1]);
    } else {
	// error situation
    }
    if (paradyndCommand)
      delete paradyndCommand;

    return(handleRemoteConnect(pid, fd[0], portFd));
}

int rexecCommand(int &pid, const string hostName, const string userName, 
		 const string command, const vector<string> &arg_list, int portFd)
{
    struct servent *inport;

    int total = command.length() + 2;
    for (int i=0; i<arg_list.size(); i++) {
      total += arg_list[i].length() + 2;
    }
    char *paradyndCommand = (char *) P_malloc(total+2);

    sprintf(paradyndCommand, "%s ", command.string_of());
    for (int j=0; j<arg_list.size(); j++) {
	P_strcat(paradyndCommand, arg_list[j].string_of());
	P_strcat(paradyndCommand, " ");
    }

    inport = P_getservbyname("exec",  "tcp");

    char *hname = P_strdup(hostName.string_of());
    char *uname = P_strdup(userName.string_of());
    int fd = P_rexec(&hname, inport->s_port, uname, NULL,
		   paradyndCommand, NULL);
    delete hname; delete uname;

    if (fd < 0) {
	perror("rexec");
	printf("rexec failed\n");
    }
    if (paradyndCommand)
      delete paradyndCommand;

    return(handleRemoteConnect(pid, fd, portFd));
}

/*
 * 
 * "command" will be appended to the front of the arg list
 *
 * if arg_list == NULL, command is the only argument used
 */
int RPCprocessCreate(int &pid, const string hostName, const string userName,
		     const string command, const vector<string> &arg_list,
		     int portFd, const bool useRexec)
{
    int ret;
    struct utsname unm;

    if (P_uname(&unm) == -1)
      assert(0);

    if ((hostName == "") || 
	(hostName == "localhost") ||
	(hostName == unm.nodename)) {
      ret = execCmd(pid, command, arg_list, portFd);
    } else if (useRexec) {
      ret = rexecCommand(pid, hostName, userName, command, arg_list, portFd);
    } else {
      ret = rshCommand(pid, hostName, userName, command, arg_list, portFd);
    }
    return(ret);
  }

int RPC_getConnect(const int fd)
{
  struct in_addr cli_addr;
  int new_fd;

  if (fd == -1)
    return -1;

  size_t clilen = sizeof(cli_addr);

  if ((new_fd = P_accept (fd, (struct sockaddr *) &cli_addr, &clilen)) < 0)
    return -1;
  else
    return new_fd;
}

// TODO -- use vectors and strings ?
/*
 *  RPCgetArg - break a string into blank separated words
 *  Used to parse a command line.
 *  
 *  input --> a null terminated string, the command line
 *  argc --> returns the number of args found
 *  returns --> words (separated by blanks on command line)
 *  Note --> this allocates memory 
 */
bool RPCgetArg(vector<string> &arg, const char *input)
{
#define BLANK ' '

  int word_len;
  if (!input) 
    return false;

  int length = strlen(input);
  int index = 0;

  /* advance past blanks in input */
  while ((index < length) && (input[index] == BLANK))
    index++;

  /* input is all blanks, or NULL */
  if (index >= length) 
    return true;

  do {
    /* the start of each string */
    word_len = 0; const char *word_start = input + index;

    /* find the next BLANK and the WORD size */
    while ((index < length) && (input[index] != BLANK)) {
      index++; word_len++;
    }

    /* copy the word */
    char *temp = new char[word_len+1];
    strncpy(temp, word_start, word_len);
    temp[word_len] = (char) 0;
    arg += temp;
    delete temp;

    /* skip past consecutive blanks */
    while ((index < length) && (input[index] == BLANK))
      index++;
  } while (index < length);
  return true;
}

