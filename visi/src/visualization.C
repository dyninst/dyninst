/* $Log: visualization.C,v $
/* Revision 1.2  1994/03/14 20:28:55  newhall
/* changed visi subdirectory structure
/*  */ 
#include "visi/h/visualization.h"

visi_DataGrid  dataGrid;
visi_MRList    metricList;
visi_MRList    resourceList;
int fileDesc[FILETABLESIZE];
int (*fileDescCallbacks[FILETABLESIZE])();
int (*eventCallbacks[EVENTSIZE])();
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
int RegistrationCallback(msgTag event,int (*callBack)()){
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

  } // for

  free(metricIds);
  free(resourceIds);
  //call user registered callback routine assoc. w/event DATAVALUES
  if(eventCallbacks[DATAVALUES] !=  NULL){
     ok = eventCallbacks[DATAVALUES]();
  }
}


void visualization::Fold(float newBucketWidth){
  
  int ok;

  if(!initDone)
    VisiInit();
  dataGrid.Fold(newBucketWidth);

  //call user registered callback routine assoc. w/event FOLD
  if(eventCallbacks[FOLD] !=  NULL){
     ok = eventCallbacks[FOLD]();
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
   ok = eventCallbacks[INVALIDMETRICSRESOURCES]();
}
}

void visualization::AddMetricsResources(metricType_Array metrics,resourceType_Array resources,float bucketWidth,int nobuckets){
int ok;

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
    fprintf(stderr,"AddMetricsResources to existing datagrid is not supported");
  }
  //call callback routine assoc. w/event ADDMETRICSRESOURCES 
  if(eventCallbacks[ADDMETRICSRESOURCES] !=  NULL){
     ok = eventCallbacks[ADDMETRICSRESOURCES]();
  }
}

void visualization::NewMetricsResources(metricType_Array metrics,resourceType_Array resources){
int ok; 


  if(!initDone)
    VisiInit();
  //call callback routine assoc. w/event NEWMETRICSRESOURCES
  if(eventCallbacks[NEWMETRICSRESOURCES] !=  NULL){
     ok = eventCallbacks[NEWMETRICSRESOURCES]();
  }
}

void visualization::Phase(float begin,float end,String name){

int size,ok;

  if(!initDone)
    VisiInit();
   size = strlen(name);
   
  //call callback routine assoc. w/event PHASENAME
  if(eventCallbacks[PHASENAME] !=  NULL){
     ok = eventCallbacks[PHASENAME]();
  }
}

