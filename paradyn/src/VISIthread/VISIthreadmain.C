/* $Log: VISIthreadmain.C,v $
/* Revision 1.1  1994/04/09 21:23:21  newhall
/* test version
/* */
#include <signal.h>
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


/////// VISIthread server routine

 void VISIthread::VISIKillVisi(){

 VISIthreadGlobals *ptr;


  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
    perror("thr_getspecific");
  }

  ptr->quit = 1;
  kill(ptr->pid,SIGKILL);

}


/////// callback routines for DataManager Upcalls

void VISIthreadDataCallback(performanceStream *ps,metricInstance *mi,
	               timeStamp startTimeStamp,timeStamp endTimeStamp,
		       sampleValue value){

 VISIthreadGlobals *ptr;
 dataValue_Array   temp;
 timeStamp         width;

  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
    perror("thr_getspecific");
    return;
  }

  // add data value to buffer
  ptr->buffer[ptr->bufferSize].data = value;

  ptr->buffer[ptr->bufferSize].metricId = (int)(mi->met);
  ptr->buffer[ptr->bufferSize].resourceId = (int)mi->focus;
  width = ptr->dmp->getCurrentBucketWidth();
  ptr->buffer[ptr->bufferSize].bucketNum = (int)(startTimeStamp/width);
  ptr->bufferSize++;

  // if buffer is full, send buffer to visualization
  if(ptr->bufferSize == BUFFERSIZE){
    temp.count = ptr->bufferSize;
    temp.data = ptr->buffer;
    ptr->visip->Data(temp);
    ptr->bufferSize = 0;
  }
}

void VISIthreadnewMetricCallback(performanceStream *ps,metric *m){

 VISIthreadGlobals *ptr;

  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
    perror("thr_getspecific");
    return;
  }
}

void VISIthreadnewResourceCallback(performanceStream *ps,resource *parent,
			      resource *newResource, char *name){

 VISIthreadGlobals *ptr;

  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
    perror("thr_getspecific");
    return;
  }

}


void VISIthreadFoldCallback(performanceStream *ps,timeStamp width){

 VISIthreadGlobals *ptr;
 dataValue_Array   temp;

  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
    perror("thr_getspecific");
    return;
  }
  // if buffer is not empty send visualization buffer of data values
  if(ptr->bufferSize != 0){
    temp.count = ptr->bufferSize;
    temp.data = ptr->buffer;
    ptr->visip->Data(temp);
    ptr->bufferSize = 0;
  }
  // call visualization::Fold routine
  ptr->visip->Fold((double)width);

}


/////// callback routines for UIM Upcalls

void VISIthreadchooseMetRes(char **metricNames,int numMetrics,
		       resourceList* focusChoice){
 VISIthreadGlobals *ptr;
 metric *currMetric;
 metricInstance *currMetInst;
 int numEnabled = 0;
 metricInstance *newEnabled[numMetrics];
 int i,found;
 metricType_Array metrics;
 resourceType_Array resources;
 metricInstance *temp;
 timeStamp binWidth = 0.0;
 int numBins = 0;
 char **y;
 int totalSize, where;
 metricInfo *temp2;


  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
    perror("thr_getspecific");
    return;
  }
  
  // determine if this focus has been enabled before
  found = 0;
  ptr->mrlist.setCurrent();
  while((!found) &&((temp = ptr->mrlist.getCurrent()) != 0)){
    if(temp->focus == focusChoice)
      found = 1;
    ptr->mrlist.advanceCurrent();
  }

  if(!found){  // this is a new focus, try to enable all metric/focus pairs
    for(i=0;i<numMetrics;i++){
      // convert metricName to metric* 
      if((currMetric = ptr->dmp->findMetric(context,metricNames[i])) != 0){
        // make enable request to DM
        //if successful, add metricInstance to mrlist  
        if((currMetInst= ptr->dmp->enableDataCollection(ptr->perStream,focusChoice,
            currMetric)) != NULL){
            ptr->mrlist.add(currMetInst,currMetInst);
	    newEnabled[numEnabled] = currMetInst;
            numEnabled++;
        }
      }
    }
  }
  else { // need to determine wch metrics are new for this focus
    for(i=0;i<numMetrics;i++){
      if((currMetric = ptr->dmp->findMetric(context,metricNames[i])) != 0){
	// search mrlist for a metricInstance corr. to currMetric focusChoice
	found = 0;
	ptr->mrlist.setCurrent();
	while((!found) &&((temp = ptr->mrlist.getCurrent()) != 0)){
	  if((temp->focus == focusChoice) && (temp->met == currMetric))
	    found = 1;
          ptr->mrlist.advanceCurrent();
	}
	if(!found){
	  // enable 
          if((currMetInst=ptr->dmp->enableDataCollection(ptr->perStream,
	      focusChoice,currMetric)) != NULL){
            ptr->mrlist.add(currMetInst,currMetInst);
	    newEnabled[numEnabled] = currMetInst;
            numEnabled++;
          }
	}
	// else this has already been enabled
      }
    }
  }

  // create metric and resource arrays and send to visualization
  metrics.count = numEnabled;
  resources.count = 1;
  if((resources.data=(resourceType *)malloc(sizeof(resourceType))) ==
      (resourceType *)NULL){
    perror("malloc in VISIthreadchooseMetRes"); 
    return;
  }
  if((metrics.data=(metricType *)malloc(sizeof(metricType)*numEnabled)) ==
      (metricType *)NULL){
    perror("malloc in VISIthreadchooseMetRes"); 
    return;
  }
  for(i=0;i<numEnabled;i++){
    temp2 = newEnabled[i]->met->getInfo();
    metrics.data[i].name = strdup(temp2->name);
    metrics.data[i].units = strdup(temp2->units);
    if(temp2->style == MetStyleEventCounter)
      metrics.data[i].aggregate = SUM;
    else 
      metrics.data[i].aggregate = AVE;
    metrics.data[i].Id = (int)newEnabled[i]->met;
  }
  resources.data[0].Id = (int)newEnabled[0]->focus;

  // create a resource name
  y = newEnabled[0]->focus->convertToStringList();
  totalSize = 0;
  for(i=0;i<newEnabled[0]->focus->getCount();i++)
    totalSize += strlen(y[i]);
  if((resources.data[0].name = (char *)malloc(sizeof(char)*(totalSize +1)))
      == NULL){
    perror("malloc in VISIthreadchooseMetRes");
    return ;
  }
  where = 0;
  for(i=0;i<newEnabled[0]->focus->getCount();i++){
    if((strncpy(&(resources.data[0].name[where]),y[i],strlen(y[i])))==NULL){
      perror("strncpy in VISIthreadchooseMetRes");
      return;
    }
    where += strlen(y[i]);
  }

  binWidth = ptr->dmp->getCurrentBucketWidth(); 
  numBins = ptr->dmp->getMaxBins();

  ptr->visip->AddMetricsResources(metrics,resources,binWidth,numBins);

  free(metrics.data);
  free(resources.data);
  free(y);

}


void VISIthreadshowMsgREPLY(int userChoice){


}


void VISIthreadshowErrorREPLY(int userChoice){
  

}


///////   visualizationUser routines for visualization Upcalls
// check if metric and resource lists have wild card chars 
// if so request metrics and resources form UIM (currently, the only option)
// else make enable data collection call to DM for each metric resource pair
void visualizationUser::GetMetricResource(String metric,String resource,
					  int type){
 VISIthreadGlobals *ptr;

printf("in visualizationUser::GetMetricResource\n");
  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
    perror("thr_getspecific");
    return;
  }
printf("in visualizationUser::GetMetricResource before ptr->ump->chooseMetricsandResources\n");
  ptr->ump->chooseMetricsandResources((chooseMandRCBFunc)VISIthreadchooseMetRes);
printf("in visualizationUser::GetMetricResource after ptr->ump->chooseMetricsandResources\n");
}


void visualizationUser::StopMetricResource(int metricId,int resourceId){
 VISIthreadGlobals *ptr;
 metricInstance *listItem;
 int found = 0;

  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
    perror("thr_getspecific");
    return;
  }
  // search metricList for matching metricId and resourceId
  // if found request DM to disable data collection of metricInstance
  ptr->mrlist.setCurrent();
  while(((listItem = ptr->mrlist.getCurrent()) != 0) && !found){
    //if metric and resource match set found to 1 
    if((listItem->met==(metric *)metricId)&&(listItem->focus
        ==(resourceList *)resourceId)){
       found = 1;
    }
    ptr->mrlist.advanceCurrent();
  }
  if(found){
    //make disable request to DM and remove this metric instance from list
    ptr->dmp->disableDataCollection(ptr->perStream,listItem);
    if(!(ptr->mrlist.remove(listItem))){
      perror("ptr->mrlist.remove"); 
      return;
    }
  }
  // else ignore request
}

// not currently implemented
void visualizationUser::PhaseName(double begin,double end,String name){

 VISIthreadGlobals *ptr;

  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
    perror("thr_getspecific");
    return;
  }

}


///////  VISIthread main routine
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

  if((globals=(VISIthreadGlobals *)malloc(sizeof(VISIthreadGlobals))) == 0){
    perror("malloc");
    return 0;
  }


  vtp = new VISIthread(VMtid);
/*
  vtp = new VISIthread(thr_self());
  globals->ump = new UIMUser(UIMtid);
  globals->vmp = new VMUser(VMtid);
  globals->dmp = new dataManagerUser(DMtid);
*/
  globals->ump = uiMgr;
  globals->vmp = vmMgr;
  globals->dmp = dataMgr;

  globals->bufferSize = 0;
  globals->fd = -1;
  globals->pid = -1;
  globals->quit = 0;


  // start visualization process
  globals->fd = RPCprocessCreate(&globals->pid, "localhost", "",args->argv[0],args->argv);
  if (globals->fd < 0) {
    perror("process Create");
    exit(-1);
  }

  if(msg_bind(globals->fd,0) != THR_OKAY) {
    thr_perror("msg_bind(globals->fd)");
    return 0;
  }

  globals->visip = new visualizationUser(globals->fd,NULL,NULL); 

  if (thr_setspecific(visiThrd_key, globals) != THR_OKAY) {
    perror("thr_setspecific");
    return 0;
  }

  // set control callback routines 
  callbacks.mFunc = (metricInfoCallback)VISIthreadnewMetricCallback;
  callbacks.rFunc = (resourceInfoCallback)VISIthreadnewResourceCallback;
  callbacks.fFunc = (histFoldCallback)VISIthreadFoldCallback;

printf("before create performance stream in visithread");
  // create performance stream
  dataHandlers.sample = (sampleDataCallbackFunc)VISIthreadDataCallback;
  globals->perStream = globals->dmp->createPerformanceStream(context,
		   Sample,dataHandlers,callbacks);
printf("after create performance stream in visithread");
  // global synch.
 
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
    else if (died = RPC_readReady(globals->fd)) {
      globals->visip->awaitResponce(-1);
    }
    else if(died == -1){  // visualization process has died
       globals->quit = 1;
       // close visi file descriptor
    }
    else {
       vtp->mainLoop();
    }
    if(globals->vmp->isValidUpCall(tag)) {
      globals->vmp->awaitResponce(-1);
    }
    else {
      vtp->mainLoop();
    }
  }


  // disable all metricInstance data collection
  globals->mrlist.setCurrent();
  while((listItem = globals->mrlist.getCurrent()) != 0){
    globals->dmp->disableDataCollection(globals->perStream,listItem);
    if(!(globals->mrlist.remove(listItem))){
      perror("globals->mrlist.remove");
    }
    globals->mrlist.advanceCurrent();
  }

  // free all malloced space
  printf("leaving visithread main\n");
  thr_exit(0);
}
