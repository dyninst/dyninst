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
 * Revision 1.8  1996/08/16 21:27:46  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.7  1996/04/09 15:52:42  naim
 * Fixing prototype for procedure DYNINSTgenerateTraceRecord and adding
 * additional parameters to a call to this function in RTtags.c that has these
 * parameters missing - naim
 *
 * Revision 1.6  1996/03/01  22:29:10  mjrg
 * Added type to resources.
 * Added function DYNINSTexit for better support for exit from the application.
 * Added reporting of sample in DYNINSTinit to avoid loosing sample values.
 *
 * Revision 1.5  1996/02/15 14:55:48  naim
 * Minor changes to timers and cost model - naim
 *
 * Revision 1.4  1996/02/01  17:48:57  naim
 * Fixing some problems related to timers and race conditions. I also tried to
 * make a more standard definition of certain procedures (e.g. reportTimer)
 * across all platforms - naim
 *
 * Revision 1.3  1994/07/14  23:36:07  hollings
 * added extra arg to generateTrace.
 *
 */

#include <stdio.h>
#include <memory.h>

#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"

#ifdef COSTTEST
extern time64 *DYNINSTtest;
extern int *DYNINSTtestN;
#endif

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

    time64 process_time = DYNINSTgetCPUtime();
    time64 wall_time = DYNINSTgetWalltime();

#ifdef COSTTEST
    time64 startT,endT;
    startT=DYNINSTgetCPUtime();
#endif

    for (i=lastTagCount; i < DYNINSTtagCount; i++) {
	memset(&newRes, '\0', sizeof(newRes));
	sprintf(newRes.name, "SyncObject/MsgTag/%d", DYNINSTtags[i]);
	strcpy(newRes.abstraction, "BASE");
	newRes.type = RES_TYPE_INT;
	DYNINSTgenerateTraceRecord(0, TR_NEW_RESOURCE, 
	    sizeof(struct _newresource), &newRes, 1, wall_time, process_time);
    }
    lastTagCount = DYNINSTtagCount;

#ifdef COSTTEST
    endT=DYNINSTgetCPUtime();
    DYNINSTtest[8]+=endT-startT;
    DYNINSTtestN[8]++;
#endif
}
