

/*
 * metricDefs-critPath.C - Compute the Critical Path.
 *
 * $Log: metricDefs-critPath.C,v $
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

