/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

/////////////////////////////////////////////////////////////////////
// * VISIthread main loop
// * Callback routines for UI Upcalls:  VISIthreadchooseMetRes
//		VISIthreadshowMsgREPLY, VISIthreadshowErrorREPLY
// * Callback routines for DataManager Upcalls:  VISIthreadDataCallback  
//   		VISIthreadnewMetricCallback, VISIthreadFoldCallback 
//   		VISIthreadnewResourceCallback VISIthreadPhaseCallback
/////////////////////////////////////////////////////////////////////

// $Id: VISIthreadmain.C,v 1.111 2005/03/13 23:44:17 legendre Exp $

#include <signal.h>
#include <math.h>
#include <stdlib.h>
#include "pdutil/h/rpcUtil.h"
#include "pdutil/h/pdDebugOstream.h"
#include "paradyn/src/VMthread/VMtypes.h"
#include "VISIthread.thread.SRVR.h"
#include "VISIthreadTypes.h"
#include "paradyn/src/pdMain/paradyn.h"
#include "dyninstRPC.xdr.CLNT.h"
#include "paradyn/src/DMthread/DMinclude.h"
#include "paradyn/src/DMthread/DVbufferpool.h"
#include "paradyn/src/TCthread/tunableConst.h"
#include "paradyn/src/UIthread/minmax.h"
#include "pdutil/h/hist.h"   // delete this after type conversion of visis
#include "paradyn/src/DMthread/DMmetric.h" // delete after type convers. complt
#include "pdutil/h/makenan.h"  // delete this after the type conv. of visis

#define  ERROR_MSG(s1, s2) \
	 uiMgr->showError(s1,s2); 

char *AbbreviatedFocus(const char *);

extern unsigned enable_pd_samplevalue_debug;

#if ENABLE_DEBUG_CERR == 1
#define sampleVal_cerr if (enable_pd_samplevalue_debug) cerr
#else
#define sampleVal_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

void flush_buffer_if_full( VISIthreadGlobals *ptr) {
  sampleVal_cerr << "flush_buffer_if_full-   buffer_next_insert_index: " 
		 << ptr->buffer_next_insert_index <<",  buffer.size: " 
		 << ptr->buffer.size() << "\n" << std::flush;
   assert(ptr->buffer_next_insert_index <= ptr->buffer.size());
   if (ptr->buffer_next_insert_index != ptr->buffer.size())
      return;

   sampleVal_cerr << "calling ptr->visip->Data\n" << std::flush;
   ptr->visip->Data(ptr->buffer);

   if (ptr->visip->did_error_occur()) {
      PARADYN_DEBUG(("igen: after visip->Data() in VISIthreadDataHandler"));
      ptr->quit = 1;
      return;
   }
   ptr->buffer_next_insert_index = 0;
}

void flush_buffer_if_nonempty( VISIthreadGlobals *ptr) {
   const unsigned num_to_send = ptr->buffer_next_insert_index;
   sampleVal_cerr << "flush_buffer_if_nonempty - sending " <<num_to_send<<"\n";
   assert(num_to_send <= ptr->buffer.size());
   if (num_to_send == 0){
      ptr->buffer_next_insert_index = 0;
      return;
   }

   if (num_to_send < ptr->buffer.size()) {
      // send less than the full buffer --> need to make a temporary buffer
      // Make sure this doesn't happen on the critical path!
      pdvector<T_visi::dataValue> temp(num_to_send);
      for (unsigned i = 0; i < num_to_send; i++)
         temp[i] = ptr->buffer[i];
      ptr->visip->Data(temp);
   }
   else
      ptr->visip->Data(ptr->buffer);

   if (ptr->visip->did_error_occur()) {
      PARADYN_DEBUG(("igen: after visip->Data() in VISIthreadDataHandler"));
      ptr->buffer_next_insert_index = 0;
      ptr->quit = 1;
      return;
    }
    ptr->buffer_next_insert_index = 0;
   sampleVal_cerr << "Leaving flush_buffer_if_nonempty\n";
}

void VISIthreadForceFlushBufferCallback() {
  sampleVal_cerr << "VISIthreadForceFlushBufferCallback\n";
  VISIthreadGlobals *ptr;
  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
      PARADYN_DEBUG(("thr_getspecific in VISIthreadDataCallback"));
      ERROR_MSG(13,"thr_getspecific in VISIthread::VISIthreadDataCallback");
      return;
  }
  flush_buffer_if_nonempty(ptr);
  sampleVal_cerr << "Leaving VISIthreadForceFlushBufferCallback\n";
}

// trace data streams
void flush_traceBuffer_if_full( VISIthreadGlobals *ptr) {
   assert(ptr->buffer_next_insert_index <= ptr->traceBuffer.size());
   if (ptr->buffer_next_insert_index != ptr->traceBuffer.size())
      return;

   ptr->visip->TraceData(ptr->traceBuffer);

   if (ptr->visip->did_error_occur()) {
      PARADYN_DEBUG(("igen: after visip->TraceData() in VISIthreadTraceDataHandler"));
      ptr->quit = 1;
      return;
   }
   ptr->buffer_next_insert_index = 0;
}

// trace data streams
void flush_traceBuffer_if_nonempty( VISIthreadGlobals *ptr) {
   const unsigned num_to_send = ptr->buffer_next_insert_index;
   assert(num_to_send <= ptr->traceBuffer.size());

   if (num_to_send == 0){
      ptr->buffer_next_insert_index = 0;
      return;
   }

   if (num_to_send < ptr->traceBuffer.size()) {
      // send less than the full buffer --> need to make a temporary buffer
      // Make sure this doesn't happen on the critical path!
      pdvector<T_visi::traceDataValue> temp(num_to_send);
      for (unsigned i = 0; i < num_to_send; i++)
         temp[i] = ptr->traceBuffer[i];
      ptr->visip->TraceData(temp);
   }
   else
      ptr->visip->TraceData(ptr->traceBuffer);

   if (ptr->visip->did_error_occur()) {
      PARADYN_DEBUG(("igen: after visip->TraceData() in VISIthreadTracceDataHandler"));
      ptr->buffer_next_insert_index = 0;
      ptr->quit = 1;
      return;
    }
    ptr->buffer_next_insert_index = 0;
}

/////////////////////////////////////////////////////////////
//  VISIthreadDataHandler: routine to handle data values from 
//    the datamanger to the visualization  
//
//  adds the data value to it's local buffer and if the buffer
//  is full sends it to the visualization process
/////////////////////////////////////////////////////////////
void VISIthreadDataHandler(metricInstanceHandle mi,
			   int bucketNum,
			   pdSample value,
			   phaseType){
  sampleVal_cerr << "VISIthreadDataHandler-  visiThrd_key: " << visiThrd_key 
		 << "\n"; 
  VISIthreadGlobals *ptr;
  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
      PARADYN_DEBUG(("thr_getspecific in VISIthreadDataCallback"));
      ERROR_MSG(13,"thr_getspecific in VISIthread::VISIthreadDataCallback");
      return;
  }

  if(ptr->start_up || ptr->quit) // process has not been started or has exited 
      return;

  // if visi has not allocated buffer space yet
  if(!(ptr->buffer.size())){  
      return;		    
  }

  if(ptr->buffer_next_insert_index >= ptr->buffer.size()) {
      PARADYN_DEBUG(("buffer_next_insert_index out of range: VISIthreadDataCallback")); 
      ERROR_MSG(16,"buffer_next_insert_index out of range: VISIthreadDataCallback");
      ptr->quit = 1;
      return; 
  }

  // find metricInstInfo for this metricInstanceHandle
  metricInstInfo *info = NULL;
  for(unsigned i=0; i < ptr->mrlist.size(); i++){
      if(ptr->mrlist[i].mi_id == mi){
          info = &(ptr->mrlist[i]);
      }
  }
  if(!info) return;  // just ignore values 

  // add data value to buffer
  T_visi::dataValue &bufferEntry = ptr->buffer[ptr->buffer_next_insert_index++];
  // -----------------------------------------------------
  // delete & cleanup the following after visis/ign are converted

  // This normalizes the value before being sent to the visis.  The visis
  // haven't been converted yet and are expecting the sample values to be
  // normalized.  In the future, the visis will correctly handle the
  // unnormalized sample values.
  metricInstance *minst = metricInstance::getMI(mi);
  metric *met = metric::getMetric(minst->getMetricHandle());
  double divisor = 0.0;

  if(met->getStyle() == SampledFunction) {
    divisor = 1.0;   // sampledFunction metrics aren't normalized
  } else {
    timeLength bucketWidth = minst->getBucketWidth(ptr->args->phase_type);
    double bwidth_ns = bucketWidth.getD(timeUnit::ns());
    sampleVal_cerr << "bucket_width: " << bwidth_ns << ",  width: " 
		   << bucketWidth << "\n";
    divisor = bwidth_ns;
  }
  double fval;
  if(value.isNaN()) {
    fval = make_Nan();
  } else {
    double sample = static_cast<double>(value.getValue());
    fval = static_cast<float>(sample / divisor);
  }

  sampleVal_cerr << "value to visis: " << fval << "\n";
  // -----------------------------------------------------

  bufferEntry.data = fval;
  bufferEntry.metricId = info->m_id;
  bufferEntry.resourceId = info->r_id; 
  bufferEntry.bucketNum = bucketNum;

  sampleVal_cerr << "VISIthreadDataHandler,  adding to buffer - bucket:" 
		 << bucketNum << ",  value: " << value << ", float_val: "
		 << fval << "\n";
  // if buffer is full, send buffer to visualization
  flush_buffer_if_full(ptr);
  info = 0;
}

/////////////////////////////////////////////////////////////
//  VISIthreadDataCallback: Callback routine for DataManager 
//    newPerfData Upcall
//
/////////////////////////////////////////////////////////////
void VISIthreadDataCallback(pdvector<dataValueType> *values,
			    u_int num_values){

  sampleVal_cerr << "VISIthreadDataCallback - num_values: " << num_values 
		 << "\n";

  VISIthreadGlobals *ptr;
  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
      PARADYN_DEBUG(("thr_getspecific in VISIthreadDataCallback"));
      ERROR_MSG(13,"thr_getspecific in VISIthread::VISIthreadDataCallback");
      return;
  }

  if(!ptr) return;

  // if visi process has not been started yet or if visi process has exited 
  if(ptr->start_up || ptr->quit)  return;

  if (values->size() < num_values) num_values = values->size();
  for(unsigned i=0; i < num_values;i++){
      VISIthreadDataHandler((*values)[i].mi,
			    (*values)[i].bucketNum,
			    (*values)[i].value,
			    (*values)[i].type);
  }
  // dealloc buffer space
  datavalues_bufferpool.dealloc(values);

}

/////////////////////////////////////////////////////////////
//  VISIthreadTraceDataHandler: routine to handle trace data values from
//    the datamanger to the visualization
//
//  adds the data value to it's local buffer and if the buffer
//  is full sends it to the visualization process
/////////////////////////////////////////////////////////////
void VISIthreadTraceDataHandler(metricInstanceHandle mi,
                            byteArray value){

  VISIthreadGlobals *ptr;
  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
      PARADYN_DEBUG(("thr_getspecific in VISIthreadDataCallback"));
      ERROR_MSG(13,"thr_getspecific in VISIthread::VISIthreadDataCallback");
      return;
  }

  if(ptr->start_up || ptr->quit) // process has not been started or has exited
      return;

  // if visi has not allocated buffer space yet
  if(!(ptr->traceBuffer.size())){
      return;
  }

  if(ptr->buffer_next_insert_index >= ptr->traceBuffer.size()) {
      PARADYN_DEBUG(("buffer_next_insert_index out of range: VISIthreadTraceDataCallback"));
      ERROR_MSG(16,"buffer_next_insert_index out of range: VISIthreadTraceDataCallback");
      ptr->quit = 1;
      return;
  }

  // find metricInstInfo for this metricInstanceHandle
  metricInstInfo *info = NULL;
  for(unsigned i=0; i < ptr->mrlist.size(); i++){
      if(ptr->mrlist[i].mi_id == mi){
          info = &(ptr->mrlist[i]);
      }
  }
  if(!info) return;  // just ignore values

  // add data value to buffer
T_visi::traceDataValue &traceBufferEntry = ptr->traceBuffer[ptr->buffer_next_insert_index++];
  traceBufferEntry.metricId = info->m_id;
  traceBufferEntry.resourceId = info->r_id;
  traceBufferEntry.traceDataRecord = value;

  //flush_buffer_if_full(ptr);
  flush_traceBuffer_if_full(ptr);
  if (!value.length())
    flush_traceBuffer_if_nonempty(ptr);

  info = 0;
}


/////////////////////////////////////////////////////////////
//  VISIthreadTraceDataCallback: Callback routine for DataManager
//    newTracePerfData Upcall
//
/////////////////////////////////////////////////////////////
void VISIthreadTraceDataCallback(perfStreamHandle ,
                            metricInstanceHandle ,
                            timeStamp *timeStPtr,
                            int num_values,
                            void *values){

  delete timeStPtr;
  VISIthreadGlobals *ptr;
  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
      PARADYN_DEBUG(("thr_getspecific in VISIthreadTraceDataCallback"));
      ERROR_MSG(13,"thr_getspecific in VISIthread::VISIthreadTraceDataCallback");
      return;
  }

  if(!ptr) return;

  // if visi process has not been started yet or if visi process has exited
  if(ptr->start_up || ptr->quit)  return;

    pdvector<traceDataValueType> *traceValues =
      (pdvector<traceDataValueType> *)values;
  if (traceValues->size() < (u_int)num_values) num_values = traceValues->size();
  for(int i=0; i < num_values;i++){
      VISIthreadTraceDataHandler((*traceValues)[i].mi,
                            (*traceValues)[i].traceRecord);
  }
  tracedatavalues_bufferpool.dealloc(traceValues);

}

/////////////////////////////////////////////////////////
//  VISIthreadnewMetricCallback: callback for dataManager
//    newMetricDefined Upcall
//    (not currently implemented)  
/////////////////////////////////////////////////////////
void VISIthreadnewMetricCallback(perfStreamHandle,
				 const char *,
				 int,
				 int,
				 const char *,
				 metricHandle,
				 dm_MetUnitsType){
 VISIthreadGlobals *ptr;

  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
    PARADYN_DEBUG(("thr_getspecific in VISIthreadnewMetricCallback"));
    ERROR_MSG(13,"thr_getspecific in VISIthread::VISIthreadnewMetricCallback");
    return;
  }
  if(ptr->start_up || ptr->quit)  // visi process has not started or exiting 
    return;
}

///////////////////////////////////////////////////////////
//  VISIthreadnewResourceCallback: callback for dataManager
//    newResourceDefined Upcall 
//    (not currently implemented)  
//////////////////////////////////////////////////////////
void VISIthreadnewResourceCallback(perfStreamHandle,
				   resourceHandle,
			           resourceHandle, 
				   const char *,
				   const char *){

  VISIthreadGlobals *ptr;
  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
   PARADYN_DEBUG(("thr_getspecific in VISIthreadnewResourceCallback"));
   ERROR_MSG(13,"thr_getspecific in VISIthread::VISIthreadnewResourceCallback");
   return;
  }

  if(ptr->start_up || ptr->quit)  // process not started or exiting 
    return;

}

///////////////////////////////////////////////////////
//  VISIthreadFoldCallback: callback for dataManager
//     histFold upcall
//
//  if thread's local data buffer is not empty send
//  the data buffer to the visualization process
//  before sending Fold msg to visi process  
///////////////////////////////////////////////////////
// TODO: do something with this phase_type info
void VISIthreadFoldCallback(perfStreamHandle,
			    timeLength *_newWidthPtr, phaseType phase_type){
 timeLength width = *_newWidthPtr;
 delete _newWidthPtr;
 VISIthreadGlobals *ptr;

  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
     PARADYN_DEBUG(("thr_getspecific in VISIthreadFoldCallback"));
     ERROR_MSG(13,"thr_getspecific in VISIthread::VISIthreadFoldCallback");
     return;
  }

  if(ptr->start_up || ptr->quit) // process has not been started or has exited 
     return;

  
  if ((ptr->buffer_next_insert_index >= ptr->buffer.size()) && 
	ptr->buffer.size()){   
     PARADYN_DEBUG(("buffer_next_insert_index out of range: VISIthreadFoldCallback")); 
     ERROR_MSG(16,"buffer_next_insert_index out of range: VISIthreadFoldCallback");
     ptr->quit = 1;
     return;  
  }

  // ignore folds for other phase types 
  if(ptr->args->phase_type != phase_type) return;
  // ignore folds for other phase data...this is an old phase visi
  if(ptr->currPhaseHandle != -1) {
      if((phase_type != GlobalPhase) && 
	 (ptr->args->my_phaseId != ((u_int)ptr->currPhaseHandle))) return;
  }

  // if new Width is same as old width ignore Fold 
  if(ptr->bucketWidth != width){
     // if buffer is not empty send visualization buffer of data values
     flush_buffer_if_nonempty(ptr);

     ptr->bucketWidth = width;
     // call visualization::Fold routine
     ptr->visip->Fold(width.getD(timeUnit::sec()));
     if(ptr->visip->did_error_occur()){
        PARADYN_DEBUG(("igen: after visip->Fold() in VISIthreadFoldCallback"));
        ptr->quit = 1;
        return;
     }
  }

}

///////////////////////////////////////////////////////
//  VISIthreadInitActualValCallback: callback for dataManager
//     histFold upcall
//
//  if thread's local data buffer is not empty send
//  the data buffer to the visualization process
//  before sending Fold msg to visi process  
///////////////////////////////////////////////////////
// TODO: do something with this phase_type info
void VISIthreadInitActualValCallback(perfStreamHandle, metricInstanceHandle mi,
				     pdSample *_newInitActValPtr, phaseType) {
 pdSample initActVal = *_newInitActValPtr;
 delete _newInitActValPtr;
 VISIthreadGlobals *ptr;
 sampleVal_cerr << "VISIthreadInitActualValCallback- mi: " << mi << ", val: " 
		<< initActVal << "\n";

  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
     PARADYN_DEBUG(("thr_getspecific in VISIthreadFoldCallback"));
     ERROR_MSG(13,"thr_getspecific in VISIthread::VISIthreadFoldCallback");
     return;
  }

  if(ptr->start_up || ptr->quit) // process has not been started or has exited 
     return;

  // find metricInstInfo for this metricInstanceHandle
  metricInstInfo *info = NULL;
  for(unsigned i=0; i < ptr->mrlist.size(); i++){
      if(ptr->mrlist[i].mi_id == mi){
          info = &(ptr->mrlist[i]);
      }
  }
  if(!info) {
    return;
  }
  int resourceId = info->r_id;
  int metricId   = info->m_id;

  // This normalizes the value before being sent to the visis.  The visis
  // haven't been converted yet and are expecting the sample values to be
  // normalized.  In the future, the visis will correctly handle the
  // unnormalized sample values.
  metricInstance *minst = metricInstance::getMI(mi);
  metric *met = metric::getMetric(minst->getMetricHandle());
  double divisor = 0.0;

  if(met->getStyle() == SampledFunction) {
    divisor = 1.0;   // sampledFunction metrics aren't normalized
  } else {
    timeLength bucketWidth = minst->getBucketWidth(ptr->args->phase_type);
    double bwidth_ns = bucketWidth.getD(timeUnit::ns());
    sampleVal_cerr << "bucket_width: " << bwidth_ns << ",  width: " 
		   << bucketWidth << "\n";
    divisor = bwidth_ns;
  }
  double sample = static_cast<double>(initActVal.getValue());
  double fval = static_cast<float>(sample / divisor);
  ptr->visip->setInitialActualValue(metricId, resourceId, fval);

  if(ptr->visip->did_error_occur()){
    PARADYN_DEBUG(("igen: after visip->setInitActVal() in VISIthreadInitActValCallback"));
    ptr->quit = 1;
    return;
  }

}


///////////////////////////////////////////////////////
//  VISIthreadPhaseCallback: callback for dataManager
//     new phase definition upcall 
//
///////////////////////////////////////////////////////
void VISIthreadPhaseCallback(perfStreamHandle, 
			     const char *name,
			     phaseHandle handle,
			     relTimeStamp *beginPtr,
			     relTimeStamp *endPtr,
			     timeLength *bucketWidthPtr,
			     bool, bool){
   relTimeStamp begin = *beginPtr;
   relTimeStamp end = *endPtr;
   timeLength bucketWidth = *bucketWidthPtr;
   delete beginPtr;
   delete endPtr;
   delete bucketWidthPtr;

   VISIthreadGlobals *ptr;
   if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
      PARADYN_DEBUG(("thr_getspecific in VISIthreadPhaseCallback"));
      ERROR_MSG(13,"thr_getspecific in VISIthread::VISIthreadPhaseCallback");
      return;
   }

   if(ptr->start_up || ptr->quit)  // process not been started yet or exiting 
     return;

   // send visi phase end call for current phase
   // if(ptr->currPhaseHandle != -1)
   double beginD = begin.getD(timeUnit::sec());
   double endD = end.getD(timeUnit::sec());
   ptr->visip->PhaseEnd(beginD, ptr->currPhaseHandle);

   ptr->currPhaseHandle = handle;

   // send visi phase start call for new phase
   double bucketWidthD = bucketWidth.getD(timeUnit::sec());
   ptr->visip->PhaseStart(beginD, endD, bucketWidthD, name, handle);
}


/////////////////////////////////////////////////////////////
// Start the visualization process:  this is called when the
// first set of valid metrics and resources is enabled
///////////////////////////////////////////////////////////
int VISIthreadStartProcess(){
  VISIthreadGlobals *ptr;

  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
    PARADYN_DEBUG(("thr_getspecific in VISIthreadStartProcess"));
    ERROR_MSG(13,"Error in VISIthreadStartProcess: thr_getspecific");
    return(0);
  }

  // start the visualization process
  if(ptr->start_up){
	PDSOCKET	visiSock;


    PARADYN_DEBUG(("start_up in VISIthreadStartProcess"));
    pdvector<pdstring> av;
    // start at 1 since RPCprocessCreate will add 0th arg to list
    unsigned index=1;
    while(ptr->args->argv[index]) {
      av += ptr->args->argv[index];
      index++;
    }
    av += "--paradyn";      // let visi know it is being started by Paradyn
    visiSock = RPCprocessCreate("localhost","",ptr->args->argv[0],"", av);

    if (visiSock == PDSOCKET_ERROR) {
      PARADYN_DEBUG(("Error in process Create: RPCprocessCreate"));
      ERROR_MSG(14,"");
      ptr->quit = 1;
      return(0);
    }

    ptr->visip = new visiUser(visiSock); 
    if (ptr->visip->errorConditionFound) {
      ERROR_MSG(14,"");
      ptr->quit = 1;
      return(0);
    }

    if(msg_bind_socket(visiSock,0,
		(int(*)(void*)) xdrrec_eof,ptr->visip->net_obj(), &(ptr->vtid)) != THR_OKAY) {
      PARADYN_DEBUG(("Error in msg_bind_socket(ptr->fd)"));
      ERROR_MSG(14,"");
      ptr->quit = 1;
      return(0);
    }

  }
  ptr->start_up = 0;  // indicates that process has been started
  return(1);
}

///////////////////////////////////////////////////////////////////
// make an enable request to the DM if the enable limit hasn't been
// reached and if there are still things to enable
///////////////////////////////////////////////////////////////////
bool VISIMakeEnableRequest(){
  VISIthreadGlobals *ptr;

  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
    PARADYN_DEBUG(("thr_getspecific in MakeEnableRequest"));
    ERROR_MSG(13,"Error in VISIMakeEnableRequest: thr_getspecific");
    return(0);
  }
  
  if(!(ptr->request)) return false;

  if(ptr->next_to_enable >= ptr->request->size()) return false;

  // check to see if the limit has been reached
  if(ptr->args->mi_limit > 0){
      if(ptr->args->mi_limit <= (int)ptr->mrlist.size()){
          pdstring msg("A visi has enabled the maximum number of metric/focus ");
          msg += pdstring("pairs that it can enable. limit = ");
          msg += pdstring(ptr->args->mi_limit); 
          msg += pdstring("\n");
          uiMgr->showError(97,P_strdup(msg.c_str()));
          // clean up state
          
          if(ptr->request) delete ptr->request;
          if(ptr->retryList) delete ptr->retryList;
          ptr->request = 0;
          ptr->retryList = 0;
          ptr->next_to_enable = 0;
          ptr->first_in_curr_request = 0;
          return false;
      }
  }

  // We used to "batch" requests sent by the visi thread to the data manager/
  // daemon. This has caused problems with the new "bundle" abstraction, and is
  // being disabled. If there are problems, check here. 

#if 0
  // get the TC value for the maximum packet size for an enable
  // request to the DM
  tunableFloatConstant packetSizeTC =
  tunableConstantRegistry::findFloatTunableConstant("EnableRequestPacketSize");
  u_int request_size = (u_int)packetSizeTC.getValue();
  if(request_size == 0) request_size = 2;
#endif
  u_int request_size = (u_int)-1;

  // there is an enable limit for this visi: adjust the request_size 
  if(ptr->args->mi_limit > 0) {
      request_size = (ptr->args->mi_limit - ptr->mrlist.size()); 
  }

  if((request_size == (u_int)-1) || 
     (request_size > (ptr->request->size() - ptr->next_to_enable))) {
      request_size = (ptr->request->size() - ptr->next_to_enable); 
  }

  // create the request vector of pairs and make enable request to DM
  pdvector<metric_focus_pair> *metResParts = 
		             new pdvector<metric_focus_pair>(request_size);
  assert(request_size == metResParts->size());

  for(u_int i=0; i < request_size; i++){
      (*metResParts)[i] = (*(ptr->request))[ptr->next_to_enable+i]; 
  }
  ptr->first_in_curr_request = ptr->next_to_enable;
  ptr->next_to_enable += request_size;
  
  ptr->dmp->enableDataRequest(ptr->ps_handle, ptr->pt_handle, metResParts,0,
			      ptr->args->phase_type,
			      ptr->args->my_phaseId,0,0,0);
  return true;
}


///////////////////////////////////////////////////////////////////
//  VISIthreadchooseMetRes: callback for User Interface Manager 
//    chooseMetricsandResources upcall
//    input: list of metric/focus matrices 
//
//  If there is not an enable request already in progress for this
//  visi, then update request info. and call VISIMakeEnableRequest
//  Otherwise, just update request info. (a call to VISIMakeEnableRequest
//  will occur when the response to the request in progress is received)
///////////////////////////////////////////////////////////////////
int VISIthreadchooseMetRes(pdvector<metric_focus_pair> *newMetRes){

    if(newMetRes)
       PARADYN_DEBUG(("In VISIthreadchooseMetRes size = %d",newMetRes->size()));
    
    VISIthreadGlobals *ptr;
    if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
        PARADYN_DEBUG(("thr_getspecific in VISIthreadchooseMetRes"));
        ERROR_MSG(13,"thr_getspecific VISIthread::VISIthreadchooseMetRes");
        return 1;
    }

    // parse globals->args->matrix, merge into newMetRes
    if (ptr->args->matrix)
    {
       if (newMetRes == NULL)
          newMetRes = new pdvector<metric_focus_pair>;
       //*newMetRes += *ptr->args->matrix;
       for (unsigned i=0;i<ptr->args->matrix->size();i++)
       {
          metric_focus_pair metfocus=(*ptr->args->matrix)[i];
          int	found=0;
          for (unsigned j=0;j<newMetRes->size();j++)
             if (metfocus.met == (*newMetRes)[j].met && metfocus.res.size() == (*newMetRes)[j].res.size())
             {
                unsigned k=0;
                for (k=0;k<metfocus.res.size();k++)
                   if (metfocus.res[k] != (*newMetRes)[j].res[k])
                      break;
                if (k == metfocus.res.size())
                {
                   found = 1;
                   break;
                }
             }
          if (found == 0)
             *newMetRes += metfocus;
       }
    }

    // there is not an enable currently in progress
    if(!(ptr->request)){ 
        // check for invalid reply ==> user picked "Cancel" menu option
        if(newMetRes == 0){
            if(ptr->start_up){
                ptr->quit = 1;
            }
            return 1;
        }
        else {
            ptr->request = newMetRes;
            newMetRes = 0;
            ptr->next_to_enable = 0;
            ptr->first_in_curr_request = 0;
        }
        if(!VISIMakeEnableRequest()){ 
            assert(!(ptr->request));
            assert(!(ptr->retryList));
            assert(!(ptr->next_to_enable));
            assert(!(ptr->first_in_curr_request));
        }
    }
    else { // add new elements to request list
       // check for invalid reply ==> user picked "Cancel" menu option
       if(newMetRes == 0) {
          return 1;
       }
       *(ptr->request) += *newMetRes;
       newMetRes = 0;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////
// send new set of metric/foci to visi along with old data buckets for
// each pair
///////////////////////////////////////////////////////////////////
bool VISISendResultsToVisi(VISIthreadGlobals *ptr,u_int numEnabled){

   // create a visi_matrix_Array to send to visualization
   pdvector<T_visi::visi_matrix> pairList;
   // the newly enabled pairs are the last numEnabled in the list
   u_int start = ptr->mrlist.size() - numEnabled; 
   assert((start+numEnabled) == ptr->mrlist.size());

   for(unsigned i=start; i < ptr->mrlist.size(); i++){
      T_visi::visi_matrix matrix;
      matrix.met.Id = ptr->mrlist[i].m_id;
      matrix.met.name = ptr->mrlist[i].metric_name; 
      matrix.met.curr_units = ptr->mrlist[i].metric_units;
      matrix.met.tot_units = ""; // let visilib figure out total units
      if(ptr->mrlist[i].units_type == UnNormalized){
	      matrix.met.unitstype = 0;
      }
      else if (ptr->mrlist[i].units_type == Normalized){
	      matrix.met.unitstype = 1;
      }
      else{
	      matrix.met.unitstype = 2;
      }
      matrix.met.aggregate = AVE;
      matrix.res.Id = ptr->mrlist[i].r_id;
      if((matrix.res.name = 
          AbbreviatedFocus(ptr->mrlist[i].focus_name.c_str()))
	      ==0){
	      ERROR_MSG(12,"in VISIthreadchooseMetRes");
	      ptr->quit = 1;
	      return false;
      }
      pairList += matrix;
   }


   PARADYN_DEBUG(("before call to AddMetricsResources\n"));
   if(ptr->args->phase_type == GlobalPhase){
      timeLength bwidth;
      ptr->dmp->getGlobalBucketWidth(&bwidth);
      ptr->visip->AddMetricsResources(pairList,
                                      bwidth.getD(timeUnit::sec()),
                                      ptr->dmp->getMaxBins(),
                                      ptr->args->start_time.getD(timeUnit::sec()),
                                      -1);
   }
   else {
      timeLength bwidth;
      ptr->dmp->getCurrentBucketWidth(&bwidth);
      ptr->visip->AddMetricsResources(pairList,
                                      bwidth.getD(timeUnit::sec()),
                                      ptr->dmp->getMaxBins(),
                                      ptr->args->start_time.getD(timeUnit::sec()),
                                      ptr->args->my_phaseId);
   }
   if(ptr->visip->did_error_occur()){
      PARADYN_DEBUG(("igen: visip->AddMetsRess(): VISIthreadchooseMetRes"));
      ptr->quit = 1;
      return false;
   }

   // get old data bucket values for new metric/resources and
   // send them to visualization
   pdSample *buckets = new pdSample[1001];
   for(unsigned q = start; q < ptr->mrlist.size(); q++){
      int howmany = ptr->dmp->getSampleValues(ptr->mrlist[q].mi_id,
                                              buckets,1000,0,
                                              ptr->args->phase_type);
      // send visi all old data bucket values
      if(howmany > 0){
	      pdvector<float> bulk_data;
	      for (u_int ve=0; ve< ((u_int)howmany); ve++){
            // -----------------------------------------------------
            // delete & cleanup the following after visis/ign are converted
            int _mi = ptr->mrlist[q].mi_id;
            metricInstance *minst = metricInstance::getMI(_mi);
            metric *met = metric::getMetric(minst->getMetricHandle());
            double divisor = 0.0;
            if(met->getStyle() == SampledFunction) {
               divisor = 1.0;
            } else {
               timeLength bucketWidth = minst->getBucketWidth(
                                                              ptr->args->phase_type);
               double bwidth_ns = bucketWidth.getD(timeUnit::ns());
               sampleVal_cerr << "BDbucket_width: " << bwidth_ns 
                              << ",  width: " << bucketWidth << "\n";
               divisor = bwidth_ns;
            }
            double fval;
            if(buckets[ve].isNaN()) {
               fval = make_Nan();
            } else {
               double sample = static_cast<double>(buckets[ve].getValue());
               fval = static_cast<float>(sample / divisor);
            }
            sampleVal_cerr << "val to visis: " << fval << "\n";
            bulk_data += fval;
            //bulk_data += static_cast<float>(buckets[ve].getValue());
            // -----------------------------------------------------

         }
         ptr->visip->BulkDataTransfer(bulk_data, (int)ptr->mrlist[q].m_id,
                                      (int)ptr->mrlist[q].r_id);
         if(ptr->visip->did_error_occur()){
            PARADYN_DEBUG(("igen:vp->BulkDataTransfer():VISIthreadchoose"));
            ptr->quit = 1;
            return false;
         }
	      bulk_data.resize(0);
      }
   }
   return true;
}

///////////////////////////////////////////////////////////////////
// Callback routine for enable data calls to the DM
//     response: the response vector from the DM corr. to the request
//     
// Checks to see if something new has been enabled, if so it sends new
// pairs to visi process (starting visi process first if it has not already
// done so).  If there are more things to enable, then a call to 
// VISIMakeEnableRequest is made, otherwise a remenuing call could be made
// to the UI with all pairs that were requested, but not enabled
///////////////////////////////////////////////////////////////////
void VISIthreadEnableCallback(pdvector<metricInstInfo> *response, u_int,
                              u_int last_cb_for_request) {
    VISIthreadGlobals *ptr;
    if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
        PARADYN_DEBUG(("thr_getspecific in VISIthreadEnableCallback"));
        ERROR_MSG(13,"thr_getspecific VISIthread::VISIthreadEnableCallback");
        return;
    }
    
    // for each successfully enabled pair, check to see if it is newly enabled
    // if it is not successfully enabled add it to the retry list
    u_int numEnabled = 0; // number enalbed

    for(u_int i=0; i < response->size(); i++){
       const metricInstInfo &curmsg = (*response)[i];

       // Decided to mark these records as processed instead of deleting
       // them.  I have in mind the unsafe situation of the dataMgr thread
       // adding to this array, because an enableDataCallback came in from a
       // daemon, at the same time that we're deleting an entry from this
       // array.
       if(curmsg.deferred)
       {
#if READY
          // show which m/f pair was disabled?
#else
          pdstring statusMsg = "For\n    ";
          statusMsg += curmsg.metric_name;
          statusMsg += ": ";
          statusMsg += curmsg.focus_name;
          statusMsg += "\n";
          uiMgr->showError(128,P_strdup(statusMsg.c_str()));
#endif // READY
          continue;
       }
       else if (curmsg.successfully_enabled) {
          bool found = false;
          for(u_int j=0; j < ptr->mrlist.size(); j++){
             if(curmsg.mi_id == ptr->mrlist[j].mi_id){
                found = true;
                break;
             } }
          if(!found){ // this is a new metric/focus pair
             ptr->mrlist += curmsg;
             numEnabled++;
          }
       }
       else { // add it to retry list
          if(ptr->retryList){
             *(ptr->retryList) += 
                (*ptr->request)[i+ptr->first_in_curr_request];
          }
          else {
             ptr->retryList = new pdvector<metric_focus_pair>;
             *(ptr->retryList) += 
                (*ptr->request)[i+ptr->first_in_curr_request];
          }
       }
    }
    // allocated in metricFocusReqBundle::flushPerfStreamMsgs()
    delete response;  // all done with response vector

    // if something was enabled and if process is not started, then start it
    if(numEnabled > 0){
       // increase the buffer size
       flush_buffer_if_nonempty(ptr);
       
       // trace data streams
       flush_traceBuffer_if_nonempty(ptr);
       
       unsigned nbufs = ptr->buffer.size() + numEnabled;
       unsigned lim = (unsigned)DM_DATABUF_LIMIT;
       unsigned newMaxBufferSize = (nbufs < lim) ? (nbufs) : (lim);
       ptr->buffer.resize(newMaxBufferSize); // new
       
       // trace data streams
       ptr->traceBuffer.resize(10); // new
       
       // if this is the first set of enabled values, start visi process
       if(ptr->start_up){
          if(!VISIthreadStartProcess()){
             ptr->quit = 1;
             return;
          }
       }
       assert(!ptr->start_up);
       
       // send new set of metric/focus pairs to visi
       if(!VISISendResultsToVisi(ptr,numEnabled)){
          cout << "error after call to VISISendResultsToVisi\n";
       }
    }

    // if we have reached the limit display limit msg and clean-up state
    // else if there are more things to enable get the next set
    // else clean up state and if the retryList is non-empty send msgs
    
    if((!ptr->request) || ptr->next_to_enable < ptr->request->size()) {
       if((ptr->args->mi_limit > 0) &&  // this visi has a limit 
          ((int)(ptr->mrlist.size()) >= ptr->args->mi_limit)){ // limit reached 
           
          pdstring msg("A visi has enabled the maximum number of metric/");
          msg += pdstring("focus pairs that it can enable. Some pairs may ");
          msg += pdstring("not have been enabled.  limit =  ");
          msg += pdstring(ptr->args->mi_limit); 
          msg += pdstring("\n");
          uiMgr->showError(97,P_strdup(msg.c_str()));
          // clean up state
          if (ptr->request) delete ptr->request;
          if (ptr->retryList) delete ptr->retryList;
          ptr->next_to_enable = 0;
          ptr->first_in_curr_request = 0;
          ptr->request = 0;
          ptr->retryList = 0;
       }
       else { // try to enable more
          if(!VISIMakeEnableRequest()) 
             cout << "error after call to VISIMakeEnableRequest\n";
       }       
    }

    if(last_cb_for_request) {
       // if remenuFlag is set and retry list is not empty: remenu
       if((ptr->args->remenuFlag)&&ptr->retryList&&(ptr->retryList->size())){
          // don't free retryList since it is passed to UI
          pdstring msg("Cannot enable the following metric/focus pair(s):\n");
          for (u_int ii=0; ii < ptr->retryList->size(); ii++) {
             pdstring *focusName=NULL;
             focusName = new 
                pdstring(dataMgr->getFocusName(&((*ptr->retryList)[ii].res)));
             msg +=  
                pdstring(dataMgr->getMetricName((*ptr->retryList)[ii].met));
             if (focusName) {
                msg += pdstring("(");
                msg += pdstring(AbbreviatedFocus((*focusName).c_str()));
                msg += pdstring(")");
             }
             msg += pdstring(" ");
          }
          ptr->ump->chooseMetricsandResources(VISIthreadchooseMetRes,
                                              ptr->retryList);
          ptr->ump->showError(86,P_strdup(msg.c_str()));
       }
       else if (ptr->start_up && (!ptr->mrlist.size())) { 
          // if nothing was ever enabled, and remenuflag is not set quit
          ptr->quit = 1;
          if(ptr->retryList) delete ptr->retryList;
       }

       // clean up state
       
       if(ptr->request) delete ptr->request;
       ptr->next_to_enable = 0;
       ptr->first_in_curr_request = 0;
       ptr->request = 0;
       ptr->retryList = 0;
    }
}

////////////////////////////////////////////////////////
//  VISIthreadshowMsgREPLY: callback for User Interface 
//    Manager showMsgREPLY upcall (not currently implemented) 
///////////////////////////////////////////////////////
void VISIthreadshowMsgREPLY(int){
}

////////////////////////////////////////////////////////
//  VISIthreadshowErrorREPLY: callback for User Interface 
//    Manager showErrorREPLY upcall (not currently implemented)
///////////////////////////////////////////////////////
void VISIthreadshowErrorREPLY(int){
}

///////////////////////////////////////////////////////////////////
//  VISIthread main routine
//  input: parent thread tid, visualization command line arguments
//
//  initializes thread local variables, starts visualization process
//  and enters main loop
///////////////////////////////////////////////////////////////////
void *VISIthreadmain(void *vargs){ 

    VISIthreadArgs* targs = (VISIthreadArgs*)vargs;
    assert( targs != NULL );

   //initialize global variables
   VISIthreadGlobals *globals;
   globals = new VISIthreadGlobals;

   VISIthread *vtp = new VISIthread( targs->vmTid );
   globals->ump = uiMgr;
   globals->vmp = vmMgr;
   globals->dmp = dataMgr;
   globals->args = targs;
   globals->visip = NULL;     // assigned value in VISIthreadStartProcess 
   globals->currPhaseHandle = -1;

   globals->start_up = 1;
   globals->fd_first = 0;

   // globals->buffer is left a 0-sized array
   globals->buffer_next_insert_index = 0;

   globals->vtid = THR_TID_UNSPEC;
   globals->quit = 0;
   globals->bucketWidth = globals->args->bucketWidth;

   globals->request = 0;
   globals->retryList = 0;
   globals->num_Enabled = 0;
   globals->next_to_enable = 0;
   globals->first_in_curr_request = 0;

   // set control callback routines 
   controlCallback callbacks;
   memset(&callbacks, '\0', sizeof(controlCallback));
   callbacks.mFunc = VISIthreadnewMetricCallback;
   callbacks.rFunc = VISIthreadnewResourceCallback; 
   callbacks.retireFunc = 0;
   callbacks.fFunc = VISIthreadFoldCallback;
   callbacks.avFunc= VISIthreadInitActualValCallback;
   callbacks.pFunc = VISIthreadPhaseCallback;
   callbacks.sFunc = 0;
   callbacks.bFunc = 0;
   callbacks.cFunc = 0;
   callbacks.eFunc = VISIthreadEnableCallback;
   callbacks.flFunc= VISIthreadForceFlushBufferCallback;

   PARADYN_DEBUG(("before create performance stream in visithread"));

   // create performance stream
   union dataCallback dataHandlers;
   dataHandlers.sample = VISIthreadDataCallback;
   if((globals->ps_handle = globals->dmp->createPerformanceStream(
                                                                  Sample,dataHandlers,callbacks)) == 0){
      PARADYN_DEBUG(("Error in createPerformanceStream"));
      ERROR_MSG(15,"Error in VISIthreadchooseMetRes: createPerformanceStream");
      globals->quit = 1;
   }

   PARADYN_DEBUG(("perf. stream = %d in visithread",(int)globals->ps_handle));

   // trace data streams
   // set control tracecallback routines
   controlCallback tracecallbacks;
   memset(&tracecallbacks, '\0', sizeof(controlCallback));
   tracecallbacks.mFunc = VISIthreadnewMetricCallback;
   tracecallbacks.rFunc = VISIthreadnewResourceCallback;
   tracecallbacks.retireFunc = 0;
   tracecallbacks.fFunc = 0;
   tracecallbacks.pFunc = 0;
   tracecallbacks.sFunc = 0;
   tracecallbacks.bFunc = 0;
   tracecallbacks.cFunc = 0;
   tracecallbacks.eFunc = VISIthreadEnableCallback;
   tracecallbacks.flFunc= 0;

   PARADYN_DEBUG(("before create performance stream in visithread"));

   // create trace performance stream
   union dataCallback traceDataHandlers;
   traceDataHandlers.trace = VISIthreadTraceDataCallback;
   if((globals->pt_handle = globals->dmp->createPerformanceStream(
                                                                  Trace,traceDataHandlers,tracecallbacks)) == 0){
      PARADYN_DEBUG(("Error in createPerformanceStream"));
      ERROR_MSG(15,"Error in VISIthreadchooseMetRes: createPerformanceStream");
      globals->quit = 1;
   }

   PARADYN_DEBUG(("perf. stream = %d in visithread",(int)globals->pt_handle));


   if (thr_setspecific(visiThrd_key, globals) != THR_OKAY) {
      PARADYN_DEBUG(("Error in thr_setspecific"));
      ERROR_MSG(14,"Error in VISIthreadmain: thr_setspecific");
      globals->quit = 1;
   }

   // if forceProcessStart is set, start visi process and skip menuing
   // and parsing of initial set of metrics and resources
   if( globals->args->forceProcessStart){

      if (globals->args->matrix){
         if (VISIthreadchooseMetRes(NULL) == 1)
            globals->quit = 1;
         // start visi process
      }else if(!VISIthreadStartProcess()){
         globals->quit = 1;
      }
   }
   // to determine if menuing needs to be done.  If so, call UIM rouitine
   // chooseMetricsandResources before entering main loop, if not, call
   // AddMetricsResources routine with metric and focus pointers (these
   // need to be created from the metricList and resourceList) 
   // until parsing routine is in place call chooseMetricsandResources
   // with NULL metric and resource pointers
   else{

      // call get metrics and resources with first set
      globals->ump->chooseMetricsandResources(VISIthreadchooseMetRes, NULL);
   }

 
   PARADYN_DEBUG(("before enter main loop"));
   while(!(globals->quit)){
      if (globals->visip) {
         // visip may not have been set yet
         // see if any async upcalls have been buffered
         while (globals->visip->buffered_requests()) {
            if (globals->visip->process_buffered() == T_visi::error) {
               cout << "error on visi\n";
               assert(0);
            }
         }
      }
      thread_t from = THR_TID_UNSPEC;
      tag_t tag = MSG_TAG_ANY;
      if( msg_poll_preference(&from, &tag, 1,globals->fd_first) != THR_OKAY )
      {
			// TODO
         cerr << "Error in VISIthreadmain.C\n";
         assert(0);
      }
      globals->fd_first = !globals->fd_first;

      if (globals->ump->isValidTag((T_UI::message_tags)tag)) {
         if (globals->ump->waitLoop(true, (T_UI::message_tags)tag) ==
             T_UI::error) {
            // TODO
            cerr << "Error in VISIthreadmain.C\n";
            assert(0);
         }
      } else if (globals->vmp->isValidTag((T_VM::message_tags)tag)) {
         if (globals->vmp->waitLoop(true, (T_VM::message_tags)tag) ==
             T_VM::error) {
            // TODO
            cerr << "Error in VISIthreadmain.C\n";
            assert(0);
         }
      } else if (globals->dmp->isValidTag((T_dataManager::message_tags)tag)) {
         if (globals->dmp->waitLoop(true, (T_dataManager::message_tags)tag) ==
             T_dataManager::error) {
            // TODO
            cerr << "Error in VISIthreadmain.C\n";
            assert(0);
         }
      } else if (tag == MSG_TAG_SOCKET){
         assert(globals->visip);

#if defined(os_windows)
            unsigned long nBytesAvailable = 0;
            if( (ioctlsocket( thr_socket(from), FIONREAD, &nBytesAvailable ) == SOCKET_ERROR) ||
            (nBytesAvailable == 0) )
            {
                continue;
            }
#endif // defined(os_windows)
         assert(thr_socket(from) == globals->visip->get_sock());
         if (globals->visip->waitLoop() == T_visi::error) {
            PARADYN_DEBUG(("igen: visip->awaitResponce() in VISIthreadmain"));
            globals->quit = 1;
         }
         // see if any async upcalls have been buffered
         while (globals->visip->buffered_requests()) {
            if (globals->visip->process_buffered() == T_visi::error) {
               cout << "error on visi\n";
               assert(0);
            }
         }
      } else if (vtp->isValidTag((T_VISIthread::message_tags)tag)) {
         if (vtp->waitLoop(true, (T_VISIthread::message_tags)tag) ==
             T_VISIthread::error) {
            // TODO
            cerr << "Error in VISIthreadmain.C\n";
            assert(0);
         }
      } else {
         cerr << "Unrecognized message in VISIthreadmain.C\n";
         assert(0);
      }
   }
   PARADYN_DEBUG(("leaving main loop"));

   // disable all metricInstance data collection
   for(unsigned i =0; i < globals->mrlist.size(); i++){
      metricInstanceHandle handle = globals->mrlist[i].mi_id;
      globals->dmp->disableDataCollection(globals->ps_handle,
                                          globals->pt_handle,
                                          handle,
                                          globals->args->phase_type);
      // trace data streams
      globals->dmp->disableDataCollection(globals->ps_handle,
                                          globals->pt_handle,
                                          handle,
                                          globals->args->phase_type);

   }

   PARADYN_DEBUG(("before destroy perfomancestream"));
   if(!(globals->dmp->destroyPerformanceStream(globals->ps_handle))){
      ERROR_MSG(16,"remove() in VISIthreadmain");
   }

   // trace data streams
   if(!(globals->dmp->destroyPerformanceStream(globals->pt_handle))){
      ERROR_MSG(16,"remove() in VISIthreadmain");
   }


   // notify UIM that VISIthread is dying 
   globals->ump->threadExiting();

   // notify VM 
   PARADYN_DEBUG(("before notify VM of thread died"));
   globals->vmp->VMVisiDied(thr_self());
   PARADYN_DEBUG(("after notify VM of thread died"));

   // unbind file descriptor associated with visualization
   if(!globals->start_up){
      if(msg_unbind(globals->vtid) == THR_ERR){
         PARADYN_DEBUG(("Error in msg_unbind(globals->vtid)"));
         ERROR_MSG(14,"Error in VISIthreadmain: msg_unbind");
      }
      delete globals->visip;
   }

   PARADYN_DEBUG(("leaving visithread main"));
   thr_exit((void *)0);

   return((void *)0);
}



// TODO: change the printf stmts to error no.s and calls to UIM 
// error reporting routine
///////////////////////////////////////////////////////////////////
//  error handler for igen errors.  For all igen error types, the 
//  visithread kills the visualization process, cleans up its state
//  and dies.  Igen errors between a visualization process and its
//  visithread will not result in the paradyn process dying. 
///////////////////////////////////////////////////////////////////
void visiUser::handle_error()
{
  VISIthreadGlobals *ptr;

   if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
      PARADYN_DEBUG(("thr_getspecific in handle_error"));
      ERROR_MSG(13,"thr_getspecific in visiUser::handle_error");
   }

  // err_state is set by the event that caused the error
  switch (get_err_state()) {
      case igen_no_err:
         fprintf(stderr,"Handle error called for igen_no_err tid = %d\n",
		 thr_self());
         break;
      case igen_encode_err:
      case igen_decode_err:
      case igen_call_err:
      case igen_request_err:
      case igen_send_err:
      case igen_read_err:
      default:
         ptr->quit = 1;
         break;
  }
}

char *AbbreviatedFocus(const char *longName){

int i,size,num = 0;
int flag  = 0;
char *newword;
int nextFocus = 0;
int numChars = 0;
int first = 0;

  if(longName == NULL)
     return(NULL);

  size = strlen(longName); 
  newword = (char *)malloc((size)+2);
  memset(newword,'\0',size+2);

  for(i = 0; i < size; i++){
      if(longName[i] == '/'){
	  if(!nextFocus){
	     nextFocus = 1;
	     first = 1;
	     numChars = 0;
          }
	  else if (first){
	    first = 0;
	  }
	  else { 
             flag = 1;
	  }
      }
      else if(longName[i] == ','){
	  nextFocus = 0;
	  flag = 0;
	  if(first){
	    num -=numChars;
	    numChars = 0;
	  }
	  else {
	    newword[num] = '.';
	    num++;
	  }
      }
      if (flag){
          newword[num] = longName[i]; 
	  num++;
      }
      else if (nextFocus) {
           newword[num] = longName[i];
           num++;
	   numChars++;
      }
  }
  if(first){
      num -=numChars;
  }

  if(num <= 0) {
     free(newword);
     newword = (char *) malloc(strlen(VISI_DEFAULT_FOCUS)+1);
     strcpy(newword,VISI_DEFAULT_FOCUS);
  } else {
     if(newword[num-1] == ','){
	newword[num-1] = '\0';
     }
     newword[num] = '\0';
  }
  PARADYN_DEBUG(("abbreviated focus = %s size = %d\n",newword,size));
  PARADYN_DEBUG(("abbreviated focus num = %d",num));
  return(newword);
}

