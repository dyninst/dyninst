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

#include "symtab.h"
#include "process.h"
#include "rtinst/h/rtinst.h"
#include "rtinst/h/critPath.h"
#include "inst.h"
#include "dyninstP.h"
#include "metric.h"
#include "ast.h"
#include "util.h"
#include "rtinst/h/trace.h"
#include "metricDef.h"
#include "os.h"
#include "main.h"

#define MILLION	1000000.0

dictionary_hash<unsigned, cpSample*> contextToSample(uiHash);

void processCP(process *proc, traceHeader *hdr, cpSample *sample)
{
    cpSample *item;

    item = contextToSample[sample->id];
    if (!item) {
	item = (cpSample *) calloc(sizeof(cpSample), 1);
	contextToSample[sample->id] = item;
	item->id = sample->id;
    }

    if (sample->length > item->length) {
	tp->cpDataCallbackFunc(0, hdr->wall/MILLION, sample->id,
	    sample->length/MILLION, sample->share/MILLION);

	*item = *sample;
	// fprintf(stderr, "Got CP Sample for %d (%f,%f) at %f\n", sample->id,
	    // sample->share/1000000.0, sample->length/1000000.0,
	    // hdr->wall/MILLION);
    }
}

