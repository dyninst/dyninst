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
#include "paradyn/src/DMthread/BufferPool.h"
#include "paradyn/src/DMthread/DVbufferpool.h"


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
}

performanceStream::~performanceStream(){
    // indicate that handle is free for reuse
    allStreams.undef(handle);
    datavalues_bufferpool.dealloc(my_buffer);
    my_buffer = 0;
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

//
// send buffer of data values to client
//
void performanceStream::flushBuffer(){

    // send data to client
    if(next_buffer_loc){
	assert(my_buffer);
	dataManager::dm->setTid(threadId);
        dataManager::dm->newPerfData(dataFunc.sample, 
				     my_buffer, next_buffer_loc);
    }
    next_buffer_loc = 0;
    my_buffer = 0;
}

// 
// fills up buffer with new data values and flushs the buffer if 
// it is full 
//
void performanceStream::callSampleFunc(metricInstanceHandle mi,
				       sampleValue *buckets,
				       int count,
				       int first,
				       phaseType type)
{
    if (dataFunc.sample) {
	for(int i = first; i < (first+count); i++) {
	    if(!my_buffer) {
	         if (!this->reallocBuffer()) assert(0);	
		 assert(my_buffer);
	    }
	    (*my_buffer)[next_buffer_loc].mi = mi; 
	    (*my_buffer)[next_buffer_loc].bucketNum = i; 
	    (*my_buffer)[next_buffer_loc].value = buckets[i-first]; 
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

void performanceStream::callPredictedCostFuc(metricHandle m_handle,
				             resourceListHandle rl_handle,
					     float cost)
{
    if (controlFunc.cFunc) {
	dataManager::dm->setTid(threadId);
	dataManager::dm->predictedDataCost(controlFunc.cFunc, m_handle,
					   rl_handle, cost);
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

void performanceStream::foldAll(timeStamp width,phaseType phase_type){

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
	dataManager::dm->newPhaseInfo(controlFunc.pFunc,
				      phase.GetPhaseHandle(),
			 	      phase.PhaseName(),
			 	      phase.GetPhaseHandle(),
			 	      phase.GetStartTime(),
			 	      phase.GetEndTime(),
			 	      phase.GetBucketWidth(),
				      with_new_pc,
				      with_visis);
    }
}

performanceStream *performanceStream::find(perfStreamHandle psh){
    if(allStreams.defines(psh)){
        return(allStreams[psh]);
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

//
// if my_buffer has data values, flush it, then decrease its size
//
void performanceStream::removeGlobalUser(perfStreamHandle p){

    performanceStream *ps = performanceStream::find(p); 
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
						  float cost){

    bool found = false; 
    u_int which;
    for(u_int i=0; i < pred_Cost_buff.size(); i++){
        if((pred_Cost_buff[i])->requestId == requestId){
	    found = true;
	    which = i;
	    break;
    } }
    if(found){
        predCostType *value = pred_Cost_buff[which]; 
	assert(value->howmany);
	if(cost > value->max) value->max = cost;
	value->howmany--;
	// if all values have been collected, send result to calling thread
	if(!value->howmany && controlFunc.cFunc ){
	    this->callPredictedCostFuc(value->m_handle, value->rl_handle,
				       value->max);
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

