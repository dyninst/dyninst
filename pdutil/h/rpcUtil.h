/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

#ifndef RPC_UTIL
#define RPC_UTIL

// $Id: rpcUtil.h,v 1.58 2005/12/12 16:37:30 gquinn Exp $

#include "common/h/headers.h"
#include "pdsocket.h"

#include "xdr_send_recv.h" // P_xdr_send() and P_xdr_recv() routines

/* define following variables are needed for linux platform as they are
   missed in /usr/include/sys/file.h                                     */
#if defined(i386_unknown_linux2_0) || defined(ia64_unknown_linux2_4) || defined(ppc64_unknown_linux2_4) || defined(x86_64_unknown_linux2_4)
  #define FSYNC O_FSYNC
#endif

// Boolean defined for igen -- xdr_bool uses an int, which clashes with gcc
// typedef bool Boolean;

extern bool RPC_readReady (PDSOCKET sock, int timeout=0);

//
// Functions common to server and client side.
//
class XDRrpc {
public:
  XDRrpc(const pdstring &machine, const pdstring &user, const pdstring &program,
	 const pdstring &remote_shell, xdr_rd_func r, xdr_wr_func w,
	 const pdvector<pdstring> &arg_list, const int nblock, PDSOCKET wellKnownSock);
  XDRrpc(PDSOCKET use_sock, xdr_rd_func readRoutine, xdr_wr_func writeRoutine,
	 const int nblock);
  XDRrpc(int family, int port, int type, const pdstring machine,
	 xdr_rd_func readRoutine, xdr_wr_func writeRoutine,
         const int nblock);
  ~XDRrpc();
  // This function does work on Windows NT. Since it is not being used
  // anywhere, I'm commenting it out -- mjrg
  //void setNonBlock() { if (fd >= 0) fcntl (fd, F_SETFL, O_NONBLOCK); }
  void closeConnect() {if (sock != PDSOCKET_ERROR) CLOSEPDSOCKET(sock); sock = INVALID_PDSOCKET; }
  PDSOCKET get_sock() const { return sock; }
  int readReady(const int timeout=0) { return RPC_readReady (sock, timeout); }

  void setDirEncode() {xdrs->x_op = XDR_ENCODE;}
  void setDirDecode() {xdrs->x_op = XDR_DECODE;}
  XDR *net_obj() { return xdrs;}
  bool opened() const { return (xdrs && (sock != PDSOCKET_ERROR));}

 private:
  // Since we haven't defined these, private makes sure they're not used. -ari
  XDRrpc(const XDRrpc &);
  XDRrpc &operator=(const XDRrpc &);

  XDR *xdrs;
  PDSOCKET sock;
};

//
// common routines that are transport independent.
//
class RPCBase {
public:
  RPCBase(const int st=0, bool v=0) { err_state = st; versionVerifyDone = v;}
  // ~RPCBase() { }
  int get_err_state() const { return err_state;}
  void clear_err_state() {err_state = 0;}
  int did_error_occur() const {return (err_state != 0);}
  bool getVersionVerifyDone() const { return versionVerifyDone;}
  void setVersionVerifyDone() { versionVerifyDone = true;}
  void set_err_state(const int s) { err_state = s;}

 private:
  // Since we haven't defined these, private makes sure they're not used. -ari
  RPCBase(const RPCBase &);
  RPCBase &operator=(const RPCBase &);

  bool versionVerifyDone;
  int err_state;
};

extern int RPC_setup_socket (PDSOCKET &sfd,   // return file descriptor
			     const int family, // AF_INET ...
			     const int type);   // SOCK_STREAM ...

#if !defined(i386_unknown_nt4_0)
// setup a unix domain socket
extern bool RPC_setup_socket_un(PDSOCKET &sfd, const char *path);
#endif // !defined(i386_unknown_nt4_0)

extern PDSOCKET RPCprocessCreate(const pdstring hostName, const pdstring userName,
			    const pdstring commandLine, const pdstring remote_shell,
			    const pdvector<pdstring> &arg_list,
			    int wellKnownPort = 0);

#if !defined(i386_unknown_nt4_0)
extern bool RPC_make_arg_list (pdvector<pdstring> &list, const int port, 
			       const int termWin_port, 
			       const int flag, const int firstPVM,
			       const pdstring machineName, const bool useMachine);
#else
extern bool RPC_make_arg_list (pdvector<pdstring> &list, const int port, 
			       const int flag, const int firstPVM,
			       const pdstring machineName, const bool useMachine);
#endif

extern PDSOCKET RPC_getConnect (PDSOCKET fd);

extern bool RPCgetArg(pdvector<pdstring> &ret, const char *input);

extern double timing_loop(const unsigned TRIES=1,
			  const unsigned LOOP_LIMIT=100000);

extern const pdstring getHostName();                            // e.g. "foo"
extern const pdstring getDomainName(const pdstring hostname="");  // "bar.net"
extern const pdstring getNetworkName(const pdstring hostname=""); // "foo.bar.net"
extern const pdstring getNetworkAddr(const pdstring hostname=""); // "127.0.0.1"

extern const char *getRshCommand();

class rpcBuffer {
  public:
    PDSOCKET fd;
    char *buf;
    int len;
};

// a vector of callback functions for reads and accepts, needed to support
// correct interation between XDR and our thread library
typedef void (*RPCSockCallbackFunc)( PDSOCKET );
extern pdvector<RPCSockCallbackFunc> rpcSockCallback;

#endif
