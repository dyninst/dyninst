/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/dyninstAPI/src/util.C,v 1.2 1994/06/22 01:43:19 markc Exp $";
#endif

/*
 * util.C - support functions.
 *
 * $Log: util.C,v $
 * Revision 1.2  1994/06/22 01:43:19  markc
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
#include <stdlib.h>
#include <errno.h>

#include "util.h"

extern "C" {
  void perror(char *s);
}

void *xmalloc(int size)
{
    void *ret;

    ret = malloc(size);
    if (!ret) {
	perror("malloc");
	exit(-1);
    }
    return(ret);
}

void *xcalloc(int size, int count)
{
    void *ret;

    ret = calloc(size, count);
    if (!ret) {
	perror("calloc");
	exit(-1);
    }
    return(ret);
}

void *xrealloc(void *ptr, int size)
{
    void *ret;

    ret = realloc(ptr, size);
    if (!ret) {
	perror("realloc");
	exit(-1);
    }
    return(ret);
}
