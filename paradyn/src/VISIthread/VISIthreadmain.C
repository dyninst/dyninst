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
// * visualizationUser routines:  GetMetricResource, PhaseName
//		StopMetricResource
// * VISIthread server routines:  VISIKillVisi
/////////////////////////////////////////////////////////////////////
/* $Log: VISIthreadmain.C,v $
/* Revision 1.21  1994/08/05 16:04:33  hollings
/* more consistant use of stringHandle vs. char *.
/*
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
#include "thread/h/thread.h"
#include "util/h/list.h"
#include "util/h/rpcUtil.h"
#include "VM.CLNT.h"
#include "UI.CLNT.h"
#include "dataManager.CLNT.h"
#include "visi.CLNT.h"
#include "VISIthread.SRVR.h"
#include "../VMthread/VMtypes.h"
#include "VISIthreadTypes.h"
#include "../pdMain/paradyn.h"
#include "dyninstRPC.CLNT.h"
#include "../DMthread/DMinternals.h"
#define  VISI_DEFAULT_FOCUS "Root Nodes"
#define  ERROR_MSG(s1, s2) \
	 uiMgr->showError(s1,s2); \
	 printf("error# %d: %s\n",s1,s2); 

char *AbbreviatedFocus(char *);

//////////////////////////////////////////////////
// VISIKillVisi:  VISIthread server routine 
//
//  called from VisiMgr, kills the visualization 
//  process and sets thread local variable "quit"
//  so that the VISIthread will die 
//////////////////////////////////////////////////
 void VISIthread::VISIKillVisi(){

 VISIthreadGlobals *ptr;


  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
    PARADYN_DEBUG(("thr_getspecific in VISIthread::VISIKillVisi"));
    ERROR_MSG(13,"thr_getspecific in VISIthread::VISIKillVisi");
    return;
  }

  ptr->quit = 1;
  kill(ptr->pid,SIGKILL);

}


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


///////////////////////////////////////////////////////////////////
//  VISIthreadchooseMetRes: callback for User Interface Manager 
//    chooseMetricsandResources upcall
//    input: list of metric names, list size, pointer to focus
//
//  if the focus has already been enabled, make enable requests to
//  the dataManager for only those metric/focus pairs that have not
//  been previously enabled for this visualization
//  else try to enable all metric/focus pairs
//
//  send each successfully enabled metric and focus to visualization 
//  process (call visualizationUser::AddMetricsResources)
///////////////////////////////////////////////////////////////////
void VISIthreadchooseMetRes(char **metricNames,
			    int numMetrics,
		            resourceList* focusChoice){
 VISIthreadGlobals *ptr;
 metric *currMetric;
 metricInstance *currMetInst;
 metricInstance *newEnabled[numMetrics];
 metricType_Array metrics;
 resourceType_Array resources;
 metricInstance *temp;
 metricInfo *temp2;
 sampleValue buckets[1000];
 dataValue_Array   tempdata;
 timeStamp binWidth = 0.0;
 int  numEnabled = 0;
 int  i,j,found;
 int  numBins = 0;
 stringHandle *y;
 int  totalSize, where;
 char errorString[128];
 int  howmany;
 int  numFoci = 0;
 char *tempName;
 stringHandle key;
 float_Array bulk_data;
 int_Array metricIds;
 List<metricInstance*> walk;

  PARADYN_DEBUG(("In VISIthreadchooseMetRes numMetrics = %d",numMetrics));

  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
    PARADYN_DEBUG(("thr_getspecific in VISIthreadchooseMetRes"));
    ERROR_MSG(13,"thr_getspecific in VISIthread::VISIthreadchooseMetRes");
    return;
  }

  // check for invalid reply
  if((numMetrics <= 0) || (focusChoice == NULL)){
    PARADYN_DEBUG(("no metric and resource in VISIthreadchooseMetRes"));
    ERROR_MSG(17,"Incomplete metric or focus list::VISIthreadchooseMetRes");
    return;
  }

  key = focusChoice->getCanonicalName();

  PARADYN_DEBUG(("\nin VISIthreadchooseMetRes callback: numMetrics = %d key = %d",numMetrics,key));

  // determine if this focus has been enabled before
  found = 0;
  for (walk= *ptr->mrlist; temp=*walk; ++walk) 
    if (temp->focus->getCanonicalName() == key) {
      found = 1;
      break;
    }

  // this is a new focus, enable all metric/focus pairs
  if(!found){  

  PARADYN_DEBUG(("new focus = %d",key));

    for(i=0;i<numMetrics;i++){

      // convert metricName to metric* 
      if((currMetric= ptr->dmp->findMetric(context,metricNames[i])) != 0){
        // make enable request to DM
        //if successful, add metricInstance to mrlist  
	PARADYN_DEBUG(("before enable metric/focus\n"));

        if((currMetInst = 
	     ptr->dmp->enableDataCollection(ptr->perStream,
	     focusChoice,currMetric)) 
	     != NULL){
	    PARADYN_DEBUG(("after enable metric/focus\n"));
            ptr->mrlist->add(currMetInst,currMetInst);
	    newEnabled[numEnabled] = currMetInst;
            numEnabled++;
        }
      }

      else {
         // there is an error with findMetric
         sprintf(errorString,"dataManager::findMetric failed (returned NULL)for metric %s.",metricNames[i]);
         ERROR_MSG(17,errorString);
         ptr->quit = 1;
	 return;
      }
    }
  }

  else { // need to determine wch metrics are new for this focus
    PARADYN_DEBUG(("old focus = %d",key));

    for(i=0;i<numMetrics;i++){

      if((currMetric = ptr->dmp->findMetric(context,metricNames[i])) != 0){
	  // search mrlist for metricInstance assoc. w/ currMetric & focus
	  found = 0;
	  for (walk= *ptr->mrlist; temp = *walk; walk++) 
	    if ((temp->focus->getCanonicalName() == key) &&
		(temp->met == currMetric)) {
	      found = 1;
 	      break;
	    }

	  if(!found){ // enable
              if((currMetInst = ptr->dmp->enableDataCollection(ptr->perStream,
		  focusChoice, currMetric)) != NULL){

                  ptr->mrlist->add(currMetInst,currMetInst);
	          newEnabled[numEnabled] = currMetInst;
                  PARADYN_DEBUG(("currMetInst %d, focus %d metric %d\n",currMetInst, currMetInst->focus, currMetInst->met));
                  numEnabled++;
              }
	  }

      }
    }
  }

  if(numEnabled > 0){
    // create metric and resource arrays and send to visualization
    metrics.count = numEnabled;
    resources.count = 1;

    if((resources.data=(resourceType *)malloc(sizeof(resourceType)))
        == (resourceType *)NULL){
	ERROR_MSG(12,"in VISIthreadchooseMetRes");
        ptr->quit = 1;
        return;
    }

    if((metrics.data=(metricType *)malloc(sizeof(metricType)*numEnabled))
	== (metricType *)NULL){
	ERROR_MSG(12,"in VISIthreadchooseMetRes");
        ptr->quit = 1;
        return;
    }

    for(i=0;i<numEnabled;i++){
        temp2 = newEnabled[i]->met->getInfo();
        metrics.data[i].name = strdup(temp2->name);
        metrics.data[i].units = strdup(temp2->units);
        if(temp2->style == MetStyleEventCounter)
            metrics.data[i].aggregate = AVE;
        else 
            metrics.data[i].aggregate = AVE;
        metrics.data[i].Id = (int)newEnabled[i]->met;
    }
   
    resources.data[0].Id = (int)key;

    // create a resource name
    y = newEnabled[0]->focus->convertToStringList();
    totalSize = 0;

    numFoci = newEnabled[0]->focus->getCount();

    for(i = 0; i < numFoci; i++)
        totalSize += strlen((char *) y[i]);
 
    totalSize += numFoci;  // space for commas

    if((tempName = 
	(char *)malloc(sizeof(char)*(totalSize +1))) == NULL){
	ERROR_MSG(12,"in VISIthreadchooseMetRes");
        ptr->quit = 1;
        return ;
    }
    where = 0;

    for(i = 0; i < numFoci; i++){
        if (!(strncpy(&(tempName[where]), 
		      (char *) y[i],strlen((char *) y[i])))){
	    ERROR_MSG(12,"strncpy in VISIthreadchooseMetRes");
            ptr->quit = 1;
            return;
        }
        where += strlen((char *) y[i]);
	if( i < (numFoci - 1)){
	  tempName[where++] = ',';
	}
    }

    tempName[where] = '\0';

    binWidth = ptr->dmp->getCurrentBucketWidth(); 
    numBins = ptr->dmp->getMaxBins();

    resources.data[0].name = AbbreviatedFocus(tempName);

    // if buffer is not empty send visualization buffer of data values
    if(ptr->bufferSize != 0){
	  tempdata.count = ptr->bufferSize;
          tempdata.data = ptr->buffer;
	  ptr->visip->Data(tempdata);
          if(ptr->visip->did_error_occur()){
             PARADYN_DEBUG(("igen: visip->Data() in VISIthreadchooseMetRes"));
             ptr->quit = 1;
             return;
          }
	  ptr->bufferSize = 0;
    }

    PARADYN_DEBUG(("before call to AddMetricsResources\n"));
    ptr->visip->AddMetricsResources(metrics,resources,binWidth,numBins);
    if(ptr->visip->did_error_occur()){
        PARADYN_DEBUG(("igen: visip->AddMetricsResources() in VISIthreadchooseMetRes"));
        ptr->quit = 1;
        return;
    }


    // get old data bucket values for new metric/resources and
    // send them to visualization
    for(i=0;i<numEnabled;i++){
        howmany = ptr->dmp->getSampleValues(newEnabled[i],
					    buckets,1000,0);
#ifdef DEBUG3
	printf("howmany = %d after call to dmp->getSampleValues for metricInstance %d\n",howmany,(int)newEnabled[i]);
#endif

        // send visi all old data bucket values
	if(howmany > 0){
            bulk_data.count = howmany; 
            bulk_data.data = buckets; 
            ptr->visip->BulkDataTransfer(bulk_data,
		   (int)newEnabled[i]->met,
		   (int)newEnabled[0]->focus->getCanonicalName());
            if(ptr->visip->did_error_occur()){
                PARADYN_DEBUG(("igen: visip->BulkDataTransfer() in VISIthreadchooseMetRes"));
                ptr->quit = 1;
                return;
            }
	}

    }

    free(metrics.data);
    free(resources.data);
    free(metricIds.data);
    free(tempName); 
    free(y);
  }
  else {
      ERROR_MSG(17,"No enabled Metric/focus pairs: VISIthreadchooseMetRes");
  }
}


///////////////////////////////////////////////////////////////////
//  VISIthreadchosenMetsRes: callback for User Interface Manager 
//    input: list of metric names, list size, list of focuses, list size
//
//  for each focus call VISIthreadchooseMetRes 
//
///////////////////////////////////////////////////////////////////
void VISIthreadchoosenMetsRes(char **metricNames,
			    int numMetrics,
		            resourceList** focusChoice,
			    int numResources){

    for(int i = 0; i < numResources; i++){
      VISIthreadchooseMetRes(metricNames,numMetrics,focusChoice[i]);
    }
}



////////////////////////////////////////////////////////
//  VISIthreadshowMsgREPLY: callback for User Interface 
//    Manager showMsgREPLY upcall (not currently implemented) ///////////////////////////////////////////////////////
void VISIthreadshowMsgREPLY(int userChoice){


}

////////////////////////////////////////////////////////
//  VISIthreadshowErrorREPLY: callback for User Interface 
//    Manager showErrorREPLY upcall (not currently implemented)
///////////////////////////////////////////////////////
void VISIthreadshowErrorREPLY(int userChoice){
  

}


//////////////////////////////////////////////////////////////////////
//  GetMetricResource: visualizationUser routine (called by visi process)
//  input: string of metric names, string of focus names, type of data
//         (0: histogram, 1: scalar) currently only 0 supported
//
// check if metric and resource lists have wild card chars 
// if so request metrics and resources form UIM (currently, the
// only option), else make enable data collection call to DM for each
// metric resource pair
//////////////////////////////////////////////////////////////////////
void visualizationUser::GetMetricResource(String metric,
					  String resource,
					  int type){
 VISIthreadGlobals *ptr;

PARADYN_DEBUG(("in visualizationUser::GetMetricResource"));
 if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
    PARADYN_DEBUG(("thr_getspecific in visiUser::GetMetricResource"));
    ERROR_MSG(13,"thr_getspecific in VISIthread::GetMetricResource");
    return;
 }
 ptr->ump->chooseMetricsandResources((chooseMandRCBFunc)VISIthreadchooseMetRes);
}


//////////////////////////////////////////////////////////////////////
//  StopMetricResource: visualizationUser routine (called by visi process)
//  input: metric and resource Ids 
//
//  if metricId and resourceId are valid, make disable data collection
//  call to dataManager for the pair, and remove the associated metric
//  instance from the threads local mrlist
//////////////////////////////////////////////////////////////////////
void visualizationUser::StopMetricResource(int metricId,
					   int resourceId){
 VISIthreadGlobals *ptr;
 metricInstance *listItem;
 int found = 0;
 List<metricInstance*> walk;

  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
    PARADYN_DEBUG(("thr_getspecific in visualizationUser::StopMetricResource"));
    ERROR_MSG(13,"thr_getspecific in VISIthread::StopMetricResource");
    return;
  }


  // search metricList for matching metricId and resourceId
  // if found request DM to disable data collection of metricInstance

  found = 0;
  for (walk = *ptr->mrlist; listItem=*walk; walk++) {
    if ((listItem->met == (metric*) metricId) &&
	(listItem->focus->getCanonicalName() == (char*) resourceId)) {
      found = 1;
      break;
    }
      PARADYN_DEBUG(("current list element: metId = %d resId = %d",
		     (int)listItem->met,(int)listItem->focus));
  }

#ifdef DEBUG
    if(found){
     PARADYN_DEBUG(("in visualizationUser::StopMetricResource: mi found"));
    }
    else{
     PARADYN_DEBUG(("visualizationUser::StopMetricResource: mi not found\n"));
     PARADYN_DEBUG(("metricId = %d resourceId = %d\n",metricId,resourceId));
    }
#endif

    if(found){
      //make disable request to DM and remove this metric instance from list
      ptr->dmp->disableDataCollection(ptr->perStream,listItem);
      if(!(ptr->mrlist->remove(listItem))){
        perror("ptr->mrlist->remove"); 
	ERROR_MSG(16,"remove() in StopMetricResource()");
	ptr->quit = 1;
        return;
      }
    }
}



///////////////////////////////////////////////////////////////////
//  PhaseName: visualizationUser routine (called by visi process)
//  input: name of phase, begining and ending timestamp for phase 
//
//  not currently implemented
///////////////////////////////////////////////////////////////////
void visualizationUser::PhaseName(double begin,
				  double end,
				  String name){

 VISIthreadGlobals *ptr;

  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
    PARADYN_DEBUG(("thr_getspecific in visualizationUser::PhaseName"));
    ERROR_MSG(13,"thr_getspecific in VISIthread::PhaseName");
    return;
  }

}


///////////////////////////////////////////////////////////////////
//  VISIthread main routine
//  input: parent thread tid, visualization command line arguments
//
//  initializes thread local variables, starts visualization process
//  and enters main loop
///////////////////////////////////////////////////////////////////
void *VISIthreadmain(void *vargs){ 
 
  visi_thread_args* args = (visi_thread_args *) vargs;
  int from;
  thread_t tag;
  VISIthreadGlobals *globals;
  VISIthread *vtp;
  controlCallback callbacks;
  metricInstance *listItem;
  union dataCallback dataHandlers;
  List<metricInstance*> walk;

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

  globals->bufferSize = 0;
  globals->fd = -1;
  globals->pid = -1;
  globals->quit = 0;
  globals->mrlist = new(List<metricInstance *>);


  // start visualization process
  PARADYN_DEBUG(("in visi thread"));
  globals->fd = RPCprocessCreate(&globals->pid, "localhost", "",
				 args->argv[0],args->argv);
  if (globals->fd < 0) {
    PARADYN_DEBUG(("Error in process Create"));
    ERROR_MSG(14,"Error in VISIthreadmain: RPCprocessCreate");
    globals->quit = 1;
  }

  globals->visip = new visiUser(globals->fd,NULL,NULL); 

  if(msg_bind_buffered(globals->fd,0,xdrrec_eof,globals->visip->__xdrs__) 
     != THR_OKAY) {
    PARADYN_DEBUG(("Error in msg_bind(globals->fd)"));
    ERROR_MSG(14,"Error in VISIthreadmain: msg_bind_buffered");
    globals->quit = 1;
    kill(globals->pid,SIGKILL);
  }

  globals->bucketWidth = globals->dmp->getCurrentBucketWidth(); 

  if (thr_setspecific(visiThrd_key, globals) != THR_OKAY) {
    PARADYN_DEBUG(("Error in thr_setspecific"));
    ERROR_MSG(14,"Error in VISIthreadmain: thr_setspecific");
    globals->quit = 1;
    kill(globals->pid,SIGKILL);
  }


  // set control callback routines 
  callbacks.mFunc = (metricInfoCallback)VISIthreadnewMetricCallback;
  callbacks.rFunc = (resourceInfoCallback)VISIthreadnewResourceCallback;
  callbacks.fFunc = (histFoldCallback)VISIthreadFoldCallback;
  callbacks.sFunc = NULL;

  PARADYN_DEBUG(("before create performance stream in visithread"));
  // create performance stream
  dataHandlers.sample = (sampleDataCallbackFunc)VISIthreadDataCallback;
  if((globals->perStream = globals->dmp->createPerformanceStream(context,
		   Sample,BASE,dataHandlers,callbacks)) == NULL){
    PARADYN_DEBUG(("Error in createPerformanceStream"));
    ERROR_MSG(15,"Error in VISIthreadmain: createPerformanceStream");
    globals->quit = 1;
    kill(globals->pid,SIGKILL);
  }

 
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
    else if ((RPC_readReady(globals->fd) == -1)) {
	globals->quit = 1;  // visualization process has died
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

  // disable all metricInstance data collection
  for (walk= *globals->mrlist; listItem= *walk; ++walk) {
    globals->dmp->disableDataCollection(globals->perStream, listItem);
    if (!(globals->mrlist->remove(listItem))) {
      perror("globals->mrlist->remove");
      ERROR_MSG(16,"remove() in VISIthreadmain"); 
    }
  }

  // notify VM 
  globals->vmp->VMVisiDied(thr_self());

  // unbind file descriptor associated with visualization
  if(msg_unbind(globals->fd) == THR_ERR){
    PARADYN_DEBUG(("Error in msg_unbind(globals->fd)"));
    ERROR_MSG(14,"Error in VISIthreadmain: msg_unbind");
  }

  delete globals->mrlist;
  free (globals);

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
  switch (err_state) {
    case igen_no_err:
         fprintf(stderr,"Handle error called for igen_no_err tid = %d\n",
		 thr_self());
         break;

    case igen_encode_err:
    case igen_decode_err:
         fprintf(stderr, "VISIthread: Could not (un)marshall parameters, pid=%d tid=%d\n",
		 getpid(),thr_self());
         ERROR_MSG(16,
	  "IGEN ERROR igen_(d,en)code_err: Could not (un)marshall parameters");
         ptr->quit = 1;
         break;

    case igen_call_err:
         fprintf(stderr, "VISIthread: can't do sync call here, pid = %d tid = %d\n",
	         getpid(),thr_self());
         ERROR_MSG(16,"IGEN ERROR igen_call_err: can't do sync call here"); 
         ptr->quit = 1;
         break;

    case igen_request_err:
         fprintf(stderr, "VISIthread: unknown message tag pid=%d tid = %d\n",
	         getpid(),thr_self());
         ERROR_MSG(16,"IGEN ERROR igen_request_err: unknown message tag"); 
         ptr->quit = 1;
         break;

    case igen_send_err:
    case igen_read_err:
    default:
         fprintf(stderr, "VISIthread: Error: err_state = %d tid = %d\n", 
		 err_state,thr_self());
         ERROR_MSG(16,"IGEN ERROR igen_(send,read)_err"); 
         ptr->quit = 1;
         break;
  }
  if(ptr->quit == 1){
     kill(ptr->pid,SIGKILL);
  }
}

char *AbbreviatedFocus(char *longName){

int i,size,num = 0;
int flag  = 0;
char *newword;
int nextFocus = 0;

  size = strlen(longName) +1; 
  newword = (char *)malloc(size);

  for(i = 0; i < size; i++){
      if(longName[i] == '/'){
	  if(!nextFocus){
	     nextFocus = 1;
          }
	  else { 
             flag = 1;
	  }
      }
      else if(longName[i] == ','){
	  nextFocus = 0;
	  flag = 0;
      }
      if(flag){
	  newword[num] = longName[i]; 
	  num++;
      }
  }
  if(num == 0) {
     free(newword);
     newword = (char *) malloc(strlen(VISI_DEFAULT_FOCUS)+1);
     strcpy(newword,VISI_DEFAULT_FOCUS);
  } else {
     newword[num] = '\0';
  }
  return(newword);
}


