
#ifndef RPC_UTIL
#define RPC_UTIL

/*
 * $Log: rpcUtil.h,v $
 * Revision 1.28  1995/05/18 11:11:35  markc
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
 * Revision 1.22  1994/08/17  18:23:53  markc
 * Added new classes: Cstring KeyList, KList
 * Added new function: RPCgetArg
 * Changed typedefs in machineType.h to #defines
 *
 * Revision 1.21  1994/07/28  22:21:26  krisna
 * changed declaration of xdrIOFunc to conform to prototypes
 *
 * Revision 1.20  1994/07/19  18:29:59  markc
 * Made machineName default to zero as last parameter to RPC_make_arg_list.
 * Added typecast to malloc call in RPC_make_arg_list.
 *
 * Revision 1.19  1994/07/18  19:09:10  hollings
 * added extra arg to RPC_make_arg_list.
 *
 * Revision 1.18  1994/06/02  23:35:15  markc
 * Modified RPCUser class to support improved error checking for igen.
 *
 * Revision 1.17  1994/04/21  23:23:44  hollings
 * removed paradynd name from make args function.
 *
 * Revision 1.16  1994/04/06  22:45:24  markc
 * Cleaned up rpcUtil.h.  Moved include files to rpcUtil.C where they belonged.
 *
 * Revision 1.15  1994/03/31  22:59:04  hollings
 * added well known port as a paramter to xdrRPC constructor.
 *
 * Revision 1.14  1994/03/25  16:07:31  markc
 * Added option to specify timeout on readReady.
 *
 * Revision 1.13  1994/03/20  01:45:23  markc
 * Changed xdr_Boolean to char rather than int to reflect Boolean change.
 *
 * Revision 1.12  1994/03/11  21:01:24  hollings
 * Changed Boolean from int to char to match X11 convention.
 *
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
  XDRrpc(const string m, const string u, const string p, xdr_rd_func r, xdr_wr_func w,
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
  int getPid() const { return pid; }
  void setPid(const int to) { pid = to;  }
  void setDirEncode() {xdrs->x_op = XDR_ENCODE;}
  void setDirDecode() {xdrs->x_op = XDR_DECODE;}
  XDR *net_obj() { return xdrs;}
  bool opened() const { return (xdrs && (fd >= 0));}

 private:
  //XDR *getXdrs() { return xdrs; }
  XDR *xdrs;
  int fd;
  int pid;		// pid of child;
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


extern bool_t xdr_string_pd(XDR*, string*);
extern bool_t xdr_Boolean(XDR*, bool*);   

inline bool_t P_xdr_string_pd(XDR *x, string *s) {
  return (xdr_string_pd(x, s));}
inline bool_t P_xdr_Boolean(XDR *x, bool *b) {
  return (xdr_Boolean(x, b));}

extern int RPCprocessCreate(int &pid, const string hostName, const string userName,
			    const string commandLine,
			    const vector<string> &arg_list,
			    int wellKnownPort = 0,
			    const bool useRexec=false);

extern bool RPC_make_arg_list (vector<string> &list, const int family,
			       const int type, const int port, 
			       const int flag, const int firstPVM,
			       const string machineName, const bool useMachine);

extern bool
RPC_undo_arg_list (string& flavor, int argc, char **arg_list, string &machine,
		   int &family, int &type, int &well_known_socket, int &flag,
		   int &firstPVM);

extern int RPC_getConnect (const int fd);

extern bool RPCgetArg(vector<string> &ret, const char *input);

extern double timing_loop(const unsigned TRIES=1,
			  const unsigned LOOP_LIMIT=100000);

#endif
