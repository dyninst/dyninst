
/*
 * Inst functions that are common to both sp and pe on CM-5.
 *
 * Used for:
 *   CMMP_receive_block
 *   CMMP_send_block
 *   CMMP_receive_async
 *   CMMP_send_async
 *   CMMP_send_noblock
 *
 * $Log: RTtags.c,v $
 * Revision 1.3  1994/07/14 23:36:07  hollings
 * added extra arg to generateTrace.
 *
 * Revision 1.2  1994/06/27  21:30:22  rbi
 * Parameter change
 *
 * Revision 1.1  1994/02/02  00:46:13  hollings
 * Changes to make it compile with the new tree.
 *
 * Revision 1.4  1993/10/19  15:29:58  hollings
 * new simpler primitives.
 *
 * Revision 1.3  1993/10/01  18:15:53  hollings
 * Added filtering and resource discovery.
 *
 * Revision 1.2  1993/09/02  22:07:31  hollings
 * new include format.
 *
 * Revision 1.1  1993/07/02  21:49:35  hollings
 * Initial revision
 *
 *
 */
#include <stdio.h>
#include <memory.h>

#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"

#define	dest_src	arg1
#define	tag		arg2
#define addr		arg3
#define element_size	arg4
#define element_stride	arg5
#define element_count	arg6

int DYNINSTtagCount;
int DYNINSTtagLimit = 1000;
int DYNINSTtags[1000];

void DYNINSTrecordTag(int tag)
{
    int i;

    for (i=0; i < DYNINSTtagCount; i++) {
	if (DYNINSTtags[i] == tag) return;
    }
    if (DYNINSTtagCount == DYNINSTtagLimit) abort();
    DYNINSTtags[DYNINSTtagCount++] = tag;
}

void DYNINSTreportNewTags()
{
    int i;
    static int lastTagCount;
    struct _newresource newRes;

    for (i=lastTagCount; i < DYNINSTtagCount; i++) {
	memset(&newRes, '\0', sizeof(newRes));
	sprintf(newRes.name, "SyncObject/MsgTag/%d", DYNINSTtags[i]);
	strcpy(newRes.abstraction, "BASE");
	DYNINSTgenerateTraceRecord(0, TR_NEW_RESOURCE, 
	    sizeof(struct _newresource), &newRes, 1);
    }
    lastTagCount = DYNINSTtagCount;
}
