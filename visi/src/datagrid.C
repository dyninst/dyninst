/* $Log: datagrid.C,v $
/* Revision 1.2  1994/03/14 20:28:44  newhall
/* changed visi subdirectory structure
/*  */ 
#include "visi/h/datagrid.h" 

Metric::Metric(char *metricUnits,char *metricName,int id,int foldMethod){
  units = new char[strlen(metricUnits) + 1];
  strcpy(units,metricUnits);
  name  = new char[strlen(metricName) + 1];
  strcpy(name,metricName);
  Id    = id;
  if(foldMethod == AVE)
    aggregate = foldMethod;
  else
    aggregate = SUM;
}

///////////////////////////////////////////
/*
 *  Resource constructor
 */
Resource::Resource(char *resourceName,int id){

  if(resourceName != NULL){
   name = new char[strlen(resourceName) + 1];
   strcpy(name,resourceName);
   Id = id;
  }
  else {
   name = new char[1];
    name[0] = '\0';
    Id = -1;
  }
}


///////////////////////////////////////////
visi_GridCellHisto::visi_GridCellHisto(int numElements){

 int i;
    
 if(numElements > 0){  
   value = new float[numElements];
   for(i = 0; i < numElements; i++)
     value[i] = ERROR;
   valid      = 1;
 }
 size       = numElements;
}

///////////////////////////////////////////
/*
 * constructor for class GridHistoArray
 */
visi_GridHistoArray::visi_GridHistoArray(int numElements){

 if(numElements > 0){  
   values = new visi_GridCellHisto[numElements];
 }
 size = numElements;

}


/*
 * destructor for class GridHistoArray
 */
visi_GridHistoArray::~visi_GridHistoArray(){

  delete[] values;
}

/*
 *
 */
int visi_GridHistoArray::Valid(int i){

  if ((i< 0) || (i>= size)){
    visi_ErrorHandler(ERROR_SUBSCRIPT,"visi_GridHistoArray::Valid");
    return(ERROR_SUBSCRIPT);  
  }
  return(values[i].Valid());

}


/*
 *
 */
int visi_GridHistoArray::Invalidate(int i){

  if ((i< 0) || (i>= size)){
    visi_ErrorHandler(ERROR_SUBSCRIPT,"visi_GridHistoArray::Invalidate");
    return(ERROR_SUBSCRIPT);  
  }
  values[i].Invalidate();
  return(OK);
}



///////////////////////////////////////////
/*
 * DataGrid constructor
 */
visi_DataGrid::visi_DataGrid(int noMetrics,int noResources,Metric *metricList,
	     Resource *resourceList,int noBins,float width){
int i;

  numMetrics   = noMetrics;
  numResources = noResources;
  metrics      = new Metric[noMetrics];
  resources    = new Resource[noResources];

  for(i = 0; i < noMetrics; i++){
    metrics[i].Metric(metricList[i].Units(),metricList[i].Name(),
		      metricList[i].Identifier(),metricList[i].Aggregate());
  }
  for(i = 0; i < noResources; i++){
    resources[i].Resource(resourceList[i].Name(),resourceList[i].Identifier());
  }

  data_values = new visi_GridHistoArray[noMetrics];
  for (i = 0; i < noMetrics; i++)
    data_values[i].visi_GridHistoArray(noResources);
  numBins  = noBins;
  binWidth = width;

}


/*
 * DataGrid constructor
 */
visi_DataGrid::visi_DataGrid(int noMetrics,int noResources,metricType *metricList,resourceType *resourceList,int noBins,float width){
int i;

  numMetrics   = noMetrics;
  numResources = noResources;
  metrics      = new Metric[noMetrics];
  resources    = new Resource[noResources];

  for(i = 0; i < noMetrics; i++){
    metrics[i].Metric(metricList[i].units,metricList[i].name,
		      metricList[i].Id,metricList[i].aggregate);
  }
  for(i = 0; i < noResources; i++){
    resources[i].Resource(resourceList[i].name,resourceList[i].Id);
  }
  data_values = new visi_GridHistoArray[noMetrics];
  for (i = 0; i < noMetrics; i++)
    data_values[i].visi_GridHistoArray(noResources);
  numBins  = noBins;
  binWidth = width;

}






/*
 *  DataGrid destructor 
 */
visi_DataGrid::~visi_DataGrid(){

  delete[] resources;
  delete[] metrics;
  delete[] data_values;
}

/* 
 * returns metric name for metric number i 
 */
char   *visi_DataGrid::MetricName(int i){
  if((i < numMetrics) && (i>=0))
    return(metrics[i].Name());
  return(NULL);
}

/* 
 * returns metric units for metric number i 
 */
char *visi_DataGrid::MetricUnits(int i){

  if((i < numMetrics) && (i>=0))
    return(metrics[i].Units());
  return(NULL);
}


/* 
 * returns resource name for resource number j 
 */
char     *visi_DataGrid::ResourceName(int j){

  if((j < numResources) && (j>0))
    return(resources[j].Name());
  return(NULL);
}

/* 
 * returns list of metrics for current visualization 
 */
char  *visi_DataGrid::MetricList(){

  return(NULL);
}

/* 
 * returns list of objects for current visualization 
 */
char  *visi_DataGrid::ObjectList(){

  return(NULL);
}

/* 
 *  returns fold method for metric i 
 */
int  visi_DataGrid::FoldMethod(int i){

  if((i < numMetrics) && (i >= 0))
    return(metrics[i].Aggregate());
  visi_ErrorHandler(ERROR_SUBSCRIPT,"visi_DataGrid::FoldMethod");
  return(ERROR_SUBSCRIPT);

}

/* 
 * returns metric identifier associated with metric number i 
 */
int  visi_DataGrid::MetricId(int i){

  if((i < numMetrics) && (i >= 0))
    return(metrics[i].Identifier());
  visi_ErrorHandler(ERROR_SUBSCRIPT,"visi_DataGrid::MetricId");
  return(ERROR_SUBSCRIPT);
}

/* 
 * returns resource identifier associated with resource number j 
 */
int  visi_DataGrid::ResourceId(int j){

  if((j < numResources) && (j >= 0))
    return(resources[j].Identifier());
  visi_ErrorHandler(ERROR_SUBSCRIPT,"visi_DataGrid::ResourceId");
  return(ERROR_SUBSCRIPT);
}

/*
 * returns 1 if datagrid element indicated by metric#, resource#
 * contains histogram values, otherwise returns false
 */
int visi_DataGrid::Valid(int metric,int resource){

  if((metric < 0) || (metric >= numMetrics)){
    visi_ErrorHandler(ERROR_SUBSCRIPT,"visi_HistoDataGrid::Valid");
    return(ERROR_SUBSCRIPT);
  }
  return(data_values[metric].Valid(resource));

}

/*
 * invalidates data_grid element indicated by metric#, resource#
 * sets valid to 0 and frees histogram space 
 */
int visi_DataGrid::Invalidate(int metric,int resource){

  if((metric < 0) || (metric >= numMetrics)){
    visi_ErrorHandler(ERROR_SUBSCRIPT,"visi_HistoDataGrid::Invalidate");
    return(ERROR_SUBSCRIPT);
  }
  return(data_values[metric].Invalidate(resource));

}

