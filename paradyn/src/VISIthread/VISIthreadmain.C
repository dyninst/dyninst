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
/////////////////////////////////////////////////////////////////////
// * VISIthread main loop
// * Callback routines for UI Upcalls:  VISIthreadchooseMetRes
//		VISIthreadshowMsgREPLY, VISIthreadshowErrorREPLY
// * Callback routines for DataManager Upcalls:  VISIthreadDataCallback  
//   		VISIthreadnewMetricCallback, VISIthreadFoldCallback 
//   		VISIthreadnewResourceCallback VISIthreadPhaseCallback
/////////////////////////////////////////////////////////////////////
/* $Log: VISIthreadmain.C,v $
/* Revision 1.48  1995/11/03 00:07:27  newhall
/* removed sending SIGKILL signal to visi process before thread exits
/*
 * Revision 1.47  1995/10/13  22:08:54  newhall
 * added phaseType parameter to VISIthreadDataHandler.   Purify fixes.
 *
 * Revision 1.46  1995/10/12  19:44:29  naim
 * Adding error recovery when a visi cannot be created. This change
 * implies that whenever the visiUser constructor is used, it
 * is the user's responsability to check whether the new object have been
 * successfully created or not (i.e. by checking the public method
 * bool errorConditionFound in class visualizationUser) - naim
 *
 * Revision 1.45  1995/09/26  20:48:43  naim
 * Minor error messages changes
 *
 * Revision 1.44  1995/09/18  18:22:34  newhall
 * changes to avoid for-scope problem
 *
 * Revision 1.43  1995/08/08  03:13:10  newhall
 * updates due to changes in DM: newPerfData, sampleDataCallbackFunc defs.
 *
 * Revision 1.42  1995/08/05  17:10:46  krisna
 * use `0' for `NULL'
 *
 * Revision 1.41  1995/08/01 02:18:41  newhall
 * changes to support phase interface
 *
 * Revision 1.40  1995/07/06  01:53:58  newhall
 * fixed arguments to RPCprocessCreate (I have no idea why this worked before)
 *
 * Revision 1.39  1995/06/02  20:54:34  newhall
 * made code compatable with new DM interface
 * replaced List templates  with STL templates
 *
 * Revision 1.38  1995/02/26  02:08:34  newhall
 * added some of the support for the phase interface
 * fix so that the vector of data values are being
 * correctly filled before call to BulkDataTransfer
 *
 * Revision 1.37  1995/02/16  19:10:56  markc
 * Removed start slash from comments
 * Removed start slash from comments
 *
 * Revision 1.36  1995/02/16  08:22:29  markc
 * Changed Boolean to bool
 * Changed wait loop code for igen messages - check for buffered messages
 * Changed char igen-array code to use strings/vectors for igen functions
 *
 * Revision 1.35  1995/01/26  17:59:12  jcargill
 * Changed igen-generated include files to new naming convention; fixed
 * some bugs compiling with gcc-2.6.3.
 *
 * Revision 1.34  1995/01/05  19:23:10  newhall
 * changed the size of the data buffer to be proportional
 * to the number of enabled metric/focus pairs.
 *
 * Revision 1.33  1994/11/04  06:41:03  newhall
 * removed printfs
 *
 * Revision 1.32  1994/11/03  05:17:43  newhall
 * removed trailing comma in AbbreviatedFocus
 *
 * Revision 1.31  1994/10/10  21:41:15  newhall
 * more changes to support new UI metric/focus selections
 *
 * Revision 1.30  1994/10/10  02:51:15  newhall
 * purify fixes, fixes to support new metric/focus choices
 *
 * Revision 1.29  1994/09/30  21:21:14  newhall
 * purify related fixes
 *
 * Revision 1.28  1994/09/30  19:18:45  rbi
 * Abstraction interface change.
 *
 * Revision 1.27  1994/09/25  01:52:08  newhall
 * updated to support the changes to the  visi, UI and VM interfaces having
 * to do with a new representation of metric/focus lists as a list of
 * metric/focus pairs.
 *
 * Revision 1.26  1994/09/22  01:19:43  markc
 * RPCprocessCreate takes &int, not int*, changed args to call
 * typecast args for msg_bind_buffered
 * access igen class members using methods
 *
 * Revision 1.25  1994/09/05  19:10:53  newhall
 * changed AbbreviatedFocus to produce entire path from root node
 *
 * Revision 1.24  1994/08/13  20:52:38  newhall
 * changed when a visualization process is started
 * added new file VISIthreadpublic.C
 *
 * Revision 1.23  1994/08/11  02:19:23  newhall
 * added call to dataManager routine destroyPerformanceStream
 *
 * Revision 1.22  1994/08/10  17:20:59  newhall
 * changed call to chooseMetricsandResources to conform to new UI interface
 *
 * Revision 1.21  1994/08/05  16:04:33  hollings
 * more consistant use of stringHandle vs. char *.
 *
 * Revision 1.20  1994/08/03  20:46:53  newhall
 * removed calls to visi interface routine Enabled()
 * added error detection code
 *
 * Revision 1.19  1994/08/02  17:12:19  newhall
 * bug fix to StopMetricResource
 *
 * Revision 1.18  1994/08/01  17:28:57  markc
 * Removed uses of getCurrent, setCurrent.  Replaced with list iterators.
 *
 * Revision 1.17  1994/07/30  20:38:05  newhall
 * Added calls to visi interface routines Enabled and BulkDataTransfer
 * durring the processing of new metric choices
 *
 * Revision 1.16  1994/07/28  22:32:59  krisna
 * proper starting sequence for VISIthreadmain thread
 *
 * Revision 1.15  1994/07/12  17:03:09  newhall
 * added error handling, changed msg binding for the visualization
 * file discriptor
 *
 * Revision 1.14  1994/07/07  17:27:14  newhall
 * fixed compile warnings
 *
 * Revision 1.13  1994/07/02  01:44:32  markc
 * Removed aggregation operator from enableDataCollection call.
 * Remove aggregation operator from enableDataCollection call.
 *
 * Revision 1.12  1994/06/29  21:46:40  hollings
 * fixed malloc on default focus case.
 *
 * Revision 1.11  1994/06/27  21:25:54  rbi
 * New abstraction parameter for performance streams
 *
 * Revision 1.10  1994/06/17  18:15:25  newhall
 * removed debug stmts
 *
 * Revision 1.9  1994/06/17  00:13:35  hollings
 * Fixed error in malloc of the string longName.
 *
 * Revision 1.8  1994/06/16  18:28:30  newhall
 * added short focus names
 *
 * Revision 1.7  1994/06/14  15:19:15  markc
 * Added new param to enableDataCollection call which specifies how a metric is
 * to be aggregated.  This is probably temporary, since the data manager or the
 * configuration language should specify this info.
 *
 * Revision 1.6  1994/06/07  18:16:32  newhall
 * support for adding metrics/resources to an existing set
 *
 * Revision 1.5  1994/06/03  18:22:51  markc
 * Changes to support igen error handling.
 *
 * Revision 1.4  1994/05/17  00:53:14  hollings
 * Changed waiting time to transfer data to visi process to 0.  We used to wait
 * for the buffer to fill which delayed the data too much.
 *
 * Revision 1.3  1994/05/11  17:21:32  newhall
 * Changes to handle multiple curves on one visualization
 * and multiple visualizations.  Fixed problems with folding
 * and resource name string passed to visualization.  Changed
 * data type from double to float.
 *
 * Revision 1.2  1994/04/28  22:08:10  newhall
 * test version 2
 *
 * Revision 1.1  1994/04/09  21:23:21  newhall
 * test version
 * */
#include <signal.h>
#include <math.h>
#include <stdlib.h>
#include "util/h/rpcUtil.h"
#include "util/h/sys.h"
#include "paradyn/src/VMthread/VMtypes.h"
#include "VISIthread.thread.SRVR.h"
#include "VISIthreadTypes.h"
#include "paradyn/src/pdMain/paradyn.h"
#include "dyninstRPC.xdr.CLNT.h"
#include "paradyn/src/DMthread/DMinclude.h"
#define  ERROR_MSG(s1, s2) \
	 uiMgr->showError(s1,s2); 

/*
#define DEBUG3
*/

char *AbbreviatedFocus(char *);

/*
#define  ASSERTTRUE(x) assert(x); 
*/
#define  ASSERTTRUE(x) 


/////////////////////////////////////////////////////////////
//  VISIthreadDataHandler: routine to handle data values from 
//    the datamanger to the visualization  
//
//  adds the data value to it's local buffer and if the buffer
//  is full sends it to the visualization process
/////////////////////////////////////////////////////////////
void VISIthreadDataHandler(perfStreamHandle handle,
			    metricInstanceHandle mi,
			    int bucketNum,
			    sampleValue value,
			    phaseType phase_type){

  VISIthreadGlobals *ptr;
  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
      PARADYN_DEBUG(("thr_getspecific in VISIthreadDataCallback"));
      ERROR_MSG(13,"thr_getspecific in VISIthread::VISIthreadDataCallback");
      return;
  }

  if(ptr->start_up)  // if visi process has not been started yet return
      return;

  if((ptr->bufferSize >= BUFFERSIZE) || (ptr->bufferSize < 0)){
      PARADYN_DEBUG(("bufferSize out of range: VISIthreadDataCallback")); 
      ERROR_MSG(16,"bufferSize out of range: VISIthreadDataCallback");
      ptr->quit = 1;
      return;
  }
  if((ptr->buffer == NULL)){
      PARADYN_DEBUG(("buffer error: VISIthreadDataCallback")); 
      ERROR_MSG(16,"buffer error: VISIthreadDataCallback");
      ptr->quit = 1;
      return;
  }

  // find metricInstInfo for this metricInstanceHandle
  metricInstInfo *info = NULL;
  for(unsigned i=0; i < ptr->mrlist.size(); i++){
      if((ptr->mrlist[i])->mi_id == mi){
          info = ptr->mrlist[i];
      }
  }
  if(!info) return;  // just ignore values 

  // add data value to buffer
  ptr->buffer[ptr->bufferSize].data = value;
  ptr->buffer[ptr->bufferSize].metricId = info->m_id;
  ptr->buffer[ptr->bufferSize].resourceId = info->r_id; 
  ptr->buffer[ptr->bufferSize].bucketNum = bucketNum;
 
#ifdef DEBUG3
  if((bucketNum % 100) == 0){ 
      fprintf(stderr,"VISIthr %d: bucket = %d value = %f met = %d res = %d\n",
	      thr_self(),bucketNum,value,info->m_id,info->r_id);
  }
#endif

  ptr->bufferSize++;

  // if buffer is full, send buffer to visualization
  if(ptr->bufferSize >= ptr->maxBufferSize) {
      vector<T_visi::dataValue> temp;
      for (unsigned ve=0; ve<ptr->bufferSize; ve++) {
          temp += ptr->buffer[ve];
      }
      
      ptr->visip->Data(temp);
      if(ptr->visip->did_error_occur()){
         PARADYN_DEBUG(("igen: after visip->Data() in VISIthreadDataHandler"));
         ptr->quit = 1;
         return;
      }
      ptr->bufferSize = 0;
  }
  info = 0;
}

#ifdef n_def
/////////////////////////////////////////////////////////////
//  VISIthreadDataCallback: Callback routine for DataManager 
//    newPerfData Upcall
//
/////////////////////////////////////////////////////////////
void VISIthreadDataCallback(perfStreamHandle handle,
			    metricInstanceHandle mi,
			    sampleValue *values,
			    int total,
			    int first){

  for(unsigned i=first; i < (first+total);i++){
      VISIthreadDataHandler(handle,mi,i,values[i-first]);
  }

}
#endif


/////////////////////////////////////////////////////////
//  VISIthreadnewMetricCallback: callback for dataManager
//    newMetricDefined Upcall
//    (not currently implemented)  
/////////////////////////////////////////////////////////
void VISIthreadnewMetricCallback(perfStreamHandle handle,
				 const char *name,
				 int style,
				 int aggregate,
				 const char *units,
				 metricHandle m_handle){
 VISIthreadGlobals *ptr;

  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
    PARADYN_DEBUG(("thr_getspecific in VISIthreadnewMetricCallback"));
    ERROR_MSG(13,"thr_getspecific in VISIthread::VISIthreadnewMetricCallback");
    return;
  }
  if(ptr->start_up)  // if visi process has not been started yet return
    return;
}

///////////////////////////////////////////////////////////
//  VISIthreadnewResourceCallback: callback for dataManager
//    newResourceDefined Upcall 
//    (not currently implemented)  
//////////////////////////////////////////////////////////
void VISIthreadnewResourceCallback(perfStreamHandle handle,
				   resourceHandle  parent,
			           resourceHandle newResource, 
				   const char *name,
				   const char *abstr){

VISIthreadGlobals *ptr;

if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
   PARADYN_DEBUG(("thr_getspecific in VISIthreadnewResourceCallback"));
   ERROR_MSG(13,"thr_getspecific in VISIthread::VISIthreadnewResourceCallback");
   return;
}

  if(ptr->start_up)  // if visi process has not been started yet return
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
void VISIthreadFoldCallback(perfStreamHandle handle,
			timeStamp width, phaseType phase_type){


 VISIthreadGlobals *ptr;
 vector<T_visi::dataValue> temp;

  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
     PARADYN_DEBUG(("thr_getspecific in VISIthreadFoldCallback"));
     ERROR_MSG(13,"thr_getspecific in VISIthread::VISIthreadFoldCallback");
     return;
  }

  if(ptr->start_up)  // if visi process has not been started yet return
     return;

  if((ptr->bufferSize >= BUFFERSIZE) || (ptr->bufferSize < 0)){
     PARADYN_DEBUG(("bufferSize out of range: VISIthreadFoldCallback")); 
     ERROR_MSG(16,"bufferSize out of range: VISIthreadFoldCallback");
     ptr->quit = 1;
     return;
  }
  if((ptr->buffer == NULL)){
     PARADYN_DEBUG(("buffer error: VISIthreadFoldCallback")); 
     ERROR_MSG(16,"buffer error: VISIthreadFoldCallback");
     ptr->quit = 1;
     return;
  }

#ifdef DEBUG3
    PARADYN_DEBUG(("Fold: new width = %f phaseType = %d\n",width,phase_type));
    PARADYN_DEBUG(("    my phaseType = %d\n",ptr->args->phase_type));
    PARADYN_DEBUG(("    my curr Width = %f\n",ptr->bucketWidth));
#endif

  // ignore folds for other phase types 
  if(ptr->args->phase_type != phase_type) return;
  // ignore folds for other phase data...this is an old phase visi
  if(ptr->currPhaseHandle != -1) {
      if((phase_type != GlobalPhase) && 
	 (ptr->args->my_phaseId != ptr->currPhaseHandle)) return;
  }

  // if new Width is same as old width ignore Fold 
  if(ptr->bucketWidth != width){
     // if buffer is not empty send visualization buffer of data values
     if(ptr->bufferSize != 0){
       // temp.count = ptr->bufferSize;
       // temp.data = ptr->buffer;
        for (unsigned ve=0; ve<ptr->bufferSize; ve++)
	  temp += ptr->buffer[ve];
        ptr->visip->Data(temp);
        if(ptr->visip->did_error_occur()){
           PARADYN_DEBUG(("igen: visip->Data() in VISIthreadFoldCallback"));
           ptr->quit = 1;
           return;
        }
        ptr->bufferSize = 0;
     }
     ptr->bucketWidth = width;
     // call visualization::Fold routine
     ptr->visip->Fold((double)width);
     if(ptr->visip->did_error_occur()){
        PARADYN_DEBUG(("igen: after visip->Fold() in VISIthreadFoldCallback"));
        ptr->quit = 1;
        return;
     }
  }

}
///////////////////////////////////////////////////////
//  VISIthreadPhaseCallback: callback for dataManager
//     new phase definition upcall 
//
///////////////////////////////////////////////////////
void VISIthreadPhaseCallback(perfStreamHandle ps_handle, 
			     const char *name,
			     phaseHandle handle,
			     timeStamp begin,
			     timeStamp end,
			     float bucketWidth){

   VISIthreadGlobals *ptr;
   if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
      PARADYN_DEBUG(("thr_getspecific in VISIthreadPhaseCallback"));
      ERROR_MSG(13,"thr_getspecific in VISIthread::VISIthreadPhaseCallback");
      return;
   }

  if(ptr->start_up)  // if visi process has not been started yet return
     return;


#ifdef DEBUG3
   fprintf(stderr,"in VISIthreadPhaseCallback\n");
   fprintf(stderr,"phase name = %s\n",name);
   fprintf(stderr,"phase begin = %f\n",begin);
   fprintf(stderr,"phase end = %f\n",end);
   fprintf(stderr,"phase width = %f\n",bucketWidth);
   fprintf(stderr,"phase handle = %d\n",handle);
#endif

   // send visi phase end call for current phase
   // if(ptr->currPhaseHandle != -1)
       ptr->visip->PhaseEnd((double)begin,ptr->currPhaseHandle);

   ptr->currPhaseHandle = handle;

   // send visi phase start call for new phase
   ptr->visip->PhaseStart((double)begin,(double)end,bucketWidth,name,handle);

   // if this visi is defined for the current phase then data values 
   // will stop arriving so remove all mrlist elements
//   if(ptr->args->phase_type == CurrentPhase){ 
//      for(unsigned i=0; i < ptr->mrlist.size(); i++){
//         metricInstInfo *next = ptr->mrlist[i]; 
//        delete(next);
//      }
//       ptr->mrlist.resize(0);
//      assert(!(ptr->mrlist.size()));
//   }
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
    PARADYN_DEBUG(("start_up in VISIthreadStartProcess"));
    vector<string> av;
    // start at 1 since RPCprocessCreate will add 0th arg to list
    unsigned index=1;
    while(ptr->args->argv[index]) {
      av += ptr->args->argv[index];
      index++;
    }
    ptr->fd = RPCprocessCreate(ptr->pid, "localhost", "",
				 ptr->args->argv[0], av);
    if (ptr->fd < 0) {
      PARADYN_DEBUG(("Error in process Create: RPCprocessCreate"));
      ERROR_MSG(14,"");
      ptr->quit = 1;
      return(0);
    }

    ptr->visip = new visiUser(ptr->fd); 
    if (ptr->visip->errorConditionFound) {
      ERROR_MSG(14,"");
      ptr->quit = 1;
      return(0);
    }

    if(msg_bind_buffered(ptr->fd,0, (int(*)(void*)) xdrrec_eof,ptr->visip->net_obj()) 
       != THR_OKAY) {
      PARADYN_DEBUG(("Error in msg_bind(ptr->fd)"));
      ERROR_MSG(14,"");
      ptr->quit = 1;
      return(0);
    }

  }
  ptr->start_up = 0;  // indicates that process has been started
  return(1);
}

#ifdef n_def
static u_int VISIthread_num_enabled = 0;
#endif
///////////////////////////////////////////////////////////////////
//  VISIthreadchooseMetRes: callback for User Interface Manager 
//    chooseMetricsandResources upcall
//    input: list of metric/focus matrices 
//
//  if the focus has already been enabled, make enable requests to
//  the dataManager for only those metric/focus pairs that have not
//  been previously enabled for this visualization
//  else try to enable all metric/focus pairs
//
//  send each successfully enabled metric and focus to visualization 
//  process (call visualizationUser::AddMetricsResources)
//
//  the visi process is started only after the first set of enabled
//  metrics and resources has been obtained 
///////////////////////////////////////////////////////////////////
int VISIthreadchooseMetRes(vector<metric_focus_pair> *newMetRes){

  if(newMetRes)
      PARADYN_DEBUG(("In VISIthreadchooseMetRes size = %d",newMetRes->size()));

  VISIthreadGlobals *ptr;
  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
    PARADYN_DEBUG(("thr_getspecific in VISIthreadchooseMetRes"));
    ERROR_MSG(13,"thr_getspecific VISIthread::VISIthreadchooseMetRes");
    return 1;
  }

  // check for invalid reply  ==> user picked "Cancel" menu option
  if(newMetRes == NULL){
      if(ptr->start_up){
          ptr->quit = 1;
      }
      return 1;
  }

  // try to enable metric/focus pairs
  vector<metric_focus_pair>  *retryList = new vector<metric_focus_pair> ;
  // vector<metric_focus_pair>  new_pairs = *newMetRes;
  int  numEnabled = 0;
  metricInstInfo *newPair = NULL;
  metricInstInfo **newEnabled = new (metricInstInfo *)[newMetRes->size()];
  for(unsigned k=0; k < newMetRes->size(); k++){

#ifdef n_def
      switch (VISIthread_num_enabled % 4) {
            case 0:
		cout << "enabling with persistent data 0 collection 0" << endl;
                newPair = ptr->dmp->enableDataCollection(ptr->ps_handle,
			      &((*newMetRes)[k].res), (*newMetRes)[k].met,
			      ptr->args->phase_type,0,0);
                
                break;
            case 1:
		cout << "enabling with persistent data 1 collection 0" << endl;
                newPair = ptr->dmp->enableDataCollection(ptr->ps_handle,
			      &((*newMetRes)[k].res), (*newMetRes)[k].met,
			      ptr->args->phase_type,1,0);
                break;
            case 2:
		cout << "enabling with persistent data 0 collection 1" << endl;
                newPair = ptr->dmp->enableDataCollection(ptr->ps_handle,
			      &((*newMetRes)[k].res), (*newMetRes)[k].met,
			      ptr->args->phase_type,0,1);
                break;
            case 3:
		cout << "enabling with persistent data 1 collection 1" << endl;
                newPair = ptr->dmp->enableDataCollection(ptr->ps_handle,
			      &((*newMetRes)[k].res), (*newMetRes)[k].met,
			      ptr->args->phase_type,1,1);
                break;
      }

      VISIthread_num_enabled++;
      if(newPair)
#endif

      if((newPair = ptr->dmp->enableDataCollection(ptr->ps_handle,
		    &((*newMetRes)[k].res), (*newMetRes)[k].met,
		    ptr->args->phase_type,0,0))){

          // check to see if this pair has already been enabled
          bool found = false;
	  for(unsigned i = 0; i < ptr->mrlist.size(); i++){
	      if((ptr->mrlist[i]->mi_id == newPair->mi_id)){
                  found = true;
		  break;
	      }
	  }
	  if(!found){  // this really is a new metric/focus pair
              ptr->mrlist += newPair;
	      newEnabled[numEnabled++] = newPair;
          }
      }
      else {  // add to retry list
	  *retryList += (*newMetRes)[k];
      }
  }

  // something was enabled
  if(numEnabled > 0){
      // increase the buffer size
      ptr->maxBufferSize += numEnabled;
      ptr->maxBufferSize = MIN(ptr->maxBufferSize, BUFFERSIZE);
      assert(ptr->maxBufferSize <= BUFFERSIZE);

      // if this is the first set of enabled values, start visi process
      if(ptr->start_up){
          if(!VISIthreadStartProcess()){
              ptr->quit = 1;
              return 1;
          }
      }
      ASSERTTRUE(!ptr->start_up);
      vector<T_visi::visi_matrix> pairList;
      // create a visi_matrix_Array to send to visualization
      for(unsigned l=0; l < numEnabled; l++){
	  T_visi::visi_matrix matrix;
          matrix.met.Id = newEnabled[l]->m_id;
	  matrix.met.name = newEnabled[l]->metric_name; 
	  matrix.met.units = newEnabled[l]->metric_units;
	  matrix.met.aggregate = AVE;
	  matrix.res.Id = newEnabled[l]->r_id;
	  if((matrix.res.name = 
	      AbbreviatedFocus((char *)newEnabled[l]->focus_name.string_of()))
	      ==0){
	      ERROR_MSG(12,"in VISIthreadchooseMetRes");
	      ptr->quit = 1;
              delete(retryList);
              delete(newMetRes);
	      return 1;
          }
          pairList += matrix;
      }
      // if buffer is not empty send visualization buffer of data values
      vector<T_visi::dataValue>   tempdata;
      if(ptr->bufferSize != 0){
          for (unsigned ve=0; ve<ptr->bufferSize; ve++)
	    tempdata += ptr->buffer[ve];
	  ptr->visip->Data(tempdata);
          if(ptr->visip->did_error_occur()){
             PARADYN_DEBUG(("igen: visip->Data() in VISIthreadchooseMetRes"));
             ptr->quit = 1;
             delete(retryList);
             delete(newMetRes);
             return 1;
          }
	  ptr->bufferSize = 0;
      }

      PARADYN_DEBUG(("before call to AddMetricsResources\n"));
      if(ptr->args->phase_type == GlobalPhase){
          ptr->visip->AddMetricsResources(pairList,
				          ptr->dmp->getGlobalBucketWidth(),
				          ptr->dmp->getMaxBins(),
				          ptr->args->start_time,
					  -1);
      }
      else {
      ptr->visip->AddMetricsResources(pairList,
				      ptr->dmp->getCurrentBucketWidth(),
				      ptr->dmp->getMaxBins(),
				      ptr->args->start_time,
				      ptr->args->my_phaseId);
      }
      if(ptr->visip->did_error_occur()){
          PARADYN_DEBUG(("igen: visip->AddMetsRess(): VISIthreadchooseMetRes"));
          ptr->quit = 1;
          delete(retryList);
          delete(newMetRes);
          return 1;
      }

      // get old data bucket values for new metric/resources and
      // send them to visualization
      sampleValue *buckets = new sampleValue[1001];
      for(unsigned q=0;q<numEnabled;q++){
        int howmany = ptr->dmp->getSampleValues(newEnabled[q]->mi_id,
					    buckets,1000,0,
					    ptr->args->phase_type);
        // send visi all old data bucket values
	if(howmany > 0){
	    vector<float> bulk_data;
	    for (unsigned ve=0; ve<howmany; ve++){
	      bulk_data += buckets[ve];
	      if(ve > 1000) cout << "Array bounds error in VISIthreadMain" << endl;
            }
            ptr->visip->BulkDataTransfer(bulk_data, (int)newEnabled[q]->m_id,
		                        (int)newEnabled[q]->r_id);
            if(ptr->visip->did_error_occur()){
            PARADYN_DEBUG(("igen:visip->BulkDataTransfer():VISIthreadchoose"));
                ptr->quit = 1;
                delete(retryList);
                delete(newMetRes);
                return 1;
            }
	    for(int j=bulk_data.size(); j>0; j--){
               bulk_data.resize(bulk_data.size()-1);
	    }
	}
      }
      // if remenuFlag is set and retry list is not empty
      // send retry list to UIM to deal with 
      if((ptr->args->remenuFlag) && (retryList->size())){
        // don't free retryList since it is passed to UI
      }
      else { // else ignore, and set remenuFlag
        ptr->args->remenuFlag = 1;     
        delete(retryList);
      }
      delete(newMetRes);
      delete(buckets);
  }
  else {
      // if nothing was enabled, and remenuflag is set make remenu request
      PARADYN_DEBUG(("No enabled Metric/focus pairs: VISIthreadchooseMetRes"));
      // remake menuing call with 
      if(ptr->args->remenuFlag){
         ptr->ump->chooseMetricsandResources(VISIthreadchooseMetRes, newMetRes);
      }
      else{ // if nothing was enabled, and remenuflag is not set quit
	 ptr->quit = 1;
      }
      ERROR_MSG(17,"Cannot select the same metric twice. Please, try again");
  }
  return 1;
}


////////////////////////////////////////////////////////
//  VISIthreadshowMsgREPLY: callback for User Interface 
//    Manager showMsgREPLY upcall (not currently implemented) 
///////////////////////////////////////////////////////
void VISIthreadshowMsgREPLY(int userChoice){


}

////////////////////////////////////////////////////////
//  VISIthreadshowErrorREPLY: callback for User Interface 
//    Manager showErrorREPLY upcall (not currently implemented)
///////////////////////////////////////////////////////
void VISIthreadshowErrorREPLY(int userChoice){
  

}


///////////////////////////////////////////////////////////////////
//  VISIthread main routine
//  input: parent thread tid, visualization command line arguments
//
//  initializes thread local variables, starts visualization process
//  and enters main loop
///////////////////////////////////////////////////////////////////
void *VISIthreadmain(void *vargs){ 
 
  //initialize global variables
  VISIthreadGlobals *globals;
  globals = new VISIthreadGlobals;

  VISIthread *vtp = new VISIthread(VMtid);
  globals->ump = uiMgr;
  globals->vmp = vmMgr;
  globals->dmp = dataMgr;
  globals->args = (visi_thread_args *) vargs;
  globals->visip = NULL;     // assigned value in VISIthreadStartProcess 
  globals->currPhaseHandle = -1;

  globals->start_up = 1;
  globals->bufferSize = 0;
  globals->maxBufferSize = 0;
  globals->fd = -1;
  globals->pid = -1;
  globals->quit = 0;
  globals->bucketWidth = globals->args->bucketWidth;

  // set control callback routines 
  controlCallback callbacks;
  callbacks.mFunc = VISIthreadnewMetricCallback;
  callbacks.rFunc = VISIthreadnewResourceCallback; 
  callbacks.fFunc = VISIthreadFoldCallback;
  callbacks.pFunc = VISIthreadPhaseCallback;
  callbacks.sFunc = NULL;

  PARADYN_DEBUG(("before create performance stream in visithread"));

  // create performance stream
  union dataCallback dataHandlers;
  dataHandlers.sample = (sampleDataCallbackFunc)VISIthreadDataHandler;
  if((globals->ps_handle = globals->dmp->createPerformanceStream(
		   Sample,dataHandlers,callbacks)) == 0){
      PARADYN_DEBUG(("Error in createPerformanceStream"));
      ERROR_MSG(15,"Error in VISIthreadchooseMetRes: createPerformanceStream");
      globals->quit = 1;
  }

  PARADYN_DEBUG(("perf. stream = %d in visithread",(int)globals->ps_handle));

  if (thr_setspecific(visiThrd_key, globals) != THR_OKAY) {
      PARADYN_DEBUG(("Error in thr_setspecific"));
      ERROR_MSG(14,"Error in VISIthreadmain: thr_setspecific");
      globals->quit = 1;
  }

  // if forceProcessStart is set, start visi process and skip menuing
  // and parsing of initial set of metrics and resources
  if( globals->args->forceProcessStart){

     // start visi process
     if(!VISIthreadStartProcess()){
          globals->quit = 1;
     }
  }
  // parse globals->args->matrix
  // to determine if menuing needs to be done.  If so, call UIM rouitine
  // chooseMetricsandResources before entering main loop, if not, call
  // AddMetricsResources routine with metric and focus pointers (these
  // need to be created from the metricList and resourceList) 
  // until parsing routine is in place call chooseMetricsandResources
  // with NULL metric and resource pointers
  else{

    // TODO: add parsing code 

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
      thread_t tag = MSG_TAG_ANY;
      int from = msg_poll(&tag, 1);
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
      } else if (tag == MSG_TAG_FILE){
	assert(globals->visip);
	assert(from == globals->visip->get_fd());
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
	metricInstanceHandle handle = globals->mrlist[i]->mi_id;
        globals->dmp->disableDataCollection(globals->ps_handle,handle,
					    globals->args->phase_type);
  }

  // kill visi process
  // if(!globals->start_up){
  //     kill(globals->pid,SIGKILL);
 //  }

  PARADYN_DEBUG(("before destroy perfomancestream"));
  if(!(globals->dmp->destroyPerformanceStream(globals->ps_handle))){
      ERROR_MSG(16,"remove() in VISIthreadmain");
  }

  // notify VM 
  PARADYN_DEBUG(("before notify VM of thread died"));
  globals->vmp->VMVisiDied(thr_self());
  PARADYN_DEBUG(("after notify VM of thread died"));

  // unbind file descriptor associated with visualization
  if(!globals->start_up){
      if(msg_unbind(globals->fd) == THR_ERR){
          PARADYN_DEBUG(("Error in msg_unbind(globals->fd)"));
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

char *AbbreviatedFocus(char *longName){

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
	    newword[num] = ',';
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
  free(longName);
  return(newword);
}

