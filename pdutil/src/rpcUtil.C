/*
 * Copyright (c) 1996-2002 Barton P. Miller
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

//
// This file defines a set of utility routines for RPC services.
// $Id: rpcUtil.C,v 1.81 2002/07/25 19:22:54 willb Exp $
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

#include <limits.h> // SHRT_MAX
#include "common/h/headers.h"
#include "pdutil/h/pdsocket.h"
#include "pdutil/h/rpcUtil.h"
#include "common/h/Types.h"  //Address

const char *DEF_RSH_COMMAND="rsh";
const char *RSH_COMMAND_ENV="PARADYN_RSH";

vector<RPCSockCallbackFunc>    rpcSockCallback;

//---------------------------------------------------------------------------
// prototypes of utility functions used in this file
//---------------------------------------------------------------------------
#if defined(i386_unknown_nt4_0)
bool CreateSocketPair( PDSOCKET& localSock, PDSOCKET& remoteSock );
#endif // defined(i386_unknown_nt4_0)


int RPCdefaultXDRRead(const void* handle, char *buf, const u_int len)
{
    PDSOCKET sock = (PDSOCKET) handle;
    int ret;

#if defined(i386_unknown_nt4_0)
    ret = recv( sock, buf, len, 0 );
#else
    do {
      ret = P_read(sock, buf, len);
    } while (ret < 0 && errno == EINTR);
#endif
    // call any user-defined callbacks
    unsigned int cbi;
    for( cbi = 0; cbi < rpcSockCallback.size(); cbi++ ) {
      assert( rpcSockCallback[cbi] != NULL );
      (*(rpcSockCallback[cbi]))( sock );
    }

    if( (ret == PDSOCKET_ERROR) || (ret == 0)) {
      return (-1);
    }
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
    PDSOCKET fd = (PDSOCKET) handle;
    unsigned header;
    int ret;
    int needCopy = 0;
    char *buffer = buf;
    u_int internal_len = 0;
    bool newfd = true;
    unsigned i;

    for (i = 0; i< partialMsgs.size(); i++) {
      assert(partialMsgs[i] != NULL);
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

#if defined(i386_unknown_nt4_0)
    ret = recv(fd, buffer+partialMsgs[i]->len,
	       len + sizeof(int) - partialMsgs[i]->len, 0);
#else
    do {
      ret = P_read(fd, buffer+partialMsgs[i]->len, 
		   len + sizeof(int) - partialMsgs[i]->len);
    } while (ret < 0 && errno == EINTR);
#endif
    // call any user-defined callbacks
    unsigned int cbi;
    for( cbi = 0; cbi < rpcSockCallback.size(); cbi++ ) {
      assert( rpcSockCallback[cbi] != NULL );
      (*(rpcSockCallback[cbi]))( fd );
    }

    if((ret == PDSOCKET_ERROR) || (ret == 0)) return(-1);

    ret += partialMsgs[i]->len;

    char *pstart = buffer;
    
    // Processing the messages received: remove those message headers;
    // check on deliminator and sequence no to ensure the correctness;
    // save the partial message if any
    char *tail = buffer;
    int is_left = ret - sizeof(int);
    while (is_left >= 0) {
      P_memcpy((char *)&header, buffer, sizeof(int));
      //printf(">> header=%x, header1=%x, len=%x\n", header, ntohl(header));
      header = ntohl(header);
      assert(0xf == ((header >> 12)&0xf));

      short seq_no;
      P_memcpy((char *)&(seq_no), buffer, sizeof(short));
      seq_no = ntohs(seq_no);
      header = (0x0fff&header);

      if (header <= (unsigned)is_left) {
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

//  if (needCopy) {
//    buffer = pstart;
//    assert(buffer != buf); // make sure we aren't deleting the 2d param
//    delete [] buffer;
//  }

    if (pstart != buf) delete [] pstart;

    return (ret);
}

int RPCdefaultXDRWrite(const void* handle, const char *buf, const u_int len)
{
    PDSOCKET sock = (PDSOCKET) handle;
    int ret;

#if defined(i386_unknown_nt4_0)
    ret = send(sock, buf, len, 0);
#else
    do {
      ret = P_write(sock, buf, len);
    } while (ret < 0 && errno == EINTR);
#endif

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
    unsigned index = 0;
    unsigned header = len;

    //printf("RPCasyncXDRWrite(handle=%p, buf=%p, len=%d)\n", handle, buf, len);

    rpcBuffer *rb = new rpcBuffer;
    rb -> fd = (int) handle;
    rb -> len = len+sizeof(int);
    rb -> buf = new char[rb -> len];

    counter = ((counter+1)%SHRT_MAX);

    assert(len <= 4040);

    // Adding a header to the messages sent   
    header = ((counter << 16) | header | 0xf000); 

    // Converting to network order
    header = htonl(header);
    //printf(">> header=%x, counter=%x, len=%x\n", header, counter, len);
    
    P_memcpy(rb -> buf, (char *)&header, sizeof(int));
    P_memcpy(rb -> buf+sizeof(int), buf, len);

    rpcBuffers += rb;

    // Write the items to the other end if possible with asynchrous write
    for (int i = 0; (i < (int)rpcBuffers.size()); i++) {

#if defined(i386_unknown_nt4_0)
      u_long ioctlsock_arg = 1; // set non-blocking
      if (ioctlsocket(rpcBuffers[i]->fd, FIONBIO, &ioctlsock_arg) == -1)
	perror("ioctlsocket");

      ret = (int) send(rpcBuffers[i]->fd, rpcBuffers[i]->buf, 
		       rpcBuffers[i]->len, 0);

      ioctlsock_arg = 0; // set blocking
      if (ioctlsocket(rpcBuffers[i]->fd, FIONBIO, &ioctlsock_arg) == -1)
	perror("ioctlsocket");
#else
      if (P_fcntl (rpcBuffers[i]->fd, F_SETFL, FNONBLOCK) == -1)
	perror("fcntl");

      ret = (int)P_write(rpcBuffers[i]->fd, rpcBuffers[i]->buf,
			 rpcBuffers[i]->len);

      if (P_fcntl (rpcBuffers[i]->fd, F_SETFL, FSYNC) == -1)
	perror("fcntl");
#endif
      if (rpcBuffers[i]->len != ret) {
	if (ret != -1) {
	  assert(ret < rpcBuffers[i]->len);
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
    for (j = 0; j < index; j++) {
      delete [] rpcBuffers[j]->buf;
      delete rpcBuffers[j];
      rpcBuffers[j] = NULL; // probably unnecessary
    }

    for (j = 0; j < rpcBuffers.size() - index; j++)
      rpcBuffers[j] = rpcBuffers[j+index];
    rpcBuffers.resize(rpcBuffers.size() - index);

    return (ret = (int)len);
}

void
doDeferedRPCasyncXDRWrite() {

    if (rpcBuffers.size() == 0) return;

    int ret, index = 0;

    // Write the items to the other end if possible 
    for (int i = 0; (i < (int)rpcBuffers.size()); i++) {

#if defined (i386_unknown_nt4_0)
      u_long ioctlsock_arg = 1; // set non-blocking
      if (ioctlsocket(rpcBuffers[i]->fd, FIONBIO, &ioctlsock_arg) == -1)
	perror("ioctlsocket");

      ret = (int) send(rpcBuffers[i]->fd, rpcBuffers[i]->buf,
		       rpcBuffers[i]->len, 0);

      ioctlsock_arg = 0; // set blocking
      if (ioctlsocket(rpcBuffers[i]->fd, FIONBIO, &ioctlsock_arg) == -1)
	perror("ioctlsocket");
#else
      if (P_fcntl (rpcBuffers[i]->fd, F_SETFL, FNONBLOCK) == -1)
	perror("fcntl");

      ret = (int)P_write(rpcBuffers[i]->fd, rpcBuffers[i]->buf,
			 rpcBuffers[i]->len);

      if (P_fcntl (rpcBuffers[i]->fd, F_SETFL, FSYNC) == -1)
	perror("fcntl");
#endif

      if (rpcBuffers[i]->len != ret) {
	if (ret != -1) {
	  assert(ret < rpcBuffers[i]->len);
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
  if (sock != PDSOCKET_ERROR) {
#if !defined(i386_unknown_nt4_0)
    P_fcntl (sock, F_SETFL, FNDELAY);
#endif
    CLOSEPDSOCKET( sock );
    sock = PDSOCKET_ERROR;
  }
  if (xdrs) {
    P_xdr_destroy (xdrs);
    delete (xdrs);
    xdrs = NULL;
  }
}

//
// prepare for RPC's to be done/received on the passed fd.
//
XDRrpc::XDRrpc(PDSOCKET use_sock, xdr_rd_func readRoutine, xdr_wr_func writeRoutine,
	       const int nblock) : xdrs(NULL), sock(use_sock) {
    assert(use_sock != PDSOCKET_ERROR);
    xdrs = new XDR;
#ifdef PURE_BUILD
    memset(xdrs,'\0',sizeof(XDR));
#endif
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
	writeRoutine = (xdr_wr_func) RPCasyncXDRWrite;
      } else {
	writeRoutine = (xdr_wr_func) RPCdefaultXDRWrite;
      }
    }
    P_xdrrec_create(xdrs, 0, 0, (char *) sock, readRoutine, writeRoutine);
}

//
// prepare for RPC's to be done/received on the passed fd.
//
XDRrpc::XDRrpc(const string &machine,
	       const string &user,
	       const string &program,
	       const string &remote_shell,
	       xdr_rd_func readRoutine,
	       xdr_wr_func writeRoutine,
	       const vector<string> &arg_list,
	       const int nblock,
	       PDSOCKET wellKnownSocket) : xdrs(NULL), sock(INVALID_PDSOCKET) {
    sock = RPCprocessCreate(machine, user, program, remote_shell, arg_list, wellKnownSocket);
    if (sock != INVALID_PDSOCKET) {
      xdrs = new XDR;
#ifdef PURE_BUILD
      memset(xdrs,'\0',sizeof(XDR));
#endif
      if (!readRoutine) {
	if (nblock == 1) {
	  readRoutine = (xdr_rd_func) RPCasyncXDRRead;
	} else {
	  readRoutine = (xdr_rd_func) RPCdefaultXDRRead;
	}
      }
      if (!writeRoutine) {
	if (nblock == 2) {
	  writeRoutine = (xdr_wr_func) RPCasyncXDRWrite;
	} else {
	  writeRoutine = (xdr_wr_func) RPCdefaultXDRWrite;
	}
      }
      P_xdrrec_create(xdrs, 0, 0, (char *) sock, readRoutine, writeRoutine);
    }
}

bool
RPC_readReady (PDSOCKET sock, int timeout)
{
  fd_set readfds;
  struct timeval tvptr, *the_tv;

  tvptr.tv_sec = timeout; tvptr.tv_usec = 0;
  if (sock == INVALID_PDSOCKET) return false;
  FD_ZERO(&readfds);
  FD_SET (sock, &readfds);

  // -1 timeout = blocking select
  if (timeout == -1)
     the_tv = 0;
  else
     the_tv = &tvptr;
 
  if (P_select (sock+1, &readfds, NULL, NULL, the_tv) == PDSOCKET_ERROR) {
    // if (errno == EBADF)
    return false;
  }
  return (FD_ISSET (sock, &readfds));
}

/*
 * Build an argument list starting at position 0.
 * Note, this arg list will be used in an exec system call
 * AND, the command name will have to be inserted at the head of the list
 * But, a NULL space will NOT be left at the head of the list
 */
bool RPC_make_arg_list(vector<string> &list,
		       const int well_known_socket,
#if !defined(i386_unknown_nt4_0)
		       const int termWin_port,
#endif
		       const int flag,
		       const int /* firstPVM */,
		       const string machine_name, const bool use_machine)
{
  char arg_str[100];

  list.resize(0);

  sprintf(arg_str, "%s%d", "-p", well_known_socket);  
  list += arg_str;
#if !defined(i386_unknown_nt4_0)
  sprintf(arg_str, "%s%d", "-P", termWin_port);
  list += arg_str;
#endif
  // arg_list[arg_count++] = strdup (arg_str);  // 0

  if (!use_machine) {
    list += string("-m") + getNetworkName();
  } else {
    list += string("-m") + machine_name;
  }
  // arg_list[arg_count++] = strdup (arg_str); // 3

  sprintf(arg_str, "%s%d", "-l", flag);
  list += arg_str;
  //arg_list[arg_count++] = strdup (arg_str);  // 4

  //-v option to paradynd is obsolete
  //sprintf(arg_str, "%s%d", "-v", firstPVM);
  //list += arg_str;
  // arg_list[arg_count++] = strdup (arg_str);  // 5 

  return true;
}

// returns fd of socket that is listened on, or -1
// (actually, looks like it returns the port number listened on, or -1)
// You can't specify a port; in this routine, the system chooses an available
// one for you; we return that port num.
// returns socket descriptor through sock parameter
int
RPC_setup_socket (PDSOCKET &sock,   // return socket descriptor
		  const int family, // AF_INET ...
		  const int type)   // SOCK_STREAM ...
{
  if ((sock = P_socket(family, type, 0)) == INVALID_PDSOCKET)
    return -1;

  struct sockaddr_in serv_addr;
  P_memset ((void*) &serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = (short) family;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(0);
  
  size_t length = sizeof(serv_addr);

  if (P_bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == PDSOCKET_ERROR)
    return -1;

  if (P_getsockname (sock, (struct sockaddr *) &serv_addr, &length) == PDSOCKET_ERROR)
    return -1;

  if (P_listen(sock, 128) == PDSOCKET_ERROR)  //Be prepared for lots of simultaneous connects
    return -1;

  return (ntohs (serv_addr.sin_port));
}

// setup a AF_UNIX domain socket of type SOCK_STREAM
// NT does not support AF_UNIX domain sockets
#if !defined(i386_unknown_nt4_0)
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

  if (P_listen(sfd, 128) < 0)  //Be prepared for lots of simultaneous connects
    return false;

  return true;
}
#endif // !defined(i386_unknown_nt4_0)

//
// connect to well known socket
//
XDRrpc::XDRrpc(int family,            
	       int req_port,             
	       int type,
	       const string machine, 
	       xdr_rd_func readRoutine,
	       xdr_wr_func writeRoutine,
	       const int nblock) : xdrs(NULL), sock(INVALID_PDSOCKET)
  // socket, connect using machine
{
  struct sockaddr_in serv_addr;
  struct hostent *hostptr = 0;
  struct in_addr *inadr = 0;
  if (!(hostptr = P_gethostbyname(machine.c_str()))) {
    cerr << "CRITICAL: Failed to find information for host " << machine.c_str() << endl;
    assert(0);
  }

  inadr = (struct in_addr *) ((void*) hostptr->h_addr_list[0]);
  P_memset ((void*) &serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = family;
  serv_addr.sin_addr = *inadr;
  serv_addr.sin_port = htons(req_port);

  if ( (sock = P_socket(family, type, 0)) == PDSOCKET_ERROR) { 
    sock = INVALID_PDSOCKET; 
    return; 
  }

  //connect() may timeout if lots of Paradynd's are trying to connect to
  //  Paradyn at the same time, so we keep retrying the connect().
  errno = 0;
  while (P_connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == PDSOCKET_ERROR) {
#if defined(i386_unknown_nt4_0)
    if (PDSOCKET_ERRNO != WSAETIMEDOUT)
#else
    if (errno != ETIMEDOUT)
#endif
      { sock = INVALID_PDSOCKET; return; } 
    CLOSEPDSOCKET(sock);
    if ((sock = P_socket(family, type, 0)) == PDSOCKET_ERROR)
      { sock = INVALID_PDSOCKET; return; }
    errno = 0;
  }

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
	writeRoutine = (xdr_wr_func) RPCasyncXDRWrite;
      } else {
	writeRoutine = (xdr_wr_func) RPCdefaultXDRWrite;
      }
    }
    P_xdrrec_create(xdrs, 0, 0, (char *) sock, readRoutine,writeRoutine);
} 

//
// directly exec the command (local).
//

PDSOCKET
execCmd(const string command, const vector<string> &arg_list)
{
  PDSOCKET ret;
  PDSOCKET sv[2];

#if !defined(i386_unknown_nt4_0)
  int execlERROR;

  errno = 0;
  ret = P_socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
  if (ret==-1) return(ret);
  execlERROR = 0;
  int al_len = arg_list.size();
  char **new_al = new char*[al_len+2];
  // TODO argv[0] should not include path
  new_al[0] = P_strdup(command.c_str());
  new_al[al_len+1] = NULL;
  for (int i=0; i<al_len; ++i)
    new_al[i+1] = P_strdup(arg_list[i].c_str());
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
    free(new_al[al_len]);
    al_len++;
  }
  delete [] new_al;

#else // !defined(i386_unknown_nt4_0)

  // create a pair of socket endpoints and connect them
  // 
  // note that we create TCP/IP sockets here rather than
  // using a named pipe because the socket returned from
  // this function is used to communicate with a Paradyn
  // daemon, and the communication between the WinNT
  // Paradyn front end and a Paradyn daemon is implemented
  // using a socket to support communication with daemons
  // on remote machines
  if( !CreateSocketPair( sv[0], sv[1] ) ) {
    return INVALID_PDSOCKET;
  }

  // build a command line for the new process
  string cmdLine( command );
  unsigned i;
  for( i = 0; i < arg_list.size(); i++ ) {
    cmdLine += " ";
    cmdLine += arg_list[i];
  }

  // set up the standard input of the new process to 
  // be connected to our shared socket, and make sure
  // that the new process inherits our standard
  // output and standard error
  STARTUPINFO startInfo;
  ZeroMemory( &startInfo, sizeof(startInfo) );
  startInfo.cb = sizeof( startInfo );

  startInfo.hStdInput = (HANDLE)sv[1];
  startInfo.hStdOutput = GetStdHandle( STD_OUTPUT_HANDLE );
  startInfo.hStdError = GetStdHandle( STD_ERROR_HANDLE );
  startInfo.dwFlags |= STARTF_USESTDHANDLES;

  // actually create the process
  PROCESS_INFORMATION procInfo;
  BOOL bCreated = CreateProcess( NULL,
				 (char*)cmdLine.c_str(),
				 NULL,
				 NULL,
				 TRUE,
				 NORMAL_PRIORITY_CLASS,
				 NULL,
				 NULL,
				 &startInfo,
				 &procInfo );

  if( bCreated ) {
    // We want to close our end of the connected socket
    // pair, but we have a race condition between our 
    // closing of the socket and the initialization of
    // the process we just created.  We need to ensure
    // that the other process is far enough along that
    // our closing of the socket won't break the
    // connection completely.
    bool bOKToContinue = false;
    while( !bOKToContinue ) {
      HANDLE hProc = OpenProcess( PROCESS_QUERY_INFORMATION,
				  NULL,
				  procInfo.dwProcessId );
      if( hProc != NULL ) {
	CloseHandle( hProc );
	bOKToContinue = true;
      }
    }

    // release our hold on the child socket endpoint
    // and keep hold of our own endpoint
    CLOSEPDSOCKET( sv[1] );
    ret = sv[0];
  } else {
    // cleanup
    CLOSEPDSOCKET( sv[0] );
    CLOSEPDSOCKET( sv[1] );

    // indicate the error
    ret = INVALID_PDSOCKET;
  }

#endif // !defined(i386_unknown_nt4_0)

  return ret;
}

// Parse an input string into space-delimited substrings, and add each as a
// separate string the given vector

void appendParsedString( vector< string > &strList, const string &str )
{
  unsigned i, j, l = str.length();

  for( i = 0, j = 0; j < l; ++j )
    if( str[ j ] == ' ' ) {
      if( i < j ) strList += str.substr( i, j-i );
      i = j + 1;
    }

  if( i < j ) strList += str.substr( i, j-i );
}

// Output a list of strings with spaces between them

void printStringList( vector< string > &strList, ostream &strout )
{
  unsigned i, l = strList.size();

  for( i = 0; i < l; ++i ) {
    strout << strList[ i ];
    if( i < l - 1 ) strout << ' ';
  }
}

// Execute 'command' on a remote machine using 'remote_shell' (which can 
// include arguments) passing an argument list of 'arg_list'

PDSOCKET remoteCommand(const string hostName, const string userName,
		       const string command, const string remoteExecCmd,
		       const vector<string> &arg_list, int portFd)
{
    PDSOCKET ret = INVALID_PDSOCKET;
    unsigned i;


    // build the command line of the remote execution command we will execute
    // note that it must support the "-l" flag to specify a username, like rsh
    vector<string> remoteExecArgList;
    vector<string> tmp;
    string rsh;

    // first extract any arguments specified with the remote execution command
    appendParsedString(tmp, remoteExecCmd);
    assert(tmp.size() > 0);
    rsh = tmp[0];
    for (i=1; i < tmp.size(); i++)
      remoteExecArgList += tmp[i];

    // next add the host name and user name to the remote execution command
    remoteExecArgList += hostName;
    if( userName.length() > 0 ) {
      // add username specification
      //remoteExecArgList += (string("-l ") + userName);
      // That was bad, because (at least on i386-solaris-2.6 rsh "-l username"
      // behaves badly -- it must patternmatch against "-l" exactly
      remoteExecArgList += string("-l");
      remoteExecArgList += userName;
    }
    // add remote command and its arguments
    remoteExecArgList += command;
    for( i = 0; i < arg_list.size(); i++ )
      remoteExecArgList += arg_list[i];
    remoteExecArgList += "-l0";


    // execute the command
    PDSOCKET s = execCmd(rsh, remoteExecArgList );
    if( s != INVALID_PDSOCKET ) {
      // establish the connection
      ret = RPC_getConnect( portFd );
    }

    return ret;
}

//
// use rsh to get a remote process started.
//
// We do an rsh to start a process and them wait for the process 
// to do a connect. There is no guarantee that the process we are waiting for
// is the one that gets the connection. This can happen with paradyndPVM: the
// daemon that is started by a rshCommand will start other daemons, and one of 
// these daemons may get the connection that should be for the first daemon.
// Daemons should always get a connection before attempting to start other
// daemons.

PDSOCKET rshCommand(const string hostName, const string userName, 
		    const string command, const vector<string> &arg_list, int portFd)
{
  // ensure we know the user's desired rsh command
  const char* rshCmd = getenv( RSH_COMMAND_ENV );
  if( rshCmd == NULL ) {
    rshCmd = DEF_RSH_COMMAND;
  }

  return remoteCommand( hostName, userName, command, rshCmd, arg_list, portFd );
}

/*
 * 
 * "command" will be appended to the front of the arg list
 *
 * if arg_list == NULL, command is the only argument used
 */

PDSOCKET RPCprocessCreate(const string hostName, const string userName,
			  const string command, const string remote_shell,
			  const vector<string> &arg_list,
			  int portFd)
{
    PDSOCKET ret;

    if ((hostName == "") ||
	(hostName == "localhost") ||
	(getNetworkName(hostName) == getNetworkName()) ||
	(getNetworkAddr(hostName) == getNetworkAddr()) )
      ret = execCmd(command, arg_list);
    else if (remote_shell.length() > 0)
      ret = remoteCommand(hostName, userName, command, remote_shell, arg_list, portFd);
    else
      ret = rshCommand(hostName, userName, command, arg_list, portFd);

    return(ret);
}

PDSOCKET RPC_getConnect(PDSOCKET sock) {
  if (sock == INVALID_PDSOCKET)
    return INVALID_PDSOCKET;

  struct sockaddr cli_addr;
  size_t clilen = sizeof(cli_addr);
  errno = 0;
  PDSOCKET new_sock = P_accept (sock, (struct sockaddr *) &cli_addr, &clilen);

  if (new_sock == PDSOCKET_ERROR) {
    if (PDSOCKET_ERRNO == EMFILE) {
      cerr << "Cannot accept more connections:  Too many open files" << endl;
      cerr << "Please see your documentation for `ulimit'" << endl << flush;
    }
    return INVALID_PDSOCKET;
  }
  else {
    // note that we have consumed the "available data" from
    // the indicated socket
    unsigned int i;
    for( i = 0; i < rpcSockCallback.size(); i++ )
	{
      (*rpcSockCallback[i])( sock );
	}

    return new_sock;
  }
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
    temp[word_len] = '\0';
    arg += temp;
    delete temp;

    /* skip past consecutive blanks */
    while ((index < length) && (input[index] == BLANK))
      index++;
  } while (index < length);
  return true;
}

#if defined(i386_unknown_nt4_0)
//
// CreateSocketPair
//
// Creates a connected pair of TCP/IP sockets.  The
// intended use of the function is to mimic the
// socketpair call available from Berkeley sockets,
// so that a "child" process created by this process can
// inherit one endpoint of the socket pair and 
// thus be connected to the "parent" process.
//
// The sockets are distinguished as local and remote
// because we need to make sure that the socket handle
// intended to be kept at the parent process must not
// be inherited by the child.  (If it were inherited,
// the child couldn't tell when the parent closed
// the socket, because the child would inherit both
// endpoints of the connection and thus the connection
// would stay open even though the parent closed its end.)
//
bool
CreateSocketPair( PDSOCKET& localSock, PDSOCKET& remoteSock )
{
    PDSOCKET sockListen = INVALID_PDSOCKET;
    bool bCreated = false;
    

    // create sockets
    localSock = socket( AF_INET, SOCK_STREAM, PF_UNSPEC );
    remoteSock = INVALID_PDSOCKET;
    sockListen = socket( AF_INET, SOCK_STREAM, PF_UNSPEC );
    if( (sockListen != INVALID_PDSOCKET) && (localSock != INVALID_PDSOCKET) )
    {
        // connect sockets...

        // ...bind one of the sockets...
        struct hostent* pHostData = gethostbyname( "localhost" );
        assert( pHostData != NULL );

        SOCKADDR_IN sin;
        sin.sin_family = AF_INET;
        sin.sin_port = 0;            // dynamically assigned
        sin.sin_addr.S_un.S_addr = *(unsigned long*)pHostData->h_addr;
        if( bind( sockListen, (SOCKADDR*)&sin, sizeof(sin) ) != PDSOCKET_ERROR )
        {
            // ...connect the other to the bound socket
            sockaddr_in sain;
            int cbSain = sizeof( sain );
            if( getsockname( sockListen, (SOCKADDR*)&sain, &cbSain ) 
                        != PDSOCKET_ERROR )
            {
                if( listen( sockListen, 1 ) != PDSOCKET_ERROR )
                {
                    // issue a "deferred" connect for socket 0
                    sin.sin_port = sain.sin_port;
                    if( connect( localSock, (SOCKADDR*)&sin, sizeof(sin) )
                                != PDSOCKET_ERROR )
                    {
                        // accept the connection
                        remoteSock = accept( sockListen, NULL, NULL );
                        if( remoteSock != INVALID_PDSOCKET )
                        {
                            // verify the sockets are connected -
                            // socket 0 should now be writable
                            fd_set wrset;
                            struct timeval tv;

                            tv.tv_sec = 0;
                            tv.tv_usec = 0;
                            FD_ZERO( &wrset );
                            FD_SET( localSock, &wrset );
                            if( select( 0, NULL, &wrset, NULL, &tv ) == 1 )
                            {
                                bCreated = true;
                            }
                        }
                    }
                }
            }
        }
    }

    if( bCreated )
    {
        // make sure that the remote endpoint handle
        // is not inheritable
        HANDLE hDup;
        if( DuplicateHandle( GetCurrentProcess(),
                (HANDLE)localSock,
                GetCurrentProcess(),
                &hDup,
                0,
                FALSE,
                DUPLICATE_SAME_ACCESS ) )
        {
            // we made the duplication,
            // so close the inheritable handle
            // and return the noninheritable one
            CLOSEPDSOCKET( localSock );
            localSock = (SOCKET)hDup;
        }
        else
        {
            bCreated = false;
        }
    }

    if( !bCreated )
    {
        CLOSEPDSOCKET( localSock );
        localSock = INVALID_PDSOCKET;
        CLOSEPDSOCKET( remoteSock );
        remoteSock = INVALID_PDSOCKET;
    }

    CLOSEPDSOCKET( sockListen );

    return bCreated;
}
#endif // defined(i386_unknown_nt4_0)

// get the current (localhost) machine name, e.g. "grilled"
const string getHostName()
{
    char hostname[256];
    int i;

    if ((i=gethostname(hostname,sizeof(hostname))) != 0) {
#if defined(i386_unknown_nt4_0)
        i=WSAGetLastError();
#endif
        fprintf(stderr,"Failed gethostname(): %d\n", i);
        return string("");
    }
    
    //cerr << "getHostName=" << hostname << endl;
    return string(hostname);
}

// get the network domain name from the given hostname (default=localhost)
// e.g. "grilled.cs.wisc.edu" -> "cs.wisc.edu"
const string getDomainName (const string hostname)
{
    unsigned index=0;

    string networkname = hostname;
    if (hostname.length() == 0) networkname = getNetworkName();

    while (index < networkname.length() && networkname[index]!='.') index++;
    if (index == networkname.length()) {
        cerr << "Failed to determine domain: hostname=<" << hostname 
             << ">" << endl;
        return ("");
    } else {
        const string simplename = networkname.substr(0,index);
        const string domain = networkname.substr(index+1,networkname.length());
        if (simplename.regexEquiv("[0-9]*",false)) {
            cerr << "Got invalid simplename: " << simplename << endl;
            return ("");
        } else {
            //cerr << "getDomainName=" << domain << endl;
            return (domain);
        }
    }
}

// get the fully-qualified network name for given hostname (default=localhost)
// e.g. "grilled" -> "grilled.cs.wisc.edu"
const string getNetworkName (const string hostname)
{
    struct hostent *hp;

    char name[256];
    strcpy(name,hostname.c_str());

    if (!name[0]) { // find this machine's hostname
        const string thishostname=getHostName();
        strcpy(name,thishostname.c_str());
    }

    hp = gethostbyname(name);
    if (hp == NULL) {
        cerr << "Host information not found for " << name << endl;
        return string("");
    }

    string networkname = string(hp->h_name);

    // check that networkname is fully-qualified with domain information
    if (getDomainName(networkname) == "") 
        networkname = getNetworkAddr(networkname); // default to IP address

    //cerr << "getNetworkName=" << networkname << endl;
    return (networkname);
}

// get the network IP address for given hostname (default=localhost)
// e.g. "grilled" -> "128.105.166.40"
const string getNetworkAddr (const string hostname)
{
    struct hostent *hp;

    char name[256];
    strcpy(name,hostname.c_str());

    if (!name[0]) { // find this machine's hostname
        const string thishostname=getHostName();
        strcpy(name,thishostname.c_str());
    }

    hp = gethostbyname(name);
    if (hp == NULL) {
        cerr << "Host information not found for " << name << endl;
        return string("");
    }

    struct in_addr in;
    memcpy(&in.s_addr, *(hp->h_addr_list), sizeof (in.s_addr));

    //cerr << "getNetworkAddr=" << inet_ntoa(in) << endl;
    return string(inet_ntoa(in));
}

