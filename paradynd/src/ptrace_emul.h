/* 
 * ptrace_emul.h:  Header file for ptrace emulation stuff.
 *
 * $Log: ptrace_emul.h,v $
 * Revision 1.1  1994/01/27 20:31:40  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.1  1993/08/24  21:58:02  jcargill
 * Initial revision
 *
 */


/*
 * Ptrace emulation section.  We send packets of "Modify memory"
 * requests to the instrumentation controller.  The format of a
 * request is as follows:
 *
 * [ request ] - modify memory request.  Currently the only request.
 * [ req id  ] - Uniq id for purposes of Acknowledgement.
 * [ node id ] - Node id:  0-(partition_size-1) for unique node.  
 *                         0xffffffff to broadcast to all nodes in partition
 * [ address ] - Memory address to modify
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
  u_int request;
  u_int req_id;
  u_int dest;			/* target nodes */
  char *address;		/* memory address */
  u_int length;
} ptraceReqHeader;


/*
 * Defines for requests to ptrace emulation code in timeslice
 * handlers.  Currently there is only one.
 */
#define MODIFY_TEXT			1
#define PTRACE_REQUESTS_AVAILABLE	2
#define HANDLER_READY			3
#define PTRACE_REQUESTS_DONE		4

