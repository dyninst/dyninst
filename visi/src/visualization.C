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
/* $Log: visualization.C,v $
/* Revision 1.6  1994/05/11 17:13:14  newhall
/* changed data type from double to float
/*
 * Revision 1.5  1994/04/13  21:34:54  newhall
 * added routines: GetMetsRes StopMetRes NamePhase
 *
 * Revision 1.4  1994/03/26  04:19:49  newhall
 * changed all floats to double
 * fix problem with null string returned for first resource name
 *
 * Revision 1.3  1994/03/17  05:23:09  newhall
 * changed eventCallbacks type, and the constraints on triggering the
 * callback routine associated with the DATAVALUES event
 *
 * Revision 1.2  1994/03/14  20:28:55  newhall
 * changed visi subdirectory structure
 *  */ 
#include "visi/h/visualization.h"
#include "visi.SRVR.h"

visi_DataGrid  dataGrid;
visi_MRList    metricList;
visi_MRList    resourceList;
int            LastBucketSent = -1;
int fileDesc[FILETABLESIZE];
int (*fileDescCallbacks[FILETABLESIZE])();
int (*eventCallbacks[EVENTSIZE])(int);
int initDone = 0;

visualization *vp;
int visi_callback(){
  return(vp->mainLoop());
}

///////////////////////////////////////////////////////////
// paradyn initialization routine connects to parent socket,
// and registers the visualization::mainLoop routine as 
// callback on events on fileDesc[0]
///////////////////////////////////////////////////////////
int VisiInit(){

int i;

  for(i=0;i<FILETABLESIZE;i++){
    fileDescCallbacks[i] = NULL;
    fileDesc[i] = -1;
  }
  for(i=0;i<EVENTSIZE;i++){
    eventCallbacks[i] = NULL;
  }

  vp = new visualization(0, NULL, NULL);
  fileDesc[0] = 0;
  fileDescCallbacks[0] = visi_callback;
  initDone = 1;

  return(fileDesc[0]);
}

///////////////////////////////////////////////////////////
// makes initial call to get Metrics and Resources 
// for visualizations that do not provide an event that 
// invokes the GetMetsRes upcall, this routine should be
// called by the visiualizaiton before entering the mainloop
///////////////////////////////////////////////////////////
int StartVisi(int argc,
	      char *argv[]){

  if(!initDone)
    VisiInit();

  // call GetMetricResources with initial metric resource lists
  if(argc >= 3)
   vp->GetMetricResource(argv[1],argv[2],0);
  else
   vp->GetMetricResource(" "," ",0);
  return(OK);

}


///////////////////////////////////////////////////////////
// registration callback routine for paradyn events
// sets eventCallbacks[event] to callback routine provided by user
///////////////////////////////////////////////////////////
int RegistrationCallback(msgTag event,
			 int (*callBack)(int)){

  if((event >= 0) && (event < EVENTSIZE)){
    eventCallbacks[event] = callBack;
    return(OK);
  }
  else{
    visi_ErrorHandler(ERROR_SUBSCRIPT,"error in RegistrationCallback");
    return(ERROR_SUBSCRIPT);
  }
}

///////////////////////////////////////////////////////////
// fd registration and callback routine registration for user
// to register callback routines when they use the provided main routine
///////////////////////////////////////////////////////////
int RegFileDescriptors(int *fd, int (*callBack)()){
  return(OK);
}

///////////////////////////////////////////////////////////
// invokes upcall to paradyn VISIthread associated with the visualization
// takes list of current metrics, list of foci, and type of data
// (0 for histogram, 1 for scalar). 
// currently, only the NULL string, type 0 case is supported 
///////////////////////////////////////////////////////////
void GetMetsRes(char *metrics,
		char *resource,
		int type){

  vp->GetMetricResource(NULL,NULL,0);
}

///////////////////////////////////////////////////////////
// invokes upcall to paradyn.  Request to stop data for the 
// metric associated with metricId and resource associated with
// resourceId
///////////////////////////////////////////////////////////
void StopMetRes(int metricId,
		int resourceId){

  vp->StopMetricResource(metricId,resourceId);
}

///////////////////////////////////////////////////////////
// invokes upcall to paradyn.  Visualization sends phase
// definition to paradyn.  
///////////////////////////////////////////////////////////
void NamePhase(timeType begin,
	       timeType end,
	       char *name){

  vp->PhaseName((double)begin,(double)end,name);
}

///////////////////////////////////////////////////////////
//  Visi interface routine.  Receives an array of data 
//  values from paradyn, adds them to the datagrid, and
//  invokes the callback routine associated with the
//  DATAVALUES event.
///////////////////////////////////////////////////////////
void visualization::Data(dataValue_Array data){

int *metricIds, *resourceIds;
int noMetrics, noResources;
int i,j,metric,ok;
int temp,min,max;


  if(!initDone)
    VisiInit();

  noMetrics = dataGrid.NumMetrics();
  noResources = dataGrid.NumResources();

  if((metricIds = (int *)malloc(sizeof(int)*noMetrics)) == NULL){
      visi_ErrorHandler(ERROR_MALLOC,"error in malloc in visi::Data()");
  }
  if((resourceIds = (int *)malloc(sizeof(int)*noResources)) == NULL){
    visi_ErrorHandler(ERROR_MALLOC,"error in malloc in visi::Data()");
  }

  for(i=0; i < noMetrics; i++){
     metricIds[i] = dataGrid.MetricId(i);
  }

  for(i=0; i < noResources; i++){
    resourceIds[i] = dataGrid.ResourceId(i);
  }

  for(i=0; i < data.count; i++){

    // find metric and resource index into dataGrid and add value if found
    for(j=0;(j<noMetrics)&&(data.data[i].metricId!=metricIds[j]);j++) ;
    metric = j;
    for(j=0;(j<noResources)&&(data.data[i].resourceId!=resourceIds[j]);j++) ;

    if((j<noResources) && (metric < noMetrics)){
       dataGrid.AddValue(metric,j,
		         data.data[i].bucketNum,
		         (float)data.data[i].data);
    }
  } 

  min = max = dataGrid.NumBins()+1;
  for(i=0; i < noMetrics; i++){
    for(j=0; j < noResources; j++){
      if(dataGrid.Valid(i,j)){
        temp = dataGrid.LastBucketFilled(i,j);  
        if((temp >= -1) && (temp < min))
          min = temp; 
      }
    }
  }


  free(metricIds);
  free(resourceIds);

  //call user registered callback routine assoc. w/event DATAVALUES
  if((min > LastBucketSent) // if a new datagrid cross-section has been filled
     && (min != max)
     && (eventCallbacks[DATAVALUES] !=  NULL)){ // there is a callback routine 

     LastBucketSent = min;
     ok = eventCallbacks[DATAVALUES](LastBucketSent);
  }
}


///////////////////////////////////////////////////////////
//  Visi interface routine.  Receives notification of a
//  fold event, and the new bucket width.  Invokes
//  a fold operation on the datagrid
///////////////////////////////////////////////////////////
void visualization::Fold(double newBucketWidth){
  
  int ok;

  if(!initDone)
    VisiInit();

  dataGrid.Fold(newBucketWidth);
  // assume a fold can only occur when datagrid histogram buckets are full
  LastBucketSent = (dataGrid.NumBins()/2) - 1;

  //call user registered callback routine assoc. w/event FOLD
  if(eventCallbacks[FOLD] !=  NULL){
     ok = eventCallbacks[FOLD](0);
  }
}

///////////////////////////////////////////////////////////
// Visi interface routine.  Receives notification of an
// invalid metric/resource pair.  Invalidataes the datagrid
// cell associated with the metricId m and resourceId r.
///////////////////////////////////////////////////////////
void visualization::InvalidMR(int m, int r){

int i,j;
int ok;

  if(!initDone)
    VisiInit();

  // search for indices associated with metricId m and
  // resourceId r
  for(i=0;
     (i<dataGrid.NumMetrics())
     &&(m!=dataGrid.MetricId(i)); i++) ;
  for(j=0;
      (j<dataGrid.NumResources())
      &&(r!=dataGrid.ResourceId(j)); j++) ;

  ok = dataGrid.Invalidate(i,j);

  //call callback routine assoc. w/event INVALIDMETRICSRESOURCES 
  if(eventCallbacks[INVALIDMETRICSRESOURCES] != NULL){
     ok = eventCallbacks[INVALIDMETRICSRESOURCES](0);
  }
}

///////////////////////////////////////////////////////////
// Visi interface routine.  Receives a list of metrics and
// resources to add to the datagrid.
///////////////////////////////////////////////////////////
void visualization::AddMetricsResources(metricType_Array metrics,
					resourceType_Array resources,
					double bucketWidth,
					int nobuckets){
  int ok;

  if(!initDone)
    VisiInit();

  // if this is the first set of metrics/resources
  // construct new dataGrid
  if(!dataGrid.NumMetrics()){
    //construct metric, resource lists
    metricList.visi_MRList(metrics.count,
			   (visi_metricType *)metrics.data);
    resourceList.visi_MRList(resources.count,
			     (visi_resourceType *)resources.data);

    // construct new dataGrid 
    dataGrid.visi_DataGrid(metrics.count,
			resources.count,
			(visi_metricType *)metrics.data,
			(visi_resourceType *)resources.data,
			nobuckets,
			(timeType)bucketWidth);
  }
  else{
    // add elements to existing data grid
    // not supported yet
  }
  //call callback routine assoc. w/event ADDMETRICSRESOURCES 
  if(eventCallbacks[ADDMETRICSRESOURCES] !=  NULL){
     ok = eventCallbacks[ADDMETRICSRESOURCES](0);
  }
}

///////////////////////////////////////////////////////////
// Visi interface routine.  Visualization recieves notification
// of a new metrics and resources. 
///////////////////////////////////////////////////////////
void visualization::NewMetricsResources(metricType_Array metrics,
					resourceType_Array resources){
int ok; 


  if(!initDone)
    VisiInit();

  //call callback routine assoc. w/event NEWMETRICSRESOURCES
  if(eventCallbacks[NEWMETRICSRESOURCES] !=  NULL){
     ok = eventCallbacks[NEWMETRICSRESOURCES](0);
  }
}

///////////////////////////////////////////////////////////
// Visi interface routine.  Visualization recieves Phase
// information from Paradyn.
///////////////////////////////////////////////////////////
void visualization::Phase(double begin,
			  double end,
			  String name){

int size,ok;

  if(!initDone)
    VisiInit();
  size = strlen(name);
   
  //call callback routine assoc. w/event PHASENAME
  if(eventCallbacks[PHASENAME] !=  NULL){
     ok = eventCallbacks[PHASENAME](0);
  }
}

