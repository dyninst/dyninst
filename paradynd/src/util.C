/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/Attic/util.C,v 1.5 1994/11/02 11:18:54 markc Exp $";
#endif

/*
 * util.C - support functions.
 *
 * $Log: util.C,v $
 * Revision 1.5  1994/11/02 11:18:54  markc
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

#include "util/h/kludges.h"
#include "util.h"

/* used for qsort */
int intComp(const void *i, const void *j)
{
    return (*((int*)i) - *((int*)j) );
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
    extern char* sys_errlist[];
    sprintf(errorLine, "%s: %s\n", msg, sys_errlist[errno]);
    logLine(errorLine);
    // fprintf(stderr, "%s", log_buffer);
}


