/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/Attic/sym-cm5.C,v 1.2 1994/06/27 18:57:13 hollings Exp $";
#endif

/*
 * sym-cm5.C - hanle CM5 binaries (two programs in one file).
 *
 * $Log: sym-cm5.C,v $
 * Revision 1.2  1994/06/27 18:57:13  hollings
 * removed printfs.  Now use logLine so it works in the remote case.
 * added internalMetric class.
 * added extra paramter to metric info for aggregation.
 *
 * Revision 1.1  1994/01/27  20:31:44  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.2  1993/08/16  22:02:30  hollings
 * added include of fcntl.h.
 *
 * Revision 1.1  1993/03/19  22:45:45  hollings
 * Initial revision
 *
 *
 */
#include <stdlib.h>
#include <sys/types.h>
#include <cmsys/cm_a.out.h>
#include <sys/file.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>

/*
 * Code to find the offset of the node executable in the joined executable.
 *
 */
int findNodeOffset(char *file, int offset)
{
    int fd;
    int ret;
    int headerOffset;
    struct stat buf;
    cm_par_id_t header;

    if (!offset) return(0);

    if (stat(file, &buf)) {
	logLine("error stating file\n");
	abort();
    }

    fd = open(file, O_RDONLY, 0);
    assert(fd > 0);

    headerOffset = buf.st_size - sizeof(cm_par_id_t);

    lseek(fd, headerOffset, SEEK_SET);

    ret = read(fd, (char *) &header, sizeof(header));
    if (ret != sizeof(header)) {
	logLine("Not a CM Executable\n");
	return(0);
    } else if (header.magic != CM_PAR_MAGIC) {
	logLine("Not a CM Executable\n");
	return(0);
    }
    return(header.par_offset);
}
