/* 
 * ptrace_emul.h:  Header file for ptrace emulation stuff.
 *
 * $Log: ptrace_emul.h,v $
 * Revision 1.3  1994/09/22 02:24:13  markc
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


#include <sys/types.h>
#include <sys/ptrace.h>

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

typedef struct _ptraceReqHeader {
  enum ptracereq request;
  u_int pid;			/* pid of CM process on CP */
  u_int nodeNum;		/* target nodes (0xffffffff = all) */
  char *addr;
  int   data;
  char *addr2;
} ptraceReqHeader;


/* Maximum DATA length for a ptrace request */
#define MAX_PTRACE_LENGTH	1000



