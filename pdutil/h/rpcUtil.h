
#ifndef RPC_UTIL
#define RPC_UTIL

/*
 * $Log: rpcUtil.h,v $
 * Revision 1.21  1994/07/28 22:21:26  krisna
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

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

extern "C" {
#include <rpc/types.h>
#include <rpc/xdr.h>
}

#define xdr_Boolean xdr_char
typedef XDR *XDRptr;
typedef int (*xdrIOFunc)(const void *, char *, int);

typedef char Boolean;
typedef char *String;

//
// Functions common to server and client side.
//
class XDRrpc {
  public:
    XDRrpc(char *m, char *u, char *p, xdrIOFunc, xdrIOFunc, 
	   char **arg_list=0, int nblock=0, int wellKnownPortFd = 0);
    XDRrpc(int fd, xdrIOFunc readRoutine, xdrIOFunc writeRoutine, int nblock=0);
    XDRrpc(int family, int port, int type, char *machine, xdrIOFunc readFunc,
	   xdrIOFunc writeFunc, int nblock=0);
    virtual ~XDRrpc();
    void closeConnect() { if (fd >= 0) close(fd); fd = -1;}
    void setNonBlock();
    int get_fd() { return fd;}
    int readReady(int timeout=0);
    XDR *__xdrs__;
    int fd;
    int pid;		// pid of child;
    static int __wellKnownPortFd__;
    // should the fd be closed by a destructor ??
};

class THREADrpc {
  public:
    THREADrpc(int tid);
    void setNonBlock() { ; }
    void setTid(int id) { tid = id; }
  protected:
    int tid;
    // these are only to be used by implmentors of thread RPCs.
    //   the value is only valid during a thread RPC.
    unsigned int requestingThread;
    unsigned int getRequestingThread()	{ return requestingThread; }
};


//
// client side common routines that are transport independent.
//
class RPCUser {
  public:
    void awaitResponce(int tag);
    void verifyProtocolAndVersion();
    RPCUser(int st=0); 
    int get_err_state() { return err_state;}
    int clear_err_state() {err_state = 0;}
    int did_error_occur() {return (err_state != 0);}
  protected:
    int err_state;
};

//
// server side routines that are transport independent.
//
class RPCServer {
  public:
    int __versionVerifyDone__;
    RPCServer(int st=0);
    int get_err_state() { return err_state;}
    int clear_err_state() {err_state = 0;}
    int did_error_occur() {return (err_state != 0);}
  protected:
    int err_state;
};

extern int RPC_readReady (int fd, int timeout=0);
extern int RPC_setup_socket (int *sfd,   // return file descriptor
			     int family, // AF_INET ...
			     int type);   // SOCK_STREAM ...
extern int xdr_String(XDR*, String*);
extern int RPCprocessCreate(int *pid, char *hostName, char *userName,
			    char *commandLine, char **arg_list = 0,
			    int wellKnownPort = 0);

extern char **RPC_make_arg_list (int family, int type, 
				 int port, 
				 int flag,
				 char *machienName = (char*) 0);

extern int 
RPC_undo_arg_list (int argc, char **arg_list, char **machine, int &family,
		   int &type, int &well_known_socket, int &flag);
extern int RPC_getConnect (int fd);

#endif
