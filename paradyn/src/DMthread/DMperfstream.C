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

// $Id: DMperfstream.C,v 1.31 2002/11/25 23:51:48 schendel Exp $

#include <assert.h>
#include <limits.h>     // UINT_MAX
extern "C" {
#include <malloc.h>
#include <stdio.h>
}
#include "DMperfstream.h"
#include "DMmetric.h"
#include "DMresource.h"
#include "paradyn/src/DMthread/BufferPool.h"
#include "paradyn/src/DMthread/DVbufferpool.h"
#include "pdutil/h/pdDebugOstream.h"

extern pdDebug_ostream sampleVal_cerr;

performanceStream::performanceStream(dataType t, 
				     dataCallback dc, 
				     controlCallback cc, 
				     int tid) {
    type = t;
    dataFunc = dc;
    controlFunc = cc;
    threadId = tid;
    num_global_mis = 0;
    num_curr_mis = 0;
    my_buffer_size = 0;
    next_buffer_loc = 0;
    my_buffer = 0;
    handle = next_id++;
    allStreams[handle] = this;
    nextpredCostReqestId = 0;
    pred_Cost_buff.resize(0);
    // trace data streams
    num_trace_mis =0;
    my_traceBuffer_size = 0;
    next_traceBuffer_loc = 0;
    my_traceBuffer = 0;
}

performanceStream::~performanceStream(){
    // indicate that handle is free for reuse
    allStreams.undef(handle);
    datavalues_bufferpool.dealloc(my_buffer);
    my_buffer = 0;
    // trace data streams
    tracedatavalues_bufferpool.dealloc(my_traceBuffer);
    my_traceBuffer = 0;
}


bool performanceStream::reallocBuffer(){
    // reallocate a buffer of size my_buffer_size
    assert(!my_buffer);
    if(my_buffer_size){
        my_buffer = datavalues_bufferpool.alloc(my_buffer_size); 
	if(!my_buffer) return false;
	assert(my_buffer);
	assert(my_buffer->size() >= my_buffer_size);
    }
    return true;
}

bool performanceStream::reallocTraceBuffer(){
    // reallocate a buffer of size my_traceBuffer_size
    assert(!my_traceBuffer);
    if(my_traceBuffer_size){
        my_traceBuffer = tracedatavalues_bufferpool.alloc(my_traceBuffer_size);
        if(!my_traceBuffer) return false;
        assert(my_traceBuffer);
        assert(my_traceBuffer->size() >= my_traceBuffer_size);
    }
    return true;
}

//
// send buffer of data values to client
//
void performanceStream::flushBuffer(){
  sampleVal_cerr << "performanceStream::flushBuffer"; 
    // send data to client
    if(next_buffer_loc){
      sampleVal_cerr << "  -  flushing\n";
	assert(my_buffer);
	dataManager::dm->setTid(threadId);
        dataManager::dm->newPerfData(dataFunc.sample, 
				     my_buffer, next_buffer_loc);
    } else sampleVal_cerr << " - not flushing\n";

    next_buffer_loc = 0;
    my_buffer = 0;
}

//
// flushes both the performance stream and visi thread buffers
//
void performanceStream::signalToFlush() {
    sampleVal_cerr << "performanceStream::signalToFlush\n";
    dataManager::dm->setTid(threadId);
    if(controlFunc.flFunc != NULL)
      dataManager::dm->forceFlush(controlFunc.flFunc);
}

//
// send buffer of trace data values to client
//
void performanceStream::flushTraceBuffer(){

    // send data to client
    if(next_traceBuffer_loc){
        assert(my_traceBuffer);
        dataManager::dm->setTid(threadId);
        dataManager::dm->newTracePerfData(dataFunc.trace,
                                     my_traceBuffer,next_traceBuffer_loc);
    }
    next_traceBuffer_loc = 0;
    my_traceBuffer = 0;
}

// 
// fills up buffer with new data values and flushs the buffer if 
// it is full 
//
void performanceStream::callSampleFunc(metricInstanceHandle mi,
				       pdSample *buckets,
				       int count,
				       int first,
				       phaseType type)
{
  sampleVal_cerr << "performanceStream::callSampleFunc\n";

    if (dataFunc.sample) {
      sampleVal_cerr << "dataFunc.sample, first: " << first << "  count: " 
		     << count << "\n";
	for(int i = first; i < (first+count); i++) {
	    if(!my_buffer) {
	         if (!this->reallocBuffer()) assert(0);	
		 assert(my_buffer);
	    }
	    (*my_buffer)[next_buffer_loc].mi = mi; 
	    (*my_buffer)[next_buffer_loc].bucketNum = i;
	    (*my_buffer)[next_buffer_loc].value  = buckets[i-first];
	    (*my_buffer)[next_buffer_loc].type = type; 
	    next_buffer_loc++;
	    if(next_buffer_loc >= my_buffer_size) {
                this->flushBuffer();
		assert(!next_buffer_loc);
		assert(!my_buffer);
	    }
	}
    }
}

//
// fills up trace buffer with new trace data values and flushs the buffer if
// it is full
//
void performanceStream::callTraceFunc(metricInstanceHandle mi,
                                      const void *data,
                                      int length)
{
    if (dataFunc.trace) {
        if(!my_traceBuffer) {
            if (!this->reallocTraceBuffer()) assert(0);
                assert(my_traceBuffer);
        }
        (*my_traceBuffer)[next_traceBuffer_loc].mi = mi;
        (*my_traceBuffer)[next_traceBuffer_loc].traceRecord = byteArray(data,length);
        next_traceBuffer_loc++;
//      if(next_traceBuffer_loc >= my_traceBuffer_size) {
          if(next_traceBuffer_loc >= my_traceBuffer_size || length == 0) {
            this->flushTraceBuffer();
            assert(!next_traceBuffer_loc);
            assert(!my_traceBuffer);
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

void performanceStream::callResourceRetireFunc(resourceHandle uniqueID,
                                               const char *name,
                                               const char *abstr)
{
    if (controlFunc.retireFunc) {
	dataManager::dm->setTid(threadId);
	dataManager::dm->retireResource_(controlFunc.retireFunc, handle,
                                         uniqueID, name, abstr);
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

void performanceStream::callFoldFunc(timeLength width, phaseType phase_type)
{
    if (controlFunc.fFunc) {
        // we're creating this width variable in the heap so that when passed
        // to a different thread it can be read (as opposed to a variable on
        // the stack)
        timeLength *width_heap = new timeLength;
        // width_heap is deleted by a callee of histFold()
        *width_heap = width;
	dataManager::dm->setTid(threadId);
	dataManager::dm->histFold(controlFunc.fFunc, handle, width_heap, 
				  phase_type);
    }
}

void performanceStream::callInitialActualValueFunc(metricHandle mi,
						   pdSample initActVal,
						   phaseType phase_type)
{
    if (controlFunc.avFunc) {
        // we're creating this width variable in the heap so that when passed
        // to a different thread it can be read (as opposed to a variable on
        // the stack)
        pdSample *heapInitActVal = new pdSample;
        // width_heap is deleted by a callee of histFold()
        *heapInitActVal = initActVal;
	dataManager::dm->setTid(threadId);
	dataManager::dm->setInitialActualValue(controlFunc.avFunc, handle, 
					       mi, heapInitActVal, phase_type);
    }
}

void performanceStream::callStateFunc(appState state)
{
    if (controlFunc.sFunc) {
	dataManager::dm->setTid(threadId);
	dataManager::dm->changeState(controlFunc.sFunc, handle, state);
    }
}

void performanceStream::callPredictedCostFuc(metricHandle m_handle,
				             resourceListHandle rl_handle,
					     float cost,
					     u_int clientID)
{
    if (controlFunc.cFunc) {
	dataManager::dm->setTid(threadId);
	dataManager::dm->predictedDataCost(controlFunc.cFunc, m_handle,
					   rl_handle, cost,clientID);
    }
}

void performanceStream::callDataEnableFunc(vector<metricInstInfo> *response,
					   u_int request_id)
{
    if (controlFunc.eFunc) {
	dataManager::dm->setTid(threadId);
	dataManager::dm->enableDataCallback(controlFunc.eFunc, response,
					    request_id);
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

void performanceStream::foldAll(timeLength width, phaseType phase_type)
{
   dictionary_hash_iter<perfStreamHandle,performanceStream*> allS(allStreams);
   perfStreamHandle h;
   performanceStream *ps;
   while(allS.next(h,ps)){
       ps->flushBuffer();  // first flush any data values in buffer
       ps->callFoldFunc(width,phase_type);
   }
}

void performanceStream::callPhaseFunc(phaseInfo& phase,
				      bool with_new_pc,
				      bool with_visis)
{
    if (controlFunc.pFunc) {
	dataManager::dm->setTid(threadId);
        // We're creating these variables on the heap (as opposed to on the
        // stack) so that when passed to a different thread they can be read.
	// The memory gets freed by a callee of newPhaseInfo.
	relTimeStamp *stTimePtr  = new relTimeStamp;
	*stTimePtr = phase.GetStartTime();
	relTimeStamp *endTimePtr = new relTimeStamp;
	*endTimePtr = phase.GetEndTime();
	timeLength *bwidthPtr = new timeLength;
	*bwidthPtr = phase.GetBucketWidth();
	
	dataManager::dm->newPhaseInfo(controlFunc.pFunc,
				      phase.GetPhaseHandle(),
			 	      phase.PhaseName(),
			 	      phase.GetPhaseHandle(),
			 	      stTimePtr, endTimePtr, bwidthPtr,
				      with_new_pc,
				      with_visis);
    }
}

performanceStream *performanceStream::find(perfStreamHandle psh){
    performanceStream *pshan;
    bool found = allStreams.find(psh, pshan);
    if (found){
      return(pshan);
    }
    return((performanceStream *)NULL);
}

//  if my_buffer has data values, flush it
//  then increase num_curr_mis count and my_buffer_size 
void performanceStream::addCurrentUser(perfStreamHandle p){

    performanceStream *ps = performanceStream::find(p); 
    if(!ps) return;
    if(ps->next_buffer_loc && ps->my_buffer){
        ps->flushBuffer();
	assert(!(ps->my_buffer));
	assert(!(ps->next_buffer_loc));
    }
    ps->num_curr_mis++;
    if(ps->my_buffer_size < DM_DATABUF_LIMIT) {
        ps->my_buffer_size++;
    }
    if(ps->num_curr_mis + ps->num_global_mis){
	assert(ps->my_buffer_size);
    }
}

//  if my_buffer has data values, flush it
//  then increase num_global_mis count and my_buffer_size 
void performanceStream::addGlobalUser(perfStreamHandle p){

    performanceStream *ps = performanceStream::find(p); 
    if(!ps) return;
    if(ps->next_buffer_loc && ps->my_buffer){
        ps->flushBuffer();
	assert(!(ps->my_buffer));
	assert(!(ps->next_buffer_loc));
    }
    ps->num_global_mis++;
    if(ps->my_buffer_size < DM_DATABUF_LIMIT) {
        ps->my_buffer_size++;
    }
    if(ps->num_curr_mis + ps->num_global_mis){
	assert(ps->my_buffer_size);
    }
}

//  if my_traceBuffer has data values, flush it
//  then increase num_trace_mis count
//  however, my_traceBuffer_size remains same (10)
void performanceStream::addTraceUser(perfStreamHandle p){

    performanceStream *ps = performanceStream::find(p);
    if(!ps) return;
    if(ps->next_traceBuffer_loc && ps->my_traceBuffer){
        ps->flushTraceBuffer();
        assert(!(ps->my_traceBuffer));
        assert(!(ps->next_traceBuffer_loc));
    }
    ps->num_trace_mis++;
    if(ps->my_traceBuffer_size < DM_DATABUF_LIMIT) {
        ps->my_traceBuffer_size++;
    }
    //if(ps->num_trace_mis){ 
    //    ps->my_traceBuffer_size = 10;
    //    assert(ps->my_traceBuffer_size);
    //}

}

//
// if my_buffer has data values, flush it, then decrease its size
//
void performanceStream::removeGlobalUser(perfStreamHandle p){
    performanceStream *ps = performanceStream::find(p); 
    sampleVal_cerr << "performanceStream::removeGlobalUser - " << ps << "\n";
    if(!ps) return;
    if(ps->next_buffer_loc && ps->my_buffer){
        ps->flushBuffer();
	assert(!(ps->my_buffer));
	assert(!(ps->next_buffer_loc));
    }
    if(ps->num_global_mis && ps->my_buffer_size){
        ps->num_global_mis--;
	if((ps->num_global_mis + ps->num_curr_mis) < DM_DATABUF_LIMIT){
	    if(ps->my_buffer_size) ps->my_buffer_size--;
	}
    }
    if(ps->num_curr_mis + ps->num_global_mis){
	assert(ps->my_buffer_size);
    }
}

//
// if my_buffer has data values, flush it, then decrease its size
//
void performanceStream::removeCurrentUser(perfStreamHandle p){
    performanceStream *ps = performanceStream::find(p); 
    sampleVal_cerr << "performanceStream::removeCurrentUser - " << ps << "\n";
    if(!ps) return;
    if(ps->next_buffer_loc && ps->my_buffer){
        ps->flushBuffer();
	assert(!(ps->my_buffer));
	assert(!(ps->next_buffer_loc));
    }
    if(ps->num_curr_mis && ps->my_buffer_size){
        ps->num_curr_mis--;
	if((ps->num_global_mis + ps->num_curr_mis) < DM_DATABUF_LIMIT){
	    if(ps->my_buffer_size) ps->my_buffer_size--;
	}
    }
    if(ps->num_curr_mis + ps->num_global_mis){
	assert(ps->my_buffer_size);
    }
}

//
// For all performanceStreams delete buffer size by the num_curr_mis,
// flush buffer if neccessary, set num_curr_mis to 0 
//
void performanceStream::removeAllCurrUsers(){

   dictionary_hash_iter<perfStreamHandle,performanceStream*> allS(allStreams);
   perfStreamHandle h;
   performanceStream *ps;
   while(allS.next(h,ps)){
       if(ps->next_buffer_loc && ps->my_buffer){
           ps->flushBuffer();
	   assert(!(ps->my_buffer));
	   assert(!(ps->next_buffer_loc));
       }
       if(ps->num_curr_mis && ps->my_buffer_size){
           ps->num_curr_mis = 0;
	   if((ps->num_global_mis) < DM_DATABUF_LIMIT){
	     ps->my_buffer_size = ps->num_global_mis;
	   }
       }
       if(ps->num_curr_mis + ps->num_global_mis){
	   assert(ps->my_buffer_size);
       }
   }
}

//
// if my_traceBuffer has data values, flush it
// if num_trace_mis becomes 0, set my_traceBuffer_size to 0 
//
void performanceStream::removeTraceUser(perfStreamHandle p){

    performanceStream *ps = performanceStream::find(p);
    if(!ps) return;
    if(ps->next_traceBuffer_loc && ps->my_traceBuffer){
        ps->flushTraceBuffer();
        assert(!(ps->my_traceBuffer));
        assert(!(ps->next_traceBuffer_loc));
    }
    if(ps->num_trace_mis && ps->my_traceBuffer_size){
        ps->num_trace_mis--;
        if(ps->num_trace_mis < DM_DATABUF_LIMIT){
            if(ps->my_traceBuffer_size) ps->my_traceBuffer_size--;
        }
    }
    if(ps->num_trace_mis){
      assert(ps->my_traceBuffer_size);
    }
    else
        ps->my_traceBuffer_size = 0;


}

//
// creates a new predCostRecord 
//
bool performanceStream::addPredCostRequest(perfStreamHandle ps_handle,
					   u_int &requestId,
					   metricHandle m_handle,
					   resourceListHandle rl_handle,
					   u_int howmany){

    performanceStream *ps = performanceStream::find(ps_handle); 
    if(!ps) return false;

    predCostType *new_value = new predCostType; 
    if(!new_value) return false;
    new_value->m_handle =  m_handle;
    new_value->rl_handle =  rl_handle;
    new_value->max = 0.0;
    new_value->howmany =  howmany;
    requestId = ps->nextpredCostReqestId++; 
    new_value->requestId = requestId;
    ps->pred_Cost_buff += new_value;
    new_value = 0;
    return true;
}

//
// update the predicted cost value, and if this is the last outstanding
// response then send result to calling thread
//
void performanceStream::predictedDataCostCallback(u_int requestId,
						  float cost,
						  u_int clientID){

    bool found = false; 
    u_int which = UINT_MAX;
    for(u_int i=0; i < pred_Cost_buff.size(); i++){
        if((pred_Cost_buff[i])->requestId == requestId){
	    found = true;
	    which = i;
	    break;
    } }
    if(found){
        assert(which < pred_Cost_buff.size());
        predCostType *value = pred_Cost_buff[which]; 
	assert(value->howmany);
	if(cost > value->max) value->max = cost;
	value->howmany--;
	// if all values have been collected, send result to calling thread
	if(!value->howmany && controlFunc.cFunc ){
	    this->callPredictedCostFuc(value->m_handle, value->rl_handle,
				       value->max,clientID);
            u_int size = pred_Cost_buff.size();
	    if(size){
                pred_Cost_buff[which] = pred_Cost_buff[size-1];
		pred_Cost_buff.resize(size-1);
	    }
	    delete value;
	}
    }
    else{
        cout << "error in perfStream::predictedDataCostCallback" << endl;
	assert(0);
    }
}

