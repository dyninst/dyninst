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
 * primitives.C - instrumentation primitives.
 *
 * $Log: primitives.C,v $
 * Revision 1.17  1996/11/19 16:28:15  newhall
 * Fix to stack walking on Solaris: find leaf functions in stack (these can occur
 * on top of stack or in middle of stack if the signal handler is on the stack)
 * Fix to relocated functions: new instrumentation points are kept on a per
 * process basis.  Cleaned up some of the code.
 *
 * Revision 1.16  1996/10/31 09:04:02  tamches
 * removed this file
 *
 * Revision 1.15  1996/08/16 21:19:35  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.14  1996/08/12 16:27:01  mjrg
 * Code cleanup: removed cm5 kludges and some unused code
 *
 * Revision 1.13  1996/05/15 18:32:53  naim
 * Fixing bug in inferiorMalloc and adding some debugging information - naim
 *
 * Revision 1.12  1996/05/08  23:55:01  mjrg
 * added support for handling fork and exec by an application
 * use /proc instead of ptrace on solaris
 * removed warnings
 *
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
 */

#include "util/h/headers.h"
#include "rtinst/h/trace.h"
#include "rtinst/h/rtinst.h"
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
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

        const instPoint *func_entry = sampleFunction->funcEntry(proc);
	ret->sampler = addInstFunc(proc, func_entry, 
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
    if (!handle) {
      logLine("Serious error: trying to free invalid intCounterHandle\n");
      assert(0);
    }
    if (handle->sampler) 
      deleteInst(handle->sampler, getAllTrampsAtPoint(handle->sampler));
#ifdef FREEDEBUG1
    sprintf(errorLine,"***** (pid=%d) In freeIntCounter, calling inferiorFree, pointer=0x%x\n",handle->proc->pid,(unsigned) handle->counterPtr);
    logLine(errorLine);
#endif    
    inferiorFree(handle->proc, (unsigned) handle->counterPtr, dataHeap,
                 pointsToCheck);
    free(handle);
}

// called when a process forks. The counter already exists in the child.
// We need to assign a new id and reset the initial value.
intCounterHandle *dupIntCounter(intCounterHandle *parentCounter, process *childProc,
				int value)
{
    intCounterHandle *ret;

    ret = new intCounterHandle;
    ret->proc = childProc;
    ret->data.id.aggregate = childProc->aggregate;
    ret->data.id.id = counterId++;
    ret->data.value = value;
    ret->counterPtr = parentCounter->counterPtr;
    assert(childProc->instInstanceMapping.defines(parentCounter->sampler));
    ret->sampler = childProc->instInstanceMapping[parentCounter->sampler];
    childProc->writeDataSpace((caddr_t) ret->counterPtr, sizeof(intCounter),
			 (caddr_t) &ret->data);
    return(ret);
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
        const instPoint *func_entry = sampleFunction->funcEntry(proc);
	ret->sampler = addInstFunc(proc, func_entry, ast,
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
    if (!handle) {
      logLine("Serious error: trying to free invalid timerHandle\n");
      assert(0);
    }
    if (handle->sampler)
      deleteInst(handle->sampler, getAllTrampsAtPoint(handle->sampler));
#ifdef FREEDEBUG1
    sprintf(errorLine,"***** (pid=%d) In freeTimer, calling inferiorFree, pointer=0x%x\n",handle->proc->pid,(unsigned) handle->timerPtr);
    logLine(errorLine);
#endif
    inferiorFree(handle->proc, (unsigned) handle->timerPtr, dataHeap,
                 pointsToCheck);
    free(handle);
}

// called when a process forks. The timer already exists in the child,
// since it was copied on the fork.
// We need to assign a new id and reset the initial value.
timerHandle *dupTimer(timerHandle *parentTimer, process *childProc)
{
    timerHandle *ret;

    ret = new timerHandle;
    ret->proc = childProc;
    ret->timerPtr = parentTimer->timerPtr;
    P_memset((void*)&ret->data, (int)'\0', sizeof(tTimer));
    ret->data.id.aggregate = childProc->aggregate;
    ret->data.id.id = counterId++;
    ret->data.type = parentTimer->data.type;
    ret->data.normalize = 1;
    assert(childProc->instInstanceMapping.defines(parentTimer->sampler));
    ret->sampler = childProc->instInstanceMapping[parentTimer->sampler];
    assert(ret->sampler);
    childProc->writeDataSpace((caddr_t)ret->timerPtr, sizeof(tTimer), (caddr_t)&ret->data);
    return(ret);
}
