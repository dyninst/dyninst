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

/*
 * $Log: rpcUtil.C,v $
 * Revision 1.50  1997/05/17 20:01:30  lzheng
 * Changes made for nonblocking write
 *
 * Revision 1.49  1997/04/21 16:57:56  hseom
 * added support for trace data (in a byte array)
 *
 * Revision 1.48  1997/02/26 23:49:54  mjrg
 * First part of WindowsNT commit: changes for compiling with VisualC++;
 * moved includes to platform header files
 *
 * Revision 1.47  1997/01/21 20:09:59  mjrg
 * Added support for unix domain sockets.
 * Added getHostName function
 *
 * Revision 1.46  1997/01/16 20:52:21  tamches
 * removed RPC_undo_arg_list (to paradynd)
 *
 * Revision 1.45  1996/11/26 16:09:37  naim
 * Fixing asserts - naim
 *
 * Revision 1.44  1996/11/12 17:50:16  mjrg
 * Removed warnings, changes for compiling with Visual C++ and xlc
 *
 * Revision 1.43  1996/08/16 21:32:02  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.42  1996/05/31 23:43:10  tamches
 * removed pid from XDRrpc.  Modified handleRemoteConnect appropriately.
 * Now paradynd doesn't try to send pid to paradyn (it wasn't being used
 * and was one cause of paradyn UI freezing on UI startup).
 *
 * Revision 1.41  1995/11/22 00:06:20  mjrg
 * Updates for paradyndPVM on solaris
 * Fixed problem with wrong daemon getting connection to paradyn
 * Removed -f and -t arguments to paradyn
 * Added cleanUpAndExit to clean up and exit from pvm before we exit paradynd
 * Fixed bug in my previous commit
 *
 * Revision 1.40  1995/11/12 00:44:28  newhall
 * fix to execCmd: forked process closes it's copy of parent's file descriptors
 *
 * Revision 1.39  1995/08/24  15:14:29  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.38  1995/05/18  11:12:15  markc
 * Added flavor arg to RPC_undo_g_list
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

#include <limits.h>
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

// one counter per file descriptor to record the sequnce no the message
// to be received
vector<int> counter_2;

// One partial message record for each file descriptor
vector<rpcBuffer *> partialMsgs;

int RPCasyncXDRRead(const void* handle, char *buf, const u_int len)
{
    /* called when paradyn/xdr detects that it needs to read a message
       from paradynd. */
    int fd = (int) handle;
    int header;
    int ret;
    int needCopy = 0;
    char *buffer = buf;
    u_int internal_len = 0;
    bool newfd = true;
    unsigned i;

    for (i = 0; i< partialMsgs.size(); i++) {
	if (partialMsgs[i] -> fd == fd) {
	    newfd = false;
	    break;
	}
    }
    
    // If new connection, allocate a partial message record
    if (newfd) {
	rpcBuffer *partial = new rpcBuffer;
	partial -> fd  = fd;
	partial -> len = 0;
	partial -> buf = NULL;
	partialMsgs += partial;
	i = partialMsgs.size() - 1;
	
	counter_2 += 0;
    }

    // There're two situations dealt with here: with partial message
    // left by previous read, without. The 'len' field is zero if it is
    // the first case, nonzero if it is the second case    
    if (partialMsgs[i] -> len) needCopy = 1;

    // Allocate buffer which is four more bytes than RPC buffer for the
    // convenience of processing message headers.
    buffer = new char[len + sizeof(int)];

    if (needCopy) {
	P_memcpy(buffer, partialMsgs[i]->buf , partialMsgs[i]->len); 
	delete [] partialMsgs[i]->buf;
	partialMsgs[i]->buf = NULL;
    }

    do {
	ret = P_read(fd, buffer+partialMsgs[i]->len, 
		     len + sizeof(int) -partialMsgs[i]->len);
    } while (ret < 0 && errno == EINTR);

    if (ret <= 0) { return(-1); }

    ret += partialMsgs[i]->len;

    char *pstart = buffer;
    
    // Processing the messages received: remove those message headers;
    // check on deliminator and sequence no to ensure the correctness;
    // save the partial message if any
    char *tail = buffer;
    int is_left = ret - sizeof(int);
    while (is_left >= 0) {
	P_memcpy((char *)&header, buffer, sizeof(int));
	assert(0xf == ((header >> 12)&0xf));

	short seq_no;
	P_memcpy((char *)&(seq_no), buffer, sizeof(short));

	header = (0x0fff&header);
	
	if (header <= is_left) {
	    P_memcpy(tail, buffer+sizeof(int), header);

	    counter_2[i] = ((counter_2[i]+1)%SHRT_MAX);
	    assert(seq_no == counter_2[i]);

	    internal_len += header;
	    if (internal_len > len) {
		abort();
		ret = ret - sizeof(int) - is_left;
		break;
	    }
	    tail += header;
	    buffer += header + sizeof(int);

	    is_left = is_left - header - sizeof(int);
	    ret -= sizeof(int);
	} else {
	    ret = ret - sizeof(int) - is_left;
	    break;
	}
    }



    if (is_left >= 0) {
	
	partialMsgs[i]->len = is_left + sizeof(int);
	partialMsgs[i]->buf = new char[partialMsgs[i]->len+1];
	P_memcpy(partialMsgs[i]->buf, buffer, partialMsgs[i]->len); 
    } else {
	partialMsgs[i]->len = 0;
	assert(is_left == (0-(int)sizeof(int)));
    }

    // Copy back to internal RPC buffer
    P_memcpy(buf, pstart, ret);

    if (needCopy) {
	buffer = pstart;
	assert(buffer != buf); // make sure we aren't deleting the 2d param
	delete [] buffer;
    }
    
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
    if (ret != (int)len)
      return(-1);
    else 
      return (ret);
}

vector<rpcBuffer *> rpcBuffers;
static short counter = 0;

int RPCasyncXDRWrite(const void* handle, const char *buf, const u_int len)
{
    int ret;
    int index = 0;
    int header = len;

    rpcBuffer *rb = new rpcBuffer;
    rb -> fd = (int) handle;
    rb -> len = len+sizeof(int);
    rb -> buf = new char[rb -> len];

    counter = ((counter+1)%SHRT_MAX);

    assert(len <= 4040);

    // Adding a header to the messages sent   
    header = ((counter << 16) | header | 0xf000); 
    P_memcpy(rb -> buf, (char *)&header, sizeof(int));

    P_memcpy(rb -> buf+sizeof(int), buf, len);

    rpcBuffers += rb;

    // Write the items to the other end if possible with asynchrous write
    for (int i = 0; (i < (int)rpcBuffers.size()); i++) {

	if (P_fcntl (rpcBuffers[i]->fd, F_SETFL, FNONBLOCK) == -1)
	    perror("fcntl");

	ret = (int)P_write(rpcBuffers[i]->fd, rpcBuffers[i]->buf,
			   rpcBuffers[i]->len);

	if (P_fcntl (rpcBuffers[i]->fd, F_SETFL, FSYNC) == -1)
	    perror("fcntl");

	if (rpcBuffers[i]->len != ret) {

	    if (ret != -1) {
		assert(ret < rpcBuffers[i]->len);
		printf("Warning: a partial message sent!\n");
		P_memcpy(rpcBuffers[i]->buf, rpcBuffers[i]->buf+ret,
			 rpcBuffers[i]->len-ret);
		rpcBuffers[i]->len -= ret;
	    }
	    break;
	}
	index++;
	
    }
    
    // Delete the items sent out
    unsigned j;
    for (j = 0; j < (unsigned)index; j++) {
	delete [] rpcBuffers[j]->buf;
    }    
    
    for (j = 0; j < rpcBuffers.size() - index; j++)
	rpcBuffers[j] = rpcBuffers[j+index];
    rpcBuffers.resize(rpcBuffers.size() - index);

    return (ret = (int)len);
}

void
doDeferedRPCasyncXDRWrite() {

    if (rpcBuffers.size() == 0)
	return;

    int ret, index = 0;

    // Write the items to the other end if possible 
    for (int i = 0; (i < (int)rpcBuffers.size()); i++) {

	if (P_fcntl (rpcBuffers[i]->fd, F_SETFL, FNONBLOCK) == -1)
	    perror("fcntl");

	ret = (int)P_write(rpcBuffers[i]->fd, rpcBuffers[i]->buf,
			   rpcBuffers[i]->len);

	if (P_fcntl (rpcBuffers[i]->fd, F_SETFL, FSYNC) == -1)
	    perror("fcntl");
	
	if (rpcBuffers[i]->len != ret) {

	    if (ret != -1) {
		assert(ret < rpcBuffers[i]->len);
		printf("Warning: a partial message sent!\n");
		P_memcpy(rpcBuffers[i]->buf, rpcBuffers[i]->buf+ret,
			 rpcBuffers[i]->len-ret);
		rpcBuffers[i]->len -= ret;
	    }
	    break;
	}
	index++;

    }
    
    // Delete the items sent out
    unsigned j;
    for (j = 0; j < (unsigned)index; j++) {
	delete [] rpcBuffers[j]->buf;
    }    

    for (j = 0; j < rpcBuffers.size() - index; j++)
	rpcBuffers[j] = rpcBuffers[j+index];
    rpcBuffers.resize(rpcBuffers.size() - index);
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
XDRrpc::XDRrpc(const int f, xdr_rd_func readRoutine, xdr_wr_func writeRoutine, const int nblock)
: xdrs(NULL), fd(f)
{
    assert(fd >= 0);
    xdrs = new XDR;
    assert(xdrs);
    if (!readRoutine) {
	if (nblock == 1) {
	    readRoutine = (xdr_rd_func) RPCasyncXDRRead;
	} else { 
	    readRoutine = (xdr_rd_func) RPCdefaultXDRRead;
	}
    }   
    if (!writeRoutine) {
	if (nblock == 2) {
	    writeRoutine = (xdr_wr_func)RPCasyncXDRWrite;
	} else {    
	    writeRoutine = (xdr_wr_func) RPCdefaultXDRWrite;
	}
    }	
    P_xdrrec_create(xdrs, 0, 0, (char *) fd, readRoutine, writeRoutine);
}

//
// prepare for RPC's to be done/received on the passed fd.
//
XDRrpc::XDRrpc(const string &machine,
	       const string &user,
	       const string &program,
	       xdr_rd_func readRoutine, 
	       xdr_wr_func writeRoutine,
	       const vector<string> &arg_list,
	       const int nblock,
	       const int wellKnownPortFd)
: xdrs(NULL), fd(-1)
{
    fd = RPCprocessCreate(machine, user, program, arg_list, wellKnownPortFd);
    if (fd >= 0) {
        xdrs = new XDR;
	if (!readRoutine) {
	    if (nblock == 1) {
		readRoutine = (xdr_rd_func) RPCasyncXDRRead;
	    } else { 
		readRoutine = (xdr_rd_func) RPCdefaultXDRRead;
	    }
	}
	if (!writeRoutine) {
	    if (nblock == 2) {
		writeRoutine = (xdr_wr_func)RPCasyncXDRWrite;
	    } else { 
		writeRoutine = (xdr_wr_func) RPCdefaultXDRWrite;
	    }
	}
	P_xdrrec_create(xdrs, 0, 0, (char *) fd, readRoutine, writeRoutine);
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

/*
 * Build an argument list starting at position 0.
 * Note, this arg list will be used in an exec system call
 * AND, the command name will have to be inserted at the head of the list
 * But, a NULL space will NOT be left at the head of the list
 */
bool RPC_make_arg_list(vector<string> &list,
		       const int well_known_socket, const int flag, const int firstPVM,
		       const string machine_name, const bool use_machine)
{
  char arg_str[100];

  list.resize(0);

  sprintf(arg_str, "%s%d", "-p", well_known_socket);  
  list += arg_str;
  // arg_list[arg_count++] = strdup (arg_str);  // 0

  if (!use_machine) {
    list += string("-m") + getHostName();
  } else {
    list += string("-m") + machine_name;
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
// (actually, looks like it returns the port number listened on, or -1)
int
RPC_setup_socket (int &sfd,   // return file descriptor
		  const int family, // AF_INET ...
		  const int type)   // SOCK_STREAM ...
{
  if ((sfd = P_socket(family, type, 0)) < 0)
    return -1;

  struct sockaddr_in serv_addr;
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

// setup a AF_UNIX domain socket of type SOCK_STREAM
bool
RPC_setup_socket_un (int &sfd,   // return file descriptor
		     const char *path) // file path socket is bound to
{
  struct sockaddr_un saddr;

  if ((sfd = P_socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    return false;

  saddr.sun_family = AF_UNIX;
  P_strcpy(saddr.sun_path, path);
  
  if (P_bind(sfd, (struct sockaddr *) &saddr, sizeof(saddr)) < 0)
    return false;

  if (P_listen(sfd, 5) < 0)
    return false;

  return true;
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
	       const int nblock) : xdrs(NULL), fd(-1)
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
    if (!readRoutine) {
	if (nblock == 1) {
	    readRoutine = (xdr_rd_func) RPCasyncXDRRead;
	} else {  
	    readRoutine = (xdr_rd_func) RPCdefaultXDRRead;
	}
	}
    if (!writeRoutine) {
	if (nblock == 2) {
	    writeRoutine = (xdr_wr_func)RPCasyncXDRWrite;
	} else { 
	    writeRoutine = (xdr_wr_func) RPCdefaultXDRWrite;
	}
    }
    P_xdrrec_create(xdrs, 0, 0, (char *) fd, readRoutine,writeRoutine);
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
    if ((length = str->length())) {
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

// trace data streams
// XDR_FREE not handled, free it yourself!
// our version of string encoding that does malloc as needed.
//

bool_t xdr_byteArray_pd(XDR *xdrs, byteArray *bArray)
{
  unsigned int length = 0;
  assert(bArray);

  // if XDR_FREE, str's memory is freed
  switch (xdrs->x_op) {
  case XDR_ENCODE:
    if (length = bArray->length()) {
      if (!P_xdr_u_int(xdrs, &length))
        return FALSE;
      else {
        char *buffer = (char*) bArray->getArray();
        bool_t res = P_xdr_byteArray (xdrs, &buffer, &length, (bArray->length()));
        return res;
      }
    } else {
      return (P_xdr_u_int(xdrs, &length));
    }
  case XDR_DECODE:
    if (!P_xdr_u_int(xdrs, &length))
      return FALSE;
     else if (!length) {
       *bArray = byteArray( (char *)NULL , 0);
       return TRUE;
     } else {
       char *temp;
       unsigned int act_len;
       unsigned int max_len = length;
       temp = new char[length];
       if (!P_xdr_byteArray (xdrs, &temp, &act_len, max_len)) {
          delete temp;
          return FALSE;
       } else if (act_len != length) {
          delete temp;
          return FALSE;
       } else {
          *bArray = byteArray(temp,act_len);
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

int execCmd(const string command, const vector<string> &arg_list, int /*portFd*/)
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

  int pid;
  pid = vfork();
  // close sv[0] after exec 
  P_fcntl(sv[0],F_SETFD,1);  

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

int handleRemoteConnect(int fd, int portFd) {
    // NOTE: This routine is not generic; it is very specific to paradyn
    // (due to the sscanf(line, "PARADYND %d")...)
    // Hence it makes absolutely no sense to put it in a so-called library.
    // It should be put into the paradyn/src tree --ari

    FILE *pfp = P_fdopen(fd, "r");
    if (pfp == NULL) {
       cerr << "handleRemoteConnect: fdopen of fd " << fd << " failed." << endl;
       return -1;
    }

    int retFd = RPC_getConnect(portFd); // calls accept(), which blocks (sigh...)
    return retFd;
}

//
// use rsh to get a remote process started.
//
// We do an rsh to start a process and them wait for the process 
// to do a connect. There is no guarantee that the process we are waiting for is
// the one that gets the connection. This can happen with paradyndPVM: the
// daemon that is started by a rshCommand will start other daemons, and one of 
// these daemons may get the connection that should be for the first daemon.
// Daemons should always get a connection before attempting to start other daemons.
//

int rshCommand(const string hostName, const string userName, 
	       const string command, const vector<string> &arg_list, int portFd)
{
    int fd[2];
    int shellPid;
    int ret;

    int total = command.length() + 2;
    for (unsigned i=0; i<arg_list.size(); i++) {
      total += arg_list[i].length() + 2;
    }

    string paradyndCommand = command + " ";

    for (unsigned j=0; j < arg_list.size(); j++)
        paradyndCommand += arg_list[j] + " ";

    // need to rsh to machine and setup io path.

    if (pipe(fd)) {
	perror("pipe");
	return (-1);
    }

    shellPid = vfork();
    if (shellPid == 0) {
	/* child */
	bool aflag;
	aflag=(-1 != dup2(fd[1], 1)); /* copy it onto stdout */
	assert(aflag);
	aflag=(-1 != close(fd[0]));
	assert(aflag);
	aflag=(-1 != close(fd[1]));
	assert(aflag);
	if (userName.length()) {
	    ret = execlp(RSH_COMMAND, RSH_COMMAND, hostName.string_of(), "-l", 
			 userName.string_of(), "-n", paradyndCommand.string_of(),
			 "-l0", NULL);
            fprintf(stderr,"rshCommand: execlp failed (ret = %d)\n",ret);
	} else {
	    ret = execlp(RSH_COMMAND, RSH_COMMAND, hostName.string_of(), "-n", 
			 paradyndCommand.string_of(), "-l0", NULL);
            fprintf(stderr,"rshCommand: execlp failed (ret = %d)\n",ret);
	}
	_exit(-1);
    } else if (shellPid > 0) {
	close(fd[1]);
    } else {
	// error situation
    }

    return(handleRemoteConnect(fd[0], portFd));
}

int rexecCommand(const string hostName, const string userName, 
		 const string command, const vector<string> &arg_list, int portFd)
{
    struct servent *inport;

    int total = command.length() + 2;
    for (unsigned i=0; i<arg_list.size(); i++) {
      total += arg_list[i].length() + 2;
    }

    char *paradyndCommand = new char[total+2];
    assert(paradyndCommand);

    sprintf(paradyndCommand, "%s ", command.string_of());
    for (unsigned j=0; j<arg_list.size(); j++) {
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

    return(handleRemoteConnect(fd, portFd));
}

/*
 * 
 * "command" will be appended to the front of the arg list
 *
 * if arg_list == NULL, command is the only argument used
 */

int RPCprocessCreate(const string hostName, const string userName,
		     const string command, const vector<string> &arg_list,
		     int portFd, const bool useRexec)
{
    int ret;
    struct utsname unm;

    if (P_uname(&unm) == -1)
      assert(0);

    if ((hostName == "") || 
	(hostName == "localhost") ||
	(hostName == unm.nodename))
      ret = execCmd(command, arg_list, portFd);
    else if (useRexec)
      ret = rexecCommand(hostName, userName, command, arg_list, portFd);
    else
      ret = rshCommand(hostName, userName, command, arg_list, portFd);

    return(ret);
}

int RPC_getConnect(const int fd) {
  if (fd == -1)
    return -1;

  struct in_addr cli_addr;
  size_t clilen = sizeof(cli_addr);
  int new_fd = P_accept (fd, (struct sockaddr *) &cli_addr, &clilen);

  if (new_fd < 0)
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


string getHostName() {
    struct utsname un;
    P_uname(&un);
    return string(un.nodename);
}
