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
/* Revision 1.12  1994/06/29 21:46:40  hollings
/* fixed malloc on default focus case.
/*
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


/*
#define DEBUG2
#define DEBUG
#define DEBUG3
*/

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
    uiMgr->showError("thr_getspecific in VISIthread::VISIKillVisi");
    printf("error # :fatal or serious\n");
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
    uiMgr->showError("thr_getspecific in VISIthreadDataCallback");
    printf("error # :fatal or serious\n");
  }
  if((ptr->bufferSize >= BUFFERSIZE) || (ptr->bufferSize < 0)){
    PARADYN_DEBUG(("bufferSize out of range: VISIthreadDataCallback")); 
    uiMgr->showError("bufferSize out of range: VISIthreadDataCallback");
    printf("error # : serious\n");
    return;
  }
  if((ptr->buffer == NULL)){
    PARADYN_DEBUG(("buffer error: VISIthreadDataCallback")); 
    uiMgr->showError("buffer error: VISIthreadDataCallback");
    printf("error # : serious\n");
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
// TODO: check igen error value after call to see if socket has closed

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

 VISIthreadGlobals *ptr;
 int i;

#ifdef DEBUG2
 printf("in VISIthreadDataCallback: first = %d total = %d\n",first,total);
#endif
  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
    PARADYN_DEBUG(("thr_getspecific in VISIthreadDataCallback"));
    uiMgr->showError("thr_getspecific in VISIthreadDataCallback");
    printf("error # :fatal or serious\n");
  }

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
    uiMgr->showError("thr_getspecific in VISIthreadnewMetricCallback");
    printf("error # :fatal or serious\n");
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
    uiMgr->showError("thr_getspecific in VISIthreadnewResourceCallback");
    printf("error # :fatal or serious\n");
  }

}

///////////////////////////////////////////////////////
//  VISIthreadFoldCallback: callback for dataManager
//     histFold upcall
//
//  if thread's local data buffer is not empty send
//  the data buffer to the visualization process process 
//  before sending Fold msg to visi process  
///////////////////////////////////////////////////////
void VISIthreadFoldCallback(performanceStream *ps,
			timeStamp width){

 VISIthreadGlobals *ptr;
 dataValue_Array   temp;


  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
    PARADYN_DEBUG(("thr_getspecific in VISIthreadFoldCallback"));
    uiMgr->showError("thr_getspecific in VISIthreadFoldCallback");
    printf("error # :fatal or serious\n");
  }
  if((ptr->bufferSize >= BUFFERSIZE) || (ptr->bufferSize < 0)){
    PARADYN_DEBUG(("bufferSize out of range: VISIthreadFoldCallback")); 
    uiMgr->showError("bufferSize out of range: VISIthreadFoldCallback");
    printf("error # : serious\n");
    return;
  }
  if((ptr->buffer == NULL)){
    PARADYN_DEBUG(("buffer error: VISIthreadFoldCallback")); 
    uiMgr->showError("buffer error: VISIthreadFoldCallback");
    printf("error # : serious\n");
    return;
  }
  // if new Width is same as old width ignore Fold 
  if(ptr->bucketWidth != width){
    // if buffer is not empty send visualization buffer of data values
    if(ptr->bufferSize != 0){
      temp.count = ptr->bufferSize;
      temp.data = ptr->buffer;
      ptr->visip->Data(temp);
// TODO: check igen error value after call to see if socket has closed

      ptr->bufferSize = 0;
    }
    ptr->bucketWidth = width;
    // call visualization::Fold routine
    ptr->visip->Fold((double)width);
// TODO: check igen error value after call to see if socket has closed

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
 int numEnabled = 0;
 metricInstance *newEnabled[numMetrics];
 int i,j,found;
 metricType_Array metrics;
 resourceType_Array resources;
 metricInstance *temp;
 timeStamp binWidth = 0.0;
 int numBins = 0;
 char **y;
 int totalSize, where;
 metricInfo *temp2;
 char errorString[128];
 sampleValue buckets[1000];
 int howmany;
 char *key;
 int  numFoci = 0;
 dataValue_Array   tempdata;
 char *tempName;


  PARADYN_DEBUG(("In VISIthreadchooseMetRes numMetrics = %d",numMetrics));

  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
    PARADYN_DEBUG(("thr_getspecific in VISIthreadchooseMetRes"));
    uiMgr->showError("thr_getspecific in VISIthreadchooseMetRes");
    printf("error # :fatal or serious\n");
  }

  // temporary check for invalid reply
  // this will be handled by error msgs later
  if((numMetrics <= 0) || (focusChoice == NULL)){
    PARADYN_DEBUG(("no metric and resource in VISIthreadchooseMetRes"));
    uiMgr->showError("Incomplete metric or focus list");
    printf("error # :informational\n");
    return;
  }

  key = focusChoice->getCanonicalName();

  PARADYN_DEBUG(("\n$$$$ in VISIthreadchooseMetRes callback: numMetrics = %d key = %d",numMetrics,key));

  // determine if this focus has been enabled before
  found = 0;
  if(!(ptr->mrlist->empty())){
      found = 0;
      ptr->mrlist->setCurrent();
      while((!found) &&((temp = ptr->mrlist->getCurrent()) != 0)){
	  if( temp->focus->getCanonicalName() == key )
	      found = 1;
          ptr->mrlist->advanceCurrent();
      }
  }
  
  // this is a new focus, enable all metric/focus pairs
  if(!found){  

  PARADYN_DEBUG(("new focus = %d",key));

    for(i=0;i<numMetrics;i++){

      // convert metricName to metric* 
      if((currMetric = ptr->dmp->findMetric(context,metricNames[i])) != 0){
        // make enable request to DM
        //if successful, add metricInstance to mrlist  
	PARADYN_DEBUG(("before enable metric/focus\n"));

        if((currMetInst = 
	     ptr->dmp->enableDataCollection(ptr->perStream,
	     focusChoice,currMetric,Sum)) 
	     != NULL){
	    PARADYN_DEBUG(("after enable metric/focus\n"));
            ptr->mrlist->add(currMetInst,currMetInst);
	    newEnabled[numEnabled] = currMetInst;
            numEnabled++;
#ifdef DEBUG
            fprintf(stderr,"currMetInst %d, focus %d metric %d\n",
	       currMetInst, currMetInst->focus, currMetInst->met);
#endif
        }
      }

      else {
         // there is an error with findMetric
         sprintf(errorString,"dataManager::findMetric failed (returned NULL)for metric %s.",metricNames[i]);
	 printf("%s\n",errorString);
	 uiMgr->showError(errorString);
         printf("error # :serious\n");
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
	  ptr->mrlist->setCurrent();
	  while((!found) &&((temp = ptr->mrlist->getCurrent()) != 0)){
	      if((temp->focus->getCanonicalName() == key)
		  && (temp->met == currMetric)){
	          found = 1;
              }
              ptr->mrlist->advanceCurrent();
	  }

	  if(!found){ // enable
              if((currMetInst = ptr->dmp->enableDataCollection(ptr->perStream,
		  focusChoice, currMetric,Sum)) != NULL){

                  ptr->mrlist->add(currMetInst,currMetInst);
	          newEnabled[numEnabled] = currMetInst;
#ifdef DEBUG
            fprintf(stderr,"currMetInst %d, focus %d metric %d\n",
	       currMetInst, currMetInst->focus, currMetInst->met);
#endif
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

        perror("malloc in VISIthreadchooseMetRes"); 
        uiMgr->showError("malloc in VISIthreadchooseMetRes");
        printf("error # : serious\n");
        return;
    }

    if((metrics.data=(metricType *)malloc(sizeof(metricType)*numEnabled))
	== (metricType *)NULL){

        perror("malloc in VISIthreadchooseMetRes"); 
        uiMgr->showError("malloc in VISIthreadchooseMetRes");
        printf("error # : serious\n");
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
        totalSize += strlen(y[i]);
 
    totalSize += numFoci;  // space for commas

    if((tempName = 
	(char *)malloc(sizeof(char)*(totalSize +1))) == NULL){
        perror("malloc in VISIthreadchooseMetRes");
        uiMgr->showError("malloc in VISIthreadchooseMetRes");
        printf("error # : serious\n");
        return ;
    }
    where = 0;

    for(i = 0; i < numFoci; i++){
        if((strncpy(&(tempName[where]),y[i],strlen(y[i])))
	    ==NULL){
            perror("strncpy in VISIthreadchooseMetRes");
            uiMgr->showError("strncpy in VISIthreadchooseMetRes");
            printf("error # : serious\n");
            return;
        }
        where += strlen(y[i]);
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
	  ptr->bufferSize = 0;
    }

    ptr->visip->AddMetricsResources(metrics,resources,binWidth,numBins);


    // get old data bucket values for new metric/resources and
    // send them to visualization
    for(i=0;i<numEnabled;i++){
        howmany = ptr->dmp->getSampleValues(newEnabled[i],buckets,1000,0);

        // if the current bucket value is valid add it to the data
        // buffer associated with this visualization
        for(j=0;j<howmany;j++){  
           if(!(isnan(buckets[j]))){
              VISIthreadDataHandler(ptr->perStream,
				    newEnabled[i],j,
				    buckets[j]);
           } 
	} 
    }

    free(metrics.data);
    free(resources.data);
    free(tempName); 
    free(y);
  }
  else {
      uiMgr->showErrorWait("No enabled Metric/focus pairs",0,NULL);
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
    uiMgr->showError("thr_getspecific in visiUser::GetMetricResource");
    printf("error # :fatal or serious\n");
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


  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
    PARADYN_DEBUG(("thr_getspecific in visualizationUser::StopMetricResource"));
    uiMgr->showError("thr_getspecific in visualizationUser::StopMetricResource");
    printf("error # :fatal or serious\n");
  }


  // search metricList for matching metricId and resourceId
  // if found request DM to disable data collection of metricInstance
  if(!ptr->mrlist->empty()){
    ptr->mrlist->setCurrent();
    while(!found && ((listItem = ptr->mrlist->getCurrent()) != 0)){
      //if metric and resource match set found to 1 
      if((listItem->met == (metric *)metricId) &&
	(listItem->focus->getCanonicalName() == (char *)resourceId)){
         found = 1;
      }
#ifdef DEBUG3
      fprintf(stderr,"current list element: metId = %d resId = %d\n",
       		(int)listItem->met,(int)listItem->focus);
#endif
      ptr->mrlist->advanceCurrent();
    }

#ifdef DEBUG3
    if(found){
     fprintf(stderr,"in visualizationUser::StopMetricResource: mi found\n");
    }
    else{
     fprintf(stderr,"in visualizationUser::StopMetricResource: mi not found\n");
     fprintf(stderr,"metricId = %d resourceId = %d\n",metricId,resourceId);
    }
#endif

    if(found){
      //make disable request to DM and remove this metric instance from list
      ptr->dmp->disableDataCollection(ptr->perStream,listItem);
      if(!(ptr->mrlist->remove(listItem))){
        perror("ptr->mrlist->remove"); 
        return;
      }
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
    uiMgr->showError("thr_getspecific in visualizationUser::PhaseName");
    printf("error # :fatal or serious\n");
  }

}


///////////////////////////////////////////////////////////////////
//  VISIthread main routine
//  input: parent thread tid, visualization command line arguments
//
//  initializes thread local variables, starts visualization process
//  and enters main loop
///////////////////////////////////////////////////////////////////
void *VISIthreadmain(visi_thread_args *args){ 
 
  int from;
  thread_t tag;
  VISIthreadGlobals *globals;
  VISIthread *vtp;
  int died;
  controlCallback callbacks;
  metricInstance *listItem;
  union dataCallback dataHandlers;



  //initialize global variables

  if((globals=(VISIthreadGlobals *)malloc(sizeof(VISIthreadGlobals)))==0){
    PARADYN_DEBUG(("Error in malloc globals"));
    uiMgr->showError("error  in malloc globals in VISIthreadmain");
    printf("error # : serious");
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
  globals->fd = RPCprocessCreate(&globals->pid, "localhost", "",args->argv[0],args->argv);

  if (globals->fd < 0) {
    PARADYN_DEBUG(("Error in process Create"));
    printf("Error in process Create : serious");
    uiMgr->showError("error in VISIthreadmain: process Create");
    printf("error # : serious");
    globals->quit = 1;
  }

  globals->visip = new visiUser(globals->fd,NULL,NULL); 

  if(msg_bind(globals->fd,0) != THR_OKAY) {
    PARADYN_DEBUG(("Error in msg_bind(globals->fd)"));
    uiMgr->showError("error in VISIthreadmain: msg_bind()");
    printf("error # : serious");
    globals->quit = 1;
  }

  globals->bucketWidth = globals->dmp->getCurrentBucketWidth(); 

  if (thr_setspecific(visiThrd_key, globals) != THR_OKAY) {
    PARADYN_DEBUG(("Error in thr_setspecific"));
    uiMgr->showError("error in thr_setspecific");
    printf("error # : serious");
    globals->quit = 1;
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
    uiMgr->showError("error in createPerformanceStream in VISImain");
    printf("error # : serious");
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
    }
    else {
       vtp->mainLoop();
    }
  }


  // disable all metricInstance data collection
  globals->mrlist->setCurrent();
  while((listItem = globals->mrlist->getCurrent()) != 0){
    globals->dmp->disableDataCollection(globals->perStream,listItem);
    if(!(globals->mrlist->remove(listItem))){
      perror("globals->mrlist->remove");
    }
    globals->mrlist->advanceCurrent();
  }

  PARADYN_DEBUG(("leaving visithread main"));
  thr_exit(0);
}

// handle_error is a virtual function in the igen generated code
// defining it allows for custom error handling routines to be implemented
// the error types are defined in igen generated code, but should be
// relatively stable
//
// THESE are elaborated in the igen documentation (coming soon)
// 
// igen_no_err --> no error
// igen_decode_err --> an error occurred while unmarshalling data
// igen_encode_err --> an error occurred while marshalling data
// igen_send_err   --> an error occurred while sending a message
// igen_read_err --> an error occurred while receiving a messaged
// igen_call_err --> attempt to do a sync call when in an async call handler
// igen_request_err --> received an unknown message tag, or a response
//                      message tag that was unexpected
//
// cannot call exit here, since a thread should not cause paradyn to exit
void visiUser::handle_error()
{
  fprintf(stderr, "Error in visiUser \n");

  // err_state is set by the event that caused the error
  switch (err_state)
    {
    case igen_encode_err:
    case igen_decode_err:
      fprintf(stderr, "Could not (un)marshall parameters, dumping core, pid=%d\n",
	      getpid());
      break;

    case igen_call_err:
      fprintf(stderr, "can't do sync call here, pid=%d\n",
	      getpid());
      break;

    case igen_request_err:
      fprintf(stderr, "unknown message tag pid=%d\n",
	      getpid());
      break;

    case igen_no_err:
      fprintf(stderr, "Why is handle error called for err_state = igen_no_err\n");
      // fall thru
    case igen_send_err:
    case igen_read_err:
      // if paradyn quits either of these errors can occur, so don't dump core
    default:
      fprintf(stderr, "Error: err_state = %d\n", err_state);
    }
}

char *AbbreviatedFocus(char *longName){

int i,size,num = 0;
int next = 0;
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


