/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) /p/paradyn/CVSROOT/core/paradynd/src/primitives.C,v 1.8 1995/02/16 08:53:58 markc Exp";
#endif

/*
 * primitives.C - instrumentation primitives.
 *
 * $Log: primitives.C,v $
 * Revision 1.11  1996/04/03 14:27:50  naim
 * Implementation of deallocation of instrumentation for solaris and sunos - naim
 *
 * Revision 1.10  1996/03/25  20:25:39  tamches
 * the reduce-mem-leaks-in-paradynd commit
 *
 * Revision 1.9  1995/08/24 15:04:28  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.8  1995/02/16  08:53:58  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.7  1995/02/16  08:34:30  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.6  1994/11/02  11:14:55  markc
 * Added suppport for process classes.
 * Fixed typos.
 *
 * Revision 1.5  1994/09/22  02:22:17  markc
 * changed *allocs to news
 * cast args to memset
 *
 * Revision 1.4  1994/07/28  22:40:44  krisna
 * changed definitions/declarations of xalloc functions to conform to alloc.
 *
 * Revision 1.3  1994/07/12  19:45:21  jcargill
 * Hardware combine on the CM5 no longer requires a special sampling function.
 *
 * Revision 1.2  1994/06/29  02:52:46  hollings
 * Added metricDefs-common.{C,h}
 * Added module level performance data
 * cleanedup types of inferrior addresses instrumentation defintions
 * added firewalls for large branch displacements due to text+data over 2meg.
 * assorted bug fixes.
 *
 * Revision 1.1  1994/01/27  20:31:36  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.9  1993/10/19  15:27:54  hollings
 * AST based mini-tramp code generator.
 *
 * Revision 1.7  1993/10/07  19:49:26  jcargill
 * Added true combines for global instrumentation
 *
 * Revision 1.6  1993/10/01  21:29:41  hollings
 * Added resource discovery and filters.
 *
 * Revision 1.5  1993/08/11  01:47:56  hollings
 * added check for sample before trying to delete it.
 *
 * Revision 1.4  1993/07/13  18:29:27  hollings
 * new include file syntax.
 *
 * Revision 1.3  1993/06/22  19:00:01  hollings
 * global inst state.
 *
 * Revision 1.2  1993/06/08  20:14:34  hollings
 * state prior to bc net ptrace replacement.
 *
 * Revision 1.1  1993/03/19  22:45:45  hollings
 * Initial revision
 *
 *
 */

#include "util/h/headers.h"
#include "rtinst/h/trace.h"
#include "rtinst/h/rtinst.h"
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "primitives.h"
#include "ast.h"
#include "util.h"

/* unique id for a counter */
static int counterId=0;

extern vector<unsigned> getAllTrampsAtPoint(instInstance *instance);

/*
 * Define the insturmentation primitives.
 *
 *    This is the creation/deletion, and read functions.   
 *    The actual primitives are in the runtime library.
 */
intCounterHandle *createIntCounter(process *proc, int value, bool report)
{
    intCounterHandle *ret;
    pdFunction *sampleFunction;

    ret = new intCounterHandle;
    ret->proc = proc;
    ret->data.id.aggregate = proc->aggregate;
    ret->data.id.id = counterId++;
    ret->counterPtr = (intCounter *) 
	inferiorMalloc(proc, sizeof(intCounter), dataHeap);
    if (!ret->counterPtr) return(NULL);
    ret->data.value = value;
    proc->writeDataSpace((caddr_t) ret->counterPtr, sizeof(intCounter),
			 (caddr_t) &ret->data);
    // copyToProcess(proc, (char*) &ret->data, (char*) ret->counterPtr, sizeof(intCounter));
    
    /*
     * add code to sample value.
     */
    if (report) {
      sampleFunction = (proc->symbols)->findOneFunction("DYNINSTsampleValues");
      if (!sampleFunction)
	abort();

	AstNode ast("DYNINSTreportCounter", 
		    AstNode(Constant, ret->counterPtr));

	ret->sampler = addInstFunc(proc, sampleFunction->funcEntry(), 
	    ast, callPreInsn, orderLastAtPoint);
    }
    return(ret);
}

/*
 * read the current value of a counter.
 */
int getIntCounterValue(intCounterHandle *handle)
{
  (handle->proc)->readDataSpace((caddr_t)handle->counterPtr, 
                                sizeof(intCounter),
				(caddr_t)&handle->data,true);
  // copyFromProcess(handle->proc, (char*)handle->counterPtr, (char*)&handle->data, sizeof(intCounter));
    return(handle->data.value);
}

void freeIntCounter(intCounterHandle *handle, 
                    vector<unsigVecType> pointsToCheck)
{
    if (handle->sampler) 
      deleteInst(handle->sampler, getAllTrampsAtPoint(handle->sampler));
    inferiorFree(handle->proc, (unsigned) handle->counterPtr, dataHeap,
                 pointsToCheck);
    free(handle);
}

timerHandle *createTimer(process *proc, timerType type, bool report)
{
    timerHandle *ret;
    pdFunction *sampleFunction;

    ret = new timerHandle;
    ret->proc = proc;
    ret->timerPtr = (tTimer *) 
	inferiorMalloc(proc, sizeof(tTimer), dataHeap);
    if (!ret->timerPtr) return(NULL);
    P_memset((void*)&ret->data, (int)'\0', sizeof(tTimer));
    ret->data.id.aggregate = proc->aggregate;
    ret->data.id.id = counterId++;
    ret->data.type = type;
    ret->data.normalize = 1;
    
    proc->writeDataSpace((caddr_t)ret->timerPtr, sizeof(tTimer), (caddr_t)&ret->data);
    // copyToProcess(proc, (char*)&ret->data, (char*)ret->timerPtr, sizeof(tTimer));

    /*
     * add code to sample value.
     */
    if (report) {
	sampleFunction = (proc->symbols)->findOneFunction("DYNINSTsampleValues");
	if (!sampleFunction)
	  abort();

	AstNode ast ("DYNINSTreportTimer",
		     AstNode(Constant, ret->timerPtr));
	ret->sampler = addInstFunc(proc, sampleFunction->funcEntry(), ast,
	    callPreInsn, orderLastAtPoint);
    }
    return(ret);
}

/*
 * read the current value of a timer.
 */
float getTimerValue(timerHandle *handle)
{
    float value;

    (handle->proc)->readDataSpace((caddr_t)handle->timerPtr, sizeof(tTimer),
				  (caddr_t)&handle->data, true);
    // copyFromProcess(handle->proc, (char*)handle->timerPtr, (char*)&handle->data,sizeof(tTimer));

    value = (double)handle->data.total/(double)handle->data.normalize;

    return(value);
}

void freeTimer(timerHandle *handle, vector<unsigVecType> pointsToCheck)
{
    deleteInst(handle->sampler, getAllTrampsAtPoint(handle->sampler));
    inferiorFree(handle->proc, (unsigned) handle->timerPtr, dataHeap,
                 pointsToCheck);
    free(handle);
}
