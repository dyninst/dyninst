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

#ifndef RPC_UTIL
#define RPC_UTIL

/*
 * $Log: rpcUtil.h,v $
 * Revision 1.33  1997/01/21 20:09:49  mjrg
 * Added support for unix domain sockets.
 * Added getHostName function
 *
 * Revision 1.32  1997/01/16 20:51:40  tamches
 * removed RPC_undo_arg_list
 *
 * Revision 1.31  1996/08/16 21:30:42  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.30  1996/05/31 23:41:08  tamches
 * removed pid from XDRrpc
 *
 * Revision 1.29  1995/11/22 00:05:54  mjrg
 * Updates for paradyndPVM on solaris
 * Fixed problem with wrong daemon getting connection to paradyn
 * Removed -f and -t arguments to paradyn
 * Added cleanUpAndExit to clean up and exit from pvm before we exit paradynd
 * Fixed bug in my previous commit
 *
 * Revision 1.28  1995/05/18  11:11:35  markc
 * add P_xdrrec_eof
 *
 * Revision 1.27  1995/02/16  09:27:11  markc
 * Modified code to remove compiler warnings.
 * Added #defines to simplify inlining.
 * Cleaned up Object file classes.
 *
 * Revision 1.26  1994/11/11  06:59:09  markc
 * Added additional argument to RPC_make_arg_list and RPC_undo_arg_list to
 * support remote executition for paradyndPVM.
 *
 * Revision 1.25  1994/11/01  16:07:33  markc
 * Added Object classes that provide os independent symbol tables.
 * Added stl-like container classes with iterators.
 *
 * Revision 1.24  1994/10/12  20:22:08  krisna
 * hpux update
 *
 * Revision 1.23  1994/09/22  03:18:05  markc
 * changes to remove compiler warnings
 * changed pid passed to RPCprocessCreate
 *
 */

#include "util/h/headers.h"
#include "util/h/String.h"
#include "util/h/Vector.h"

// Boolean defined for igen -- xdr_bool uses an int, which clashes with gcc
// typedef bool Boolean;

extern bool RPC_readReady (int fd, int timeout=0);

//
// Functions common to server and client side.
//
class XDRrpc {
public:
  XDRrpc(const string &machine, const string &user, const string &program,
	 xdr_rd_func r, xdr_wr_func w,
	 const vector<string> &arg_list, const bool nblock, const wellKnownPortFd);
  XDRrpc(const int use_fd, xdr_rd_func readRoutine, xdr_wr_func writeRoutine,
	 const bool nblock);
  XDRrpc(int family, int port, int type, const string machine,
	 xdr_rd_func readFunc, xdr_wr_func writeFunc, const bool block);
  ~XDRrpc();
  void setNonBlock() { if (fd >= 0) fcntl (fd, F_SETFL, O_NONBLOCK); }
  void closeConnect() {if (fd >= 0) close(fd); fd = -1; }
  int get_fd() const { return fd; }
  int readReady(const int timeout=0) { return RPC_readReady (fd, timeout); }

  void setDirEncode() {xdrs->x_op = XDR_ENCODE;}
  void setDirDecode() {xdrs->x_op = XDR_DECODE;}
  XDR *net_obj() { return xdrs;}
  bool opened() const { return (xdrs && (fd >= 0));}

 private:
  // Since we haven't defined these, private makes sure they're not used. -ari
  XDRrpc(const XDRrpc &);
  XDRrpc &operator=(const XDRrpc &);

  XDR *xdrs;
  int fd;
};

//
// common routines that are transport independent.
//
class RPCBase {
public:
  RPCBase(const int st=0, const int v=0) { err_state = st; versionVerifyDone = v;}
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

class THREADrpc {
public:
  THREADrpc(const unsigned t) { tid = t; }
  // ~THREADrpc() { }
  void setTid(const unsigned id) { tid = id; }
  unsigned getTid() const { return tid;}

  // see not on requestingThread, the use of this may be unsafe
  unsigned getRequestingThread() const { return requestingThread; }
  void setRequestingThread(const unsigned t) { requestingThread = t;}
  unsigned net_obj() const { return tid;}

 private:
  unsigned tid;
  // these are only to be used by implmentors of thread RPCs.
  //   the value is only valid during a thread RPC.
  unsigned requestingThread;
};

extern int RPC_setup_socket (int &sfd,   // return file descriptor
			     const int family, // AF_INET ...
			     const int type);   // SOCK_STREAM ...

// setup a unix domain socket
extern bool RPC_setup_socket_un(int &sfd, const char *path);


extern bool_t xdr_string_pd(XDR*, string*);
extern bool_t xdr_Boolean(XDR*, bool*);   

inline bool_t P_xdr_string_pd(XDR *x, string *s) {
  return (xdr_string_pd(x, s));}
inline bool_t P_xdr_Boolean(XDR *x, bool *b) {
  return (xdr_Boolean(x, b));}

extern int RPCprocessCreate(const string hostName, const string userName,
			    const string commandLine,
			    const vector<string> &arg_list,
			    int wellKnownPort = 0,
			    const bool useRexec=false);

extern bool RPC_make_arg_list (vector<string> &list, const int port, 
			       const int flag, const int firstPVM,
			       const string machineName, const bool useMachine);

extern int RPC_getConnect (const int fd);

extern bool RPCgetArg(vector<string> &ret, const char *input);

extern double timing_loop(const unsigned TRIES=1,
			  const unsigned LOOP_LIMIT=100000);

extern string getHostName();

#endif
