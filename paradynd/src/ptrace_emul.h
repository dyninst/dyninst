
#ifndef PTRACE_EMUL
#define PTRACE_EMUL

/* 
 * ptrace_emul.h:  Header file for ptrace emulation stuff.
 *
 * $Log: ptrace_emul.h,v $
 * Revision 1.7  1995/02/16 08:34:41  markc
 * Changed igen interfaces to use strings/vectors rather than char*/igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.6  1995/02/10  22:36:22  jcargill
 * Removed include of util kludges
 *
 * Revision 1.5  1994/11/02  11:15:58  markc
 * Put our "PTRACE" defines here.
 *
 * Revision 1.4  1994/10/13  07:24:59  krisna
 * solaris porting and updates
 *
 * Revision 1.3  1994/09/22  02:24:13  markc
 * changed types to agree with ptrace signature
 *
 * Revision 1.2  1994/07/14  14:26:09  jcargill
 * Changes to ptraceReq header structure to accomodate new node ptrace
 *
 * Revision 1.1  1994/01/27  20:31:40  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.1  1993/08/24  21:58:02  jcargill
 * Initial revision
 *
 */

/*
 * Node-Ptrace request forwarding protocol.  We send ptrace requests
 * to the nodes on the CM-5 for execution using the ptrace function in
 * the node-kernel.  The format of a request is as follows:
 *
 * [ request ] - modify memory request.  Currently the only request.
 * [ req id  ] - Uniq id for purposes of Acknowledgement.     NOTE: OUTDATED
 * [ node id ] - Node id:  0-(partition_size-1) for unique node.  
 *                         0xffffffff to broadcast to all nodes in partition
 * [ addr ]    - Memory address to modify; same as addr argument to ptrace
 * [ length  ] - Number of words of data to write
 * [ data    ]
 * [ ...     ]
 * [ data    ]
 *
 * Everything between brackets above ([...]) is one word in length.
 * Since the instrumentation controller is running on the front-end
 * (so it can really use ptrace to control the application), we can
 * assume no byte-order problems.
 *
 *
 */

#include "util/h/headers.h"

typedef struct _ptraceReqHeader {
  int request;
  u_int pid;			/* pid of CM process on CP */
  u_int nodeNum;		/* target nodes (0xffffffff = all) */
  char *addr;
  int   data;
  char *addr2;
} ptraceReqHeader;


/* Maximum DATA length for a ptrace request */
#define MAX_PTRACE_LENGTH	1000

/* 
 * This define really shouldn't be here, but then it really shouldn't
 * be in instP.h, either.  Where should it go?  XXX 
 */
#define PTRACE_INTERRUPT        PTRACE_26
#define PTRACE_STATUS		PTRACE_27
#define PTRACE_SNARFBUFFER	PTRACE_28

#endif

