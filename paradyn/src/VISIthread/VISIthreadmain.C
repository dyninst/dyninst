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
//   		VISIthreadnewResourceCallback
/////////////////////////////////////////////////////////////////////
/* $Log: VISIthreadmain.C,v $
/* Revision 1.29  1994/09/30 21:21:14  newhall
/* purify related fixes
/*
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
#include <memory.h>
#include "util/h/list.h"
#include "util/h/rpcUtil.h"
#include "../VMthread/VMtypes.h"
#include "VISIthread.SRVR.h"
#include "VISIthreadTypes.h"
#include "../pdMain/paradyn.h"
#include "dyninstRPC.CLNT.h"
#include "../DMthread/DMinternals.h"
#define  ERROR_MSG(s1, s2) \
	 uiMgr->showError(s1,s2); \
	 printf("error# %d: %s\n",s1,s2); 

char *AbbreviatedFocus(char *);
char *GetResourceName(resourceList *);

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
void VISIthreadDataHandler(performanceStream *ps,
			    metricInstance *mi,
			    int bucketNum,
			    sampleValue value){



 VISIthreadGlobals *ptr;
 dataValue_Array   temp;

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
  // add data value to buffer
  ptr->buffer[ptr->bufferSize].data = value;

  ptr->buffer[ptr->bufferSize].metricId = (int)(mi->met);
  ptr->buffer[ptr->bufferSize].resourceId = 
	(int)mi->focus->getCanonicalName();
  ptr->buffer[ptr->bufferSize].bucketNum = bucketNum;

#ifdef DEBUG3
if((bucketNum % 100) == 0){ 
  fprintf(stderr,"VISIthread %d: bucketNum = %d value = %f metric = %d resource = %d\n", thr_self(),bucketNum,value,(int)(mi->met),ptr->buffer[ptr->bufferSize].resourceId);
}
#endif

  ptr->bufferSize++;

  // if buffer is full, send buffer to visualization
  // if(ptr->bufferSize == BUFFERSIZE)
  if(ptr->bufferSize) {

    temp.count = ptr->bufferSize;
    temp.data = ptr->buffer;
    ptr->visip->Data(temp);
    if(ptr->visip->did_error_occur()){
       PARADYN_DEBUG(("igen: after visip->Data() in VISIthreadDataHandler"));
       ptr->quit = 1;
       return;
    }
    ptr->bufferSize = 0;
  }
}

/////////////////////////////////////////////////////////////
//  VISIthreadDataCallback: Callback routine for DataManager 
//    newPerfData Upcall
//
/////////////////////////////////////////////////////////////
void VISIthreadDataCallback(performanceStream *ps,
			    metricInstance *mi,
			    sampleValue *values,
			    int total,
			    int first){

  int i;

  for(i=first; i < (first+total);i++){
    VISIthreadDataHandler(ps,mi,i,values[i-first]);
  }

}


/////////////////////////////////////////////////////////
//  VISIthreadnewMetricCallback: callback for dataManager
//    newMetricDefined Upcall
//    (not currently implemented)  
/////////////////////////////////////////////////////////
void VISIthreadnewMetricCallback(performanceStream *ps,
				 metric *m){

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
void VISIthreadnewResourceCallback(performanceStream *ps,
				   resource *parent,
			           resource *newResource, 
				   char *name){

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
void VISIthreadFoldCallback(performanceStream *ps,
			timeStamp width){

 VISIthreadGlobals *ptr;
 dataValue_Array   temp;


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
  // if new Width is same as old width ignore Fold 
  if(ptr->bucketWidth != width){
     // if buffer is not empty send visualization buffer of data values
     if(ptr->bufferSize != 0){
        temp.count = ptr->bufferSize;
        temp.data = ptr->buffer;
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
    ptr->fd = RPCprocessCreate(ptr->pid, "localhost", "",
				 ptr->args->argv[0],ptr->args->argv);
    if (ptr->fd < 0) {
      PARADYN_DEBUG(("Error in process Create"));
      ERROR_MSG(14,"Error in VISIthreadStartProcess: RPCprocessCreate");
      ptr->quit = 1;
      return(0);
    }

    ptr->visip = new visiUser(ptr->fd,NULL,NULL); 

    if(msg_bind_buffered(ptr->fd,0, (int(*)(void*)) xdrrec_eof,ptr->visip->getXdrs()) 
       != THR_OKAY) {
      PARADYN_DEBUG(("Error in msg_bind(ptr->fd)"));
      ERROR_MSG(14,"Error VISIthreadStartProcess: msg_bind_buffered");
      ptr->quit = 1;
      return(0);
    }

  }
  ptr->start_up = 0;  // indicates that process has been started
  return(1);
}


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
void VISIthreadchooseMetRes(metrespair *newMetRes,
			    int numElements){

 VISIthreadGlobals *ptr;
 metric *currMetric;
 metricInstance *currMetInst;
 metricInstance *newEnabled[numElements];
 metrespair *retryList;
 visi_matrix_Array pairList;
 metricInstance *temp;
 metricInfo *temp2;
 sampleValue buckets[1000];
 dataValue_Array   tempdata;
 timeStamp binWidth = 0.0;
 int  numEnabled = 0;
 int  numRetry = 0;
 int  i,j,found;
 int  numBins = 0;
 int  howmany;
 stringHandle key;
 float_Array bulk_data;
 List<metricInstance*> walk;
 int k;

  PARADYN_DEBUG(("In VISIthreadchooseMetRes numElements = %d",numElements));

  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
    PARADYN_DEBUG(("thr_getspecific in VISIthreadchooseMetRes"));
    ERROR_MSG(13,"thr_getspecific VISIthread::VISIthreadchooseMetRes");
    return;
  }

  // check for invalid reply  ==> user picked "Cancel" menu option
  if((numElements <= 0) || (newMetRes == NULL)){
    PARADYN_DEBUG(("no metric and resource in VISIthreadchooseMetRes"));
    ERROR_MSG(17,"Incomplete metric/focus list:VISIthreadchooseMetRes");
    if(ptr->start_up){
      ptr->quit = 1;
    }
    return;
  }

  if((retryList  = 
      (metrespair *)malloc(sizeof(metrespair)*numElements)) == NULL){
      ERROR_MSG(12,"in VISIthreadchooseMetRes");
      ptr->quit = 1;
      return;
  }

  pairList.data = (visi_matrix *)NULL;
  for(k=0; k < numElements; k++){

      key = newMetRes[k].focus->getCanonicalName();
      currMetric = newMetRes[k].met;

      // determine if this metric/focus has been enabled before
      found = 0;
      for (walk= *ptr->mrlist; temp=*walk; ++walk){ 
          if ((temp->focus->getCanonicalName() == key) &&
	      (temp->met == currMetric)){

              found = 1;
              break;
      } }
      // this is a new metric/focus pair try to enable
      if(!found){  

          PARADYN_DEBUG(("before enable metric/focus\n"));
          if((currMetInst = 
	      ptr->dmp->enableDataCollection(ptr->perStream,
	      newMetRes[k].focus,currMetric)) != NULL){

              PARADYN_DEBUG(("after enable metric/focus metInst = %d\n",
			   (int)currMetInst));
              ptr->mrlist->add(currMetInst,currMetInst);
	      newEnabled[numEnabled++] = currMetInst;
          }
          else{  
	      retryList[numRetry].met = newMetRes[k].met;
	      retryList[numRetry++].focus = newMetRes[k].focus;
          }

      }
      // else ignore: this has already been enabled
  }

  if(numEnabled > 0){

    // if this is the first set of enabled values, start visi process
    if(ptr->start_up){
        if(!VISIthreadStartProcess()){
          ptr->quit = 1;
          return;
        }
    }
    ASSERTTRUE(!ptr->start_up);

    // create a visi_matrix_Array to send to visualization
    pairList.count = numEnabled;
    if((pairList.data  = 
	(visi_matrix *)malloc(sizeof(visi_matrix)*pairList.count)) == NULL){
	ERROR_MSG(12,"in VISIthreadchooseMetRes");
        ptr->quit = 1;
        free(retryList);
        free(newMetRes);
        return;
    }
    for(i=0; i < numEnabled; i++){
	temp2 = newEnabled[i]->met->getInfo();
        pairList.data[i].met.Id = (int)newEnabled[i]->met;
	pairList.data[i].met.name = strdup(temp2->name); 
	pairList.data[i].met.units = strdup(temp2->units);
	pairList.data[i].met.aggregate = AVE;
	pairList.data[i].res.Id =  (int)key;
	if((pairList.data[i].res.name =  
	    AbbreviatedFocus(GetResourceName(newEnabled[i]->focus))) == NULL){
            ERROR_MSG(12,"in VISIthreadchooseMetRes");
	    ptr->quit = 1;
            free(retryList);
            free(newMetRes);
            if(pairList.data){
                 for(j=0; j < i-1; j++){
                        free(pairList.data[j].met.units);
                        free(pairList.data[j].met.name);
                        free(pairList.data[j].res.name);
                }
            }
            free(pairList.data[i].met.name);
            free(pairList.data[i].res.name);
            free(pairList.data);
	    return;
        }
    }

    // if buffer is not empty send visualization buffer of data values
    if(ptr->bufferSize != 0){
	  tempdata.count = ptr->bufferSize;
          tempdata.data = ptr->buffer;
	  ptr->visip->Data(tempdata);
          if(ptr->visip->did_error_occur()){
             PARADYN_DEBUG(("igen: visip->Data() in VISIthreadchooseMetRes"));
             ptr->quit = 1;
             free(retryList);
             free(newMetRes);
             if(pairList.data){
                 for(i=0; i < pairList.count; i++){
                     free(pairList.data[i].met.units);
                     free(pairList.data[i].met.name);
                     free(pairList.data[i].res.name);
                 }
                 free(pairList.data);
             }
             return;
          }
	  ptr->bufferSize = 0;
    }

    binWidth = ptr->dmp->getCurrentBucketWidth(); 
    numBins = ptr->dmp->getMaxBins();
    PARADYN_DEBUG(("before call to AddMetricsResources\n"));
    ptr->visip->AddMetricsResources(pairList,binWidth,numBins);
    if(ptr->visip->did_error_occur()){
        PARADYN_DEBUG(("igen: visip->AddMetsRess(): VISIthreadchooseMetRes"));
        ptr->quit = 1;
        free(retryList);
        free(newMetRes);
        if(pairList.data){
            for(i=0; i < pairList.count; i++){
                free(pairList.data[i].met.units);
                free(pairList.data[i].met.name);
                free(pairList.data[i].res.name);
            }
            free(pairList.data);
        }
        return;
    }

    // get old data bucket values for new metric/resources and
    // send them to visualization
    for(i=0;i<numEnabled;i++){
        howmany = ptr->dmp->getSampleValues(newEnabled[i],
					    buckets,1000,0);
        // send visi all old data bucket values
	if(howmany > 0){
            bulk_data.count = howmany; 
            bulk_data.data = buckets; 
            ptr->visip->BulkDataTransfer(bulk_data,
		   (int)newEnabled[i]->met,
		   (int)newEnabled[0]->focus->getCanonicalName());
            if(ptr->visip->did_error_occur()){
            PARADYN_DEBUG(("igen:visip->BulkDataTransfer():VISIthreadchoose"));
                ptr->quit = 1;
                free(retryList);
                free(newMetRes);
                if(pairList.data){
                    for(i=0; i < pairList.count; i++){
                        free(pairList.data[i].met.units);
                        free(pairList.data[i].met.name);
                        free(pairList.data[i].res.name);
                    }
                    free(pairList.data);
                }
                return;
            }
	}
    }
    // if remenuFlag is set and retry list is not empty
    // send retry list to UIM to deal with 
    if((ptr->args->remenuFlag) && (numRetry)){
        // don't free retryList since it is passed to UI
    }
    else { // else ignore, and set remenuFlag
        ptr->args->remenuFlag = 1;     
        free(retryList);
    }
    free(newMetRes);
  }
  else {

      // if nothing was enabled, and remenuflag is set make remenu request
      ERROR_MSG(17,"No enabled Metric/focus pairs: VISIthreadchooseMetRes");
      PARADYN_DEBUG(("No enabled Metric/focus pairs: VISIthreadchooseMetRes"));

      // remake menuing call with 
      if(ptr->args->remenuFlag){
         ptr->ump->chooseMetricsandResources(
		     (chooseMandRCBFunc)VISIthreadchooseMetRes,
                     newMetRes,
		     numElements);

      }
      else{ // if nothing was enabled, and remenuflag is not set quit
	 ptr->quit = 1;
      }
  }
  if(pairList.data){
    for(i=0; i < pairList.count; i++){
     free(pairList.data[i].met.units);
     free(pairList.data[i].met.name);
     free(pairList.data[i].res.name);
    }
    free(pairList.data);
  }

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
 
  int from;
  thread_t tag;
  VISIthreadGlobals *globals;
  VISIthread *vtp;
  metricInstance *listItem;
  List<metricInstance*> walk;
  union dataCallback dataHandlers;
  controlCallback callbacks;

  //initialize global variables
  if((globals=(VISIthreadGlobals *)malloc(sizeof(VISIthreadGlobals)))==0){
    PARADYN_DEBUG(("Error in malloc globals"));
    ERROR_MSG(13,"malloc in VISIthread::main");
    globals->quit = 1;
  }

  vtp = new VISIthread(VMtid);
  globals->ump = uiMgr;
  globals->vmp = vmMgr;
  globals->dmp = dataMgr;
  globals->args = (visi_thread_args *) vargs;
  globals->visip = NULL;     // assigned value in VISIthreadchooseMetRes 
  globals->perStream = NULL; // assigned value in VISIthreadchooseMetRes

  globals->start_up = 1;
  globals->bufferSize = 0;
  globals->fd = -1;
  globals->pid = -1;
  globals->quit = 0;
  globals->mrlist = new(List<metricInstance *>);
  globals->bucketWidth = globals->dmp->getCurrentBucketWidth(); 

  // set control callback routines 
  callbacks.mFunc = (metricInfoCallback)VISIthreadnewMetricCallback;
  callbacks.rFunc = (resourceInfoCallback)
		      VISIthreadnewResourceCallback;
  callbacks.fFunc = (histFoldCallback)VISIthreadFoldCallback;
  callbacks.sFunc = NULL;

  PARADYN_DEBUG(("before create performance stream in visithread"));

  // create performance stream
  dataHandlers.sample =(sampleDataCallbackFunc)VISIthreadDataCallback;
  if((globals->perStream = globals->dmp->createPerformanceStream(context,
		   Sample,dataHandlers,callbacks)) == NULL){
      PARADYN_DEBUG(("Error in createPerformanceStream"));
      ERROR_MSG(15,"Error in VISIthreadchooseMetRes: createPerformanceStream");
      globals->quit = 1;
  }

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
    globals->ump->chooseMetricsandResources(
			    (chooseMandRCBFunc)VISIthreadchooseMetRes,
                             NULL,0);
  }


 
  PARADYN_DEBUG(("before enter main loop"));
  while(!(globals->quit)){
      tag = MSG_TAG_ANY;
      from = msg_poll(&tag, 1);
      if (globals->ump->isValidUpCall(tag)) {
           globals->ump->awaitResponce(-1);
      }
      else if (globals->vmp->isValidUpCall(tag)) {
          globals->vmp->awaitResponce(-1);
      }
      else if (globals->dmp->isValidUpCall(tag)) {
          globals->dmp->awaitResponce(-1);
      }
      else if (tag == MSG_TAG_FILE){
          globals->visip->awaitResponce(-1);
          if(globals->visip->did_error_occur()){
             PARADYN_DEBUG(("igen: visip->awaitResponce() in VISIthreadmain"));
             globals->quit = 1;
          }
      }
      else {
          vtp->mainLoop();
      }
  }
  PARADYN_DEBUG(("leaving main loop"));

  // disable all metricInstance data collection
  for (walk= *globals->mrlist; listItem= *walk; ++walk) {
      globals->dmp->disableDataCollection(globals->perStream, listItem);
  }

  /*
  for (walk= *globals->mrlist; listItem= *walk; ++walk) {
      if (!(globals->mrlist->remove(listItem))) {
          perror("globals->mrlist->remove");
          ERROR_MSG(16,"remove() in VISIthreadmain"); 
      }
  }
  */

  // kill visi process
  if(!globals->start_up){
       kill(globals->pid,SIGKILL);
  }

  PARADYN_DEBUG(("before destroy perfomancestream"));
  if(!(globals->dmp->destroyPerformanceStream(context,globals->perStream))){
      ERROR_MSG(16,"remove() in VISIthreadmain");
  }

  // notify VM 
  PARADYN_DEBUG(("before notify VM of thread died"));
  globals->vmp->VMVisiDied(thr_self());
  PARADYN_DEBUG(("after notify VM of thread died"));

  // unbind file descriptor associated with visualization
  if(msg_unbind(globals->fd) == THR_ERR){
      PARADYN_DEBUG(("Error in msg_unbind(globals->fd)"));
      ERROR_MSG(14,"Error in VISIthreadmain: msg_unbind");
  }

  delete globals->mrlist;
  delete globals->visip;

  PARADYN_DEBUG(("leaving visithread main"));
  thr_exit(0);
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
      PARADYN_DEBUG(("thr_getspecific in visualizationUser::PhaseName"));
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
         fprintf(stderr, "VISIthread: Could not (un)marshall parameters, pid=%d tid=%d\n", getpid(),thr_self());
         ptr->quit = 1;
         break;

    case igen_call_err:
         fprintf(stderr, "VISIthread: can't do sync call here, pid = %d tid = %d\n",
	         getpid(),thr_self());
         ptr->quit = 1;
         break;

    case igen_request_err:
         fprintf(stderr, "VISIthread: unknown message tag pid=%d tid = %d\n",
	         getpid(),thr_self());
         ptr->quit = 1;
         break;

    case igen_send_err:
    case igen_read_err:
    default:
         fprintf(stderr, "VISIthread: Error: err_state = %d tid = %d\n", 
		 get_err_state(),thr_self());
         ptr->quit = 1;
         break;
  }
}


char *GetResourceName(resourceList *resource){

 char **y;
 char *tempName;
 int  i;
 int  where, numRes, totalSize = 4;

    y = resource->convertToStringList();
    numRes = resource->getCount();
    for(i = 0; i < numRes; i++)
	totalSize += strlen((char *) y[i]) + 1;
    if((tempName =
	    (char *)malloc(sizeof(char)*(totalSize))) == NULL){
	    free(y);
	    return(NULL);
     }
     memset(tempName,'\0',totalSize);
     where = 0;
     for(i = 0; i < numRes; i++){
	 if (!(strncpy(&(tempName[where]),
	    (char *) y[i],strlen((char *) y[i])))){
	    free(tempName);
	    free(y);
	    return(NULL);
	 }
	 where += strlen((char *) y[i]);
	 if( i < (numRes - 1)){
	     tempName[where++] = ',';
	 }
    }

    tempName[where] = '\0';

    PARADYN_DEBUG(("tempName = %s size = %d\n",tempName,totalSize));
    PARADYN_DEBUG(("tempName where = %d\n",where));
    free(y);
    return(tempName);

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
     newword[num] = '\0';
  }
  PARADYN_DEBUG(("abbreviated focus = %s size = %d\n",newword,size));
  PARADYN_DEBUG(("abbreviated focus num = %d",num));
  free(longName);
  return(newword);
}

