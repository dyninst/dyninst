/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/dyninstAPI/src/util.C,v 1.9 1996/06/01 00:01:48 tamches Exp $";
#endif

/*
 * util.C - support functions.
 *
 * $Log: util.C,v $
 * Revision 1.9  1996/06/01 00:01:48  tamches
 * addrHash replaced by addrHash16
 *
 * Revision 1.8  1996/05/11 23:16:17  tamches
 * added addrHash
 *
 * Revision 1.7  1995/02/16 08:54:28  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.6  1995/02/16  08:35:03  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.5  1994/11/02  11:18:54  markc
 * Remove old malloc wrappers.
 *
 * Revision 1.4  1994/09/22  02:27:37  markc
 * Changed signature to intComp
 *
 * Revision 1.3  1994/07/28  22:40:49  krisna
 * changed definitions/declarations of xalloc functions to conform to alloc.
 *
 * Revision 1.2  1994/06/22  01:43:19  markc
 * Removed warnings.  Changed bcopy in inst-sparc.C to memcpy.  Changed process.C
 * reference to proc->status to use proc->heap->status.
 *
 * Revision 1.1  1994/01/27  20:31:48  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.2  1993/08/16  22:01:22  hollings
 * added include for errno.h.
 *
 * Revision 1.1  1993/03/19  22:45:45  hollings
 * Initial revision
 *
 *
 */

#include "util/h/headers.h"
#include "util.h"

unsigned addrHash16(const unsigned &iaddr) {
   // inspired by hashs of string class
   // NOTE: this particular hash fn assumes that "addr" is divisible by 16

   unsigned addr = iaddr >> 4;
      // since the address is divisible by 16, the low 4 bits
      // are always the same for each address and hence contribute
      // nothing to an even hash distribution.  Hence, we zap
      // those bits right now.

   unsigned result = 5381;

   unsigned accumulator = addr;
   while (accumulator > 0) {
      // We use 3 bits at a time from the address
      result = (result << 4) + result + (accumulator & 0x07);
      accumulator >>= 3;
   }

   return result;
}

#ifdef notdef
void
log_printf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsprintf(errorLine, fmt, ap);
    va_end(ap);
    logLine(errorLine);
    // printf("%s", log_buffer);
}
#endif

void
pd_log_perror(const char* msg) {
    sprintf(errorLine, "%s: %s\n", msg, sys_errlist[errno]);
    logLine(errorLine);
    // fprintf(stderr, "%s", log_buffer);
}


