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
 * metricDefs-critPath.C - Compute the Critical Path.
 *
 * $Log: metricDefs-critPath.C,v $
 * Revision 1.6  2001/06/20 20:33:44  schendel
 * Use pdutil instead of pdutilOld.  Update to use new time and sample value
 *   types (ie. timeStamp, relTimeStamp, timeLength, pdSample, pdRate).
 *
 * Revision 1.5  1997/10/10 00:42:17  tamches
 * removed a warning
 *
 * Revision 1.4  1997/02/21 20:16:00  naim
 * Moving files from paradynd to dyninstAPI + eliminating references to
 * dataReqNode from the ast class. This is the first pre-dyninstAPI commit! - naim
 *
 * Revision 1.3  1996/08/16 21:19:27  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.2  1996/05/08 15:55:10  hollings
 * Commented out a debugging printf
 *
 * Revision 1.1  1995/08/24  15:04:23  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.5  1995/07/14  21:25:56  hollings
 * Updated to mid July 1995 Wisconsin version (umd4).
 *
 * Revision 1.4  1995/05/23  14:57:19  hollings
 * added CP zeroing.
 *
 * Revision 1.3  1995/04/03  18:48:23  hollings
 * Added CP code.
 *
 * Changed pause/continue to attach/detach ptrace to make signals faster.
 *
 * Revision 1.2  1995/02/19  22:32:43  hollings
 * Merge in changes from umd3 release from Wisconsin.
 *
 * Revision 1.1  1995/02/16  15:39:33  hollings
 * added support for critical path.
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "rtinst/h/rtinst.h"
#include "rtinst/h/critPath.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/dyninstP.h"
#include "paradynd/src/metric.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "rtinst/h/trace.h"
#include "paradynd/src/metricDef.h"
#include "dyninstAPI/src/os.h"
#include "paradynd/src/main.h"
#include "paradynd/src/init.h"

#define MILLION	1000000.0

dictionary_hash<unsigned, cpSample*> contextToSample(uiHash);

void processCP(process *, traceHeader *hdr, cpSample *sample)
{
    cpSample *item;

    item = contextToSample[sample->id];
    if (!item) {
	item = (cpSample *) calloc(sizeof(cpSample), 1);
	contextToSample[sample->id] = item;
	item->id = sample->id;
    }

    if (sample->length > item->length) {
      timeStamp trWall(getWallTimeMgr().units2timeStamp(hdr->wall));
      double wall_secs = trWall.getD(timeUnit::sec(), timeBase::bStd());
      tp->cpDataCallbackFunc(0, wall_secs, sample->id,
	    sample->length/MILLION, sample->share/MILLION);

	*item = *sample;
    }
}

