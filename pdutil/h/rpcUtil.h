
#ifndef RPC_UTIL
#define RPC_UTIL

#include <rpc/xdr.h>

#define xdr_Boolean 	xdr_bool
typedef XDR *XDRptr;
typedef int (*xdrIOFunc)(int handle, char *buf, unsigned int len);

typedef int Boolean;
typedef char *String;

//
// Functions common to server and client side.
//
class XDRrpc {
  public:
    XDRrpc(char *m, char *u, char *p, xdrIOFunc, xdrIOFunc);
    XDRrpc(int fd, xdrIOFunc readRoutine, xdrIOFunc writeRoutine);
    XDR *__xdrs__;
    int fd;
    int pid;		// pid of child;
};

class THREADrpc {
  public:
    THREADrpc(int tid);
  protected:
    int tid;
    // this are only to be used by implmentors of thread RPCs.
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
};

//
// server side routines that are transport independent.
//
class RPCServer {
};

extern int xdr_String(XDR*, String*);
extern int RPCprocessCreate(int *pid, char *hostName, char *userName, char *commandLine);

#endif
