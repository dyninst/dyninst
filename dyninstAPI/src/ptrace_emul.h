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

#ifndef PTRACE_EMUL
#define PTRACE_EMUL

/* 
 * ptrace_emul.h:  Header file for ptrace emulation stuff.
 *
 * $Log: ptrace_emul.h,v $
 * Revision 1.9  1996/08/16 21:19:43  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.8  1995/02/16 08:54:09  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.7  1995/02/16  08:34:41  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
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

