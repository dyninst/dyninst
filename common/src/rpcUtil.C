
//
// This file defines a set of utility routines for RPC services.
//
//
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>

extern "C" {
#include <rpc/types.h>
}
#include "util/h/rpcUtil.h"

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
    assert(ret == len);
    return (ret);
}

//
// prepare for RPC's to be done/received on the passed fd.
//
XDRrpc::XDRrpc(int f, xdrIOFunc readRoutine, xdrIOFunc writeRoutine)
{
    fd = f;
    __xdrs__ = (XDR *) malloc(sizeof(XDR));
    if (!readRoutine) readRoutine = RPCdefaultXDRRead;
    if (!writeRoutine) writeRoutine = RPCdefaultXDRWrite;
    (void) xdrrec_create(__xdrs__, 0, 0, (char *) fd, readRoutine,writeRoutine);
}


//
// prepare for RPC's to be done/received on the passed fd.
//
XDRrpc::XDRrpc(char *machine,
	       char *user,
	       char *program,
	       xdrIOFunc readRoutine, 
	       xdrIOFunc writeRoutine)
{
    fd = RPCprocessCreate(&pid, machine, user, program);
    if (fd >= 0) {
	__xdrs__ = (XDR *) malloc(sizeof(XDR));
	if (!readRoutine) readRoutine = RPCdefaultXDRRead;
	if (!writeRoutine) writeRoutine = RPCdefaultXDRWrite;
	(void) xdrrec_create(__xdrs__, 0, 0, (char *) fd, 
		readRoutine, writeRoutine);
    } else {
	__xdrs__ = NULL;
	fd = -1;
    }
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

    if (xdrs->x_op == XDR_ENCODE) {
	len = strlen(*str)+1;
    } else {
	*str = NULL;
    }
    // should we have a better max length ???. 
    xdr_bytes(xdrs, str, &len, 65536*32768);
    return(TRUE);
}

int RPCprocessCreate(int *pid, char *hostName, char *userName, char *command)
{
    int ret;
    int sv[2];
    int execlERROR;

    if (!hostName || !strcmp(hostName, "") || !strcmp(hostName, "localhost")) {
	ret = socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
	if (ret) return(ret);
	execlERROR = 0;
	*pid = vfork();
	if (*pid == 0) {
	    close(sv[0]);
	    dup2(sv[1], 0);
	    execl(command, command);
	    execlERROR = errno;
	    _exit(-1);
	} else if (*pid > 0 && !execlERROR) {
	    close(sv[1]);
	    return(sv[0]);
	} else {
	    return(-1);
	}
    } else {
	// need to rsh to machine and setup io path.
	printf("remote starts not implemented\n");
	exit(-1);
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
