/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/Attic/primitives.C,v 1.4 1994/07/28 22:40:44 krisna Exp $";
#endif

/*
 * primitives.C - instrumentation primitives.
 *
 * $Log: primitives.C,v $
 * Revision 1.4  1994/07/28 22:40:44  krisna
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
static int counterId;

extern process *nodePseudoProcess;

/*
 * Define the insturmentation primitives.
 *
 *    This is the creation/deletion, and read functions.   
 *    The actual primitives are in the runtime library.
 */
intCounterHandle *createIntCounter(process *proc, int value, Boolean report)
{
    AstNode *ast;
    intCounterHandle *ret;
    function *sampleFunction;

    ret = (intCounterHandle*) xcalloc(1, sizeof(intCounterHandle));
    ret->proc = proc;
    ret->data.id.aggregate = proc->aggregate;
    ret->data.id.id = counterId++;
    ret->counterPtr = (intCounter *) inferriorMalloc(proc, sizeof(intCounter));
    ret->data.value = value;
    copyToProcess(proc, &ret->data, ret->counterPtr, sizeof(intCounter));

    /*
     * add code to sample value.
     */
    if (report) {
	sampleFunction = findFunction(proc->symbols, "DYNINSTsampleValues");
	ast = new AstNode("DYNINSTreportCounter", 
			  new AstNode(Constant, ret->counterPtr), NULL);
	ret->sampler = addInstFunc(proc, sampleFunction->funcEntry, 
	    ast, callPreInsn, orderLastAtPoint);
    }
    return(ret);
}

/*
 * read the current value of a counter.
 */
int getIntCounterValue(intCounterHandle *handle)
{

    copyFromProcess(handle->proc, handle->counterPtr, &handle->data,
	sizeof(intCounter));
    return(handle->data.value);
}

void freeIntCounter(intCounterHandle *handle)
{
    if (handle->sampler) deleteInst(handle->sampler);
    inferriorFree(handle->proc, (int) handle->counterPtr);
    free(handle);
}


timerHandle *createTimer(process *proc, timerType type, Boolean report)
{
    AstNode *ast;
    timerHandle *ret;
    function *sampleFunction;

    ret = (timerHandle*) xcalloc(1, sizeof(timerHandle));
    ret->proc = proc;
    ret->timerPtr = (tTimer *) inferriorMalloc(proc, sizeof(tTimer));

    memset(&ret->data, '\0', sizeof(tTimer));
    ret->data.id.aggregate = proc->aggregate;
    ret->data.id.id = counterId++;
    ret->data.type = type;
    ret->data.normalize = 1;

    copyToProcess(proc, &ret->data, ret->timerPtr, sizeof(tTimer));

    /*
     * add code to sample value.
     */
    if (report) {
	sampleFunction = findFunction(proc->symbols, "DYNINSTsampleValues");
	ast = new AstNode("DYNINSTreportTimer",
			  new AstNode(Constant, ret->timerPtr), NULL);
	ret->sampler = addInstFunc(proc, sampleFunction->funcEntry, ast,
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

    copyFromProcess(handle->proc, handle->timerPtr, 
	&handle->data,sizeof(tTimer));

    value = (double)handle->data.total/(double)handle->data.normalize;

    return(value);
}

void freeTimer(timerHandle *handle)
{
    deleteInst(handle->sampler);
    inferriorFree(handle->proc, (int) handle->timerPtr);
    free(handle);
}
