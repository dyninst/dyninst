/* $Log: visualization.C,v $
/* Revision 1.4  1994/03/26 04:19:49  newhall
/* changed all floats to double
/* fix problem with null string returned for first resource name
/*
 * Revision 1.3  1994/03/17  05:23:09  newhall
 * changed eventCallbacks type, and the constraints on triggering the
 * callback routine associated with the DATAVALUES event
 *
 * Revision 1.2  1994/03/14  20:28:55  newhall
 * changed visi subdirectory structure
 *  */ 
#include "visi/h/visualization.h"
/* #define DEBUG */

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

// paradyn initialization routine connects to parent socket,
// and registers the visualization::mainLoop routine as callback
// on events on fileDesc[0], argv contains initial parameters to 
// visualization: metriclist, resourcelist 
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

int StartVisi(int argc,char *argv[]){

  if(!initDone)
    VisiInit();

  // call GetMetricResources with initial metric resource lists
  if(argc >= 3)
   vp->GetMetricResource(argv[1],argv[2],0);
  else
   vp->GetMetricResource(" "," ",0);
  return(OK);

}


// registration callback routine for paradyn events
// sets eventCallbacks[event] to callback routine provided by user
int RegistrationCallback(msgTag event,int (*callBack)(int)){
  if((event >= 0) && (event < EVENTSIZE)){
    eventCallbacks[event] = callBack;
    return(OK);
  }
  else{
    visi_ErrorHandler(ERROR_SUBSCRIPT,"error in RegistrationCallback");
    return(ERROR_SUBSCRIPT);
  }
}

// fd registration and callback routine registration for user
// to register callback routines when they use the provided main routine
int RegFileDescriptors(int *fd, int (*callBack)()){
  return(OK);
}



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
     dataGrid.AddValue(metric,j,data.data[i].bucketNum,data.data[i].data);
   }

  } 
  min = max = dataGrid.NumBins()+1;
  for(i=0; i < noMetrics; i++){
    for(j=0; j < noResources; j++){
      if(dataGrid.Valid(i,j)){
        temp = dataGrid.LastBucketFilled(i,j);  
        if((temp >= -1) && (temp < min))
          min = temp; 
#ifdef DEBUG
  fprintf(stderr,"@@@ in visualization::datagrid(%d,%d).LastBucketFilled =  %d\n",i,j,temp);
#endif
      }
    }
  }

#ifdef DEBUG
  fprintf(stderr,"@@@ in visualization::Data min = %d LastBucketSent = %d\n",min,LastBucketSent);
#endif

  free(metricIds);
  free(resourceIds);

  //call user registered callback routine assoc. w/event DATAVALUES
  if((min > LastBucketSent) && (min != max)&& (eventCallbacks[DATAVALUES] !=  NULL)){
#ifdef DEBUG
  fprintf(stderr,"@@@ before callback on event DATAVALUES in visualization::Data\n");
#endif
     LastBucketSent = min; 
     ok = eventCallbacks[DATAVALUES](LastBucketSent);
  }
}


void visualization::Fold(double newBucketWidth){
  
  int ok;

#ifdef DEBUG
  fprintf(stderr,"@@@ visualization::Fold LastBucketSent=%d\n",LastBucketSent);
#endif

  if(!initDone)
    VisiInit();
  dataGrid.Fold(newBucketWidth);
  // assume a fold can only occur when datagrid histogram buckets are full
  LastBucketSent = (dataGrid.NumBins()/2) - 1;

#ifdef DEBUG
  fprintf(stderr,"@@@ visualization::Fold LastBucketSent=%d\n",LastBucketSent);
#endif

  //call user registered callback routine assoc. w/event FOLD
  if(eventCallbacks[FOLD] !=  NULL){
     ok = eventCallbacks[FOLD](0);
  }
}

void visualization::InvalidMR(int m,int r){

int i,j;
int ok;

  if(!initDone)
    VisiInit();
for(i=0;(i<dataGrid.NumMetrics())&&(m!=dataGrid.MetricId(i));i++) ;
for(j=0;(j<dataGrid.NumResources())&&(r!=dataGrid.ResourceId(j));j++);
ok = dataGrid.Invalidate(i,j);

//call callback routine assoc. w/event INVALIDMETRICSRESOURCES 
if(eventCallbacks[INVALIDMETRICSRESOURCES] !=  NULL){
   ok = eventCallbacks[INVALIDMETRICSRESOURCES](0);
}
}

void visualization::AddMetricsResources(metricType_Array metrics,resourceType_Array resources,double bucketWidth,int nobuckets){
  int ok;
#ifdef DEBUG
  int i;
#endif

  if(!initDone)
    VisiInit();

  // construct new dataGrid
  if(!dataGrid.NumMetrics()){
    //construct metric, resource lists
    metricList.visi_MRList(metrics.count,metrics.data);
    resourceList.visi_MRList(resources.count,resources.data);

    // construct new dataGrid 
    dataGrid.visi_DataGrid(metrics.count,resources.count,metrics.data,resources.data,nobuckets,bucketWidth);
  }
  else{
    // add elements to existing data grid
    // not supported yet
#ifdef DEBUG
    fprintf(stderr,"@@@AddMetricsResources to existing datagrid is not supported");
#endif
  }
  //call callback routine assoc. w/event ADDMETRICSRESOURCES 
  if(eventCallbacks[ADDMETRICSRESOURCES] !=  NULL){
     ok = eventCallbacks[ADDMETRICSRESOURCES](0);
  }
}

void visualization::NewMetricsResources(metricType_Array metrics,resourceType_Array resources){
int ok; 


  if(!initDone)
    VisiInit();
  //call callback routine assoc. w/event NEWMETRICSRESOURCES
  if(eventCallbacks[NEWMETRICSRESOURCES] !=  NULL){
     ok = eventCallbacks[NEWMETRICSRESOURCES](0);
  }
}

void visualization::Phase(double begin,double end,String name){

int size,ok;

  if(!initDone)
    VisiInit();
   size = strlen(name);
   
  //call callback routine assoc. w/event PHASENAME
  if(eventCallbacks[PHASENAME] !=  NULL){
     ok = eventCallbacks[PHASENAME](0);
  }
}

