/*
 * Copyright (c) 1993, 1994 Barton P. Miller, Jeff Hollingsworth,
 *     Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, Karen
 *     Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.
 *
 * This software is furnished under the condition that it may not be
 * provided or otherwise made available to, or used by, any other
 * person, except as provided for by the terms of applicable license
 * agreements.  No title to or ownership of the software is hereby
 * transferred.  The name of the principals may not be used in any
 * advertising or publicity related to this software without specific,
 * written prior authorization.  Any use of this software must include
 * the above copyright notice.
 *
 */
#include <assert.h>
extern "C" {
double   quiet_nan();
#include <malloc.h>
#include <stdio.h>
}
#include "DMperfstream.h"
#include "DMmetric.h"
#include "DMresource.h"

performanceStream::performanceStream(dataType t, 
				     dataCallback dc, 
				     controlCallback cc, 
				     int tid) {
    type = t;
    dataFunc = dc;
    controlFunc = cc;
    sampleRate = 10000;
    threadId = tid;
    //  check for reuse of existing handle
    for(unsigned i=0; i < nextId.size(); i++){
        if((!nextId[i])){
	    handle = i;
            nextId[i] = true;
            allStreams[handle] = this;
	    return;
    }}
    // if not found, add a new element to the id array
    handle = nextId.size();
    nextId += true;
    assert(handle < nextId.size());
    allStreams[handle] = this;
}

performanceStream::~performanceStream(){
    // indicate that handle is free for reuse
    nextId[handle] = false;
    allStreams.undef(handle);
}

void performanceStream::callSampleFunc(metricInstanceHandle mi,
				       sampleValue *buckets,
				       int count,
				       int first)
{
#ifdef n_def
    for(unsigned j=first; j < (first+count); j++){
        if((j%50) == 0){
	    cout << "performanceStream::callSampleFunc: " << endl;
	    cout << "    first: " << first << " last: " << (first+count) << endl;
    } }
#endif

    if (dataFunc.sample) {
	dataManager::dm->setTid(threadId);
	// dataManager::dm->newPerfData(dataFunc.sample, handle, mi, 
        // 			     buckets, count, first);
	for(unsigned i = first; i < (first+count); i++) {
	    dataManager::dm->newPerfData(dataFunc.sample, handle, mi,
				i, buckets[i-first]);
	}
    }
}

void performanceStream::callResourceFunc(resourceHandle parent,
				         resourceHandle  child,
				         const char *name,
					 const char *abstr)
{
    if (controlFunc.rFunc) {
	dataManager::dm->setTid(threadId);
	dataManager::dm->newResourceDefined(controlFunc.rFunc,handle,
			 parent,child,name,abstr);
    }
}

void performanceStream::callResourceBatchFunc(batchMode mode)
{
    if (controlFunc.bFunc) {
	dataManager::dm->setTid(threadId);
	dataManager::dm->changeResourceBatchMode(controlFunc.bFunc, 
						 handle, mode);
    }
}

void performanceStream::callFoldFunc(timeStamp width,phaseType phase_type)
{
    if (controlFunc.fFunc) {
	dataManager::dm->setTid(threadId);
	dataManager::dm->histFold(controlFunc.fFunc, handle, width, phase_type);
    }
}


void performanceStream::callStateFunc(appState state)
{
    if (controlFunc.sFunc) {
	dataManager::dm->setTid(threadId);
	dataManager::dm->changeState(controlFunc.sFunc, handle, state);
    }
}

void performanceStream::notifyAllChange(appState state){

   dictionary_hash_iter<perfStreamHandle,performanceStream*> allS(allStreams);
   perfStreamHandle h;
   performanceStream *ps;
   while(allS.next(h,ps)){
       ps->callStateFunc(state);
   }
}

void performanceStream::ResourceBatchMode(batchMode mode){
   dictionary_hash_iter<perfStreamHandle,performanceStream*> allS(allStreams);
   perfStreamHandle h;
   performanceStream *ps;
   while(allS.next(h,ps)){
       ps->callResourceBatchFunc(mode);
   }

}

void performanceStream::foldAll(timeStamp width,phaseType phase_type){

   dictionary_hash_iter<perfStreamHandle,performanceStream*> allS(allStreams);
   perfStreamHandle h;
   performanceStream *ps;
   while(allS.next(h,ps)){
       ps->callFoldFunc(width,phase_type);
   }
}

void performanceStream::callPhaseFunc(phaseInfo& phase)
{
    if (controlFunc.pFunc) {
	dataManager::dm->setTid(threadId);
	dataManager::dm->newPhaseInfo(controlFunc.pFunc,
				      phase.GetPhaseHandle(),
			 	      phase.PhaseName(),
			 	      phase.GetPhaseHandle(),
			 	      phase.GetStartTime(),
			 	      phase.GetEndTime(),
			 	      phase.GetBucketWidth());
    }
}

performanceStream *performanceStream::find(perfStreamHandle psh){
    if(allStreams.defines(psh)){
        return(allStreams[psh]);
    }
    return((performanceStream *)NULL);
}
