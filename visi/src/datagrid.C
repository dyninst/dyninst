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
/* $Log: datagrid.C,v $
/* Revision 1.11  1994/07/30 03:25:35  newhall
/* added enabled member to gridcell to indicate that the metric associated
/* w/ this cell has been enabled and data will arrive for it eventually
/* updated member functions affected by this addition
/*
 * Revision 1.10  1994/07/20  22:17:50  newhall
 * added FirstValidBucket method function to visi_GridCellHisto class
 *
 * Revision 1.9  1994/07/07  22:40:30  newhall
 * fixed compile warnings
 *
 * Revision 1.8  1994/06/16  18:24:50  newhall
 * fix to visualization::Data
 *
 * Revision 1.7  1994/06/07  17:48:46  newhall
 * support for adding metrics and resources to existing visualization
 *
 * Revision 1.6  1994/05/23  20:56:46  newhall
 * To visi_GridCellHisto class: added deleted flag, SumValue
 * method function, and fixed AggregateValue method function
 *
 * Revision 1.5  1994/05/11  17:12:44  newhall
 * changed data values type from double to float
 * fixed fold method function to support a folding
 * at any point in the histogram, rather than only
 * when the histogram is full
 *
 * Revision 1.4  1994/03/26  04:19:46  newhall
 * changed all floats to double
 * fix problem with null string returned for first resource name
 *
 * Revision 1.3  1994/03/17  05:19:59  newhall
 * changed bucket width and time value's type to double
 *
 * Revision 1.2  1994/03/14  20:28:44  newhall
 * changed visi subdirectory structure
 *  */ 
///////////////////////////////////////////////
// Member functions for the following classes:
//  Metric, Resource, visi_GridCellHisto,
//  visi_GridHistoArray, visi_DataGrid
///////////////////////////////////////////////
#include "visi/h/datagrid.h" 

Metric::Metric(char *metricUnits,
	       char *metricName,
	       int id,
	       int foldMethod){

  int len;

  if(metricUnits != NULL){
    len = strlen(metricUnits);
    units = new char[len + 1];
    strcpy(units,metricUnits);
    units[len] = '\0';
  }
  else{
   units = NULL;
  }
    if(metricName != NULL){
    len = strlen(metricName);
    name  = new char[len + 1];
    strcpy(name,metricName);
    name[len] = '\0';
  }
  else{
    name = NULL;
  }
  Id    = id;
  if(foldMethod == AVE)
    aggregate = foldMethod;
  else
    aggregate = SUM;
}

///////////////////////////////////////////
//
//  Resource constructor
//
Resource::Resource(char *resourceName,
		   int id){
  int len;

  if(resourceName != NULL){
    len = strlen(resourceName);
    name = new char[len+1];
    strcpy(name,resourceName);
    name[len] = '\0';
    Id = id;
  }
  else {
    name = new char[1];
    name[0] = '\0';
    Id = -1;
  }
}


///////////////////////////////////////////
//
//  visi_GridCellHisto constructor
//
visi_GridCellHisto::visi_GridCellHisto(int numElements){

 int i;
    
 if(numElements > 0){  
   value = new sampleType[numElements];
   for(i = 0; i < numElements; i++)
     value[i] = ERROR;
   valid      = 1;
 }
 deleted = 0;
 enabled = 0;
 size       = numElements;
 lastBucketFilled = -1;
 firstValidBucket = -1;
}

///////////////////////////////////////////
//
// constructor for class GridHistoArray
//
visi_GridHistoArray::visi_GridHistoArray(int numElements){

 if(numElements > 0){  
   values = new visi_GridCellHisto[numElements];
 }
 size = numElements;

}


//
// destructor for class GridHistoArray
//
visi_GridHistoArray::~visi_GridHistoArray(){

  delete[] values;
}

//
// evaluates to true if the grid cell indexed by i (foucus index)
// contains a histogram (is a valid metric/focus pair)
//
int visi_GridHistoArray::Valid(int i){

  if ((i< 0) || (i>= size)){
    visi_ErrorHandler(ERROR_SUBSCRIPT,"visi_GridHistoArray::Valid");
    return(ERROR_SUBSCRIPT);  
  }
  return(values[i].Valid());

}


//
// invalidates the grid cell indexed by i 
//
int visi_GridHistoArray::Invalidate(int i){

  if ((i< 0) || (i>= size)){
    visi_ErrorHandler(ERROR_SUBSCRIPT,"visi_GridHistoArray::Invalidate");
    return(ERROR_SUBSCRIPT);  
  }
  values[i].Invalidate();
  return(OK);
}


//
// add new elements to the values array
//
int visi_GridHistoArray::AddNewResources(int howmany){

visi_GridCellHisto *temp;

  if(howmany > 0){
    temp = values;
    values = new visi_GridCellHisto[howmany + size];
    for(int i = 0; i < size; i++){
       if(values[i].AddNewValues(temp[i].Value(),
				  temp[i].Size(),
				  temp[i].LastBucketFilled(),
				  temp[i].userdata,
				  temp[i].Valid(),
				  temp[i].Deleted(),
				  temp[i].Enabled()) != OK){
	 return(ERROR_CREATEGRID);
       }
       temp[i].userdata = NULL;
    }
    size += howmany;
    
  }
  return(OK);

}


//
//  add new elements to the values array
//
int visi_GridHistoArray::AddNewValues(visi_GridCellHisto *rarray,int howmany){

  values = rarray;
  size   = howmany;
  rarray = NULL;
  return(OK);

}

///////////////////////////////////////////
//
// DataGrid constructor: creates metric and 
// resource lists and empty datagrid
//
visi_DataGrid::visi_DataGrid(int noMetrics,
			     int noResources,
			     Metric *metricList,
	     		     Resource *resourceList,
			     int noBins,
			     timeType width){
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


//
// DataGrid constructor: creates metric and 
// resource lists and empty datagrid
//
visi_DataGrid::visi_DataGrid(int noMetrics,
			     int noResources,
			     visi_metricType *metricList,
			     visi_resourceType *resourceList,
			     int noBins,
			     timeType width){
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



//
//  DataGrid destructor 
//
visi_DataGrid::~visi_DataGrid(){

  delete[] resources;
  delete[] metrics;
  delete[] data_values;
}

// 
// returns metric name for metric number i 
//
char   *visi_DataGrid::MetricName(int i){
  if((i < numMetrics) && (i>=0))
    return(metrics[i].Name());
  return(NULL);
}

// 
// returns metric units for metric number i 
//
char *visi_DataGrid::MetricUnits(int i){

  if((i < numMetrics) && (i>=0))
    return(metrics[i].Units());
  return(NULL);
}


// 
// returns resource name for resource number j 
//
char     *visi_DataGrid::ResourceName(int j){

  if((j < numResources) && (j>=0))
    return(resources[j].Name());
  return(NULL);
}


// 
//  returns fold method for metric i 
//
int  visi_DataGrid::FoldMethod(int i){

  if((i < numMetrics) && (i >= 0))
    return(metrics[i].Aggregate());
  visi_ErrorHandler(ERROR_SUBSCRIPT,"visi_DataGrid::FoldMethod");
  return(ERROR_SUBSCRIPT);

}

// 
// returns metric identifier associated with metric number i 
//
int  visi_DataGrid::MetricId(int i){

  if((i < numMetrics) && (i >= 0))
    return(metrics[i].Identifier());
  visi_ErrorHandler(ERROR_SUBSCRIPT,"visi_DataGrid::MetricId");
  return(ERROR_SUBSCRIPT);
}

// 
// returns resource identifier associated with resource number j 
//
int  visi_DataGrid::ResourceId(int j){

  if((j < numResources) && (j >= 0))
    return(resources[j].Identifier());
  visi_ErrorHandler(ERROR_SUBSCRIPT,"visi_DataGrid::ResourceId");
  return(ERROR_SUBSCRIPT);
}

//
// returns 1 if datagrid element indicated by metric#, resource#
// contains histogram values, otherwise returns false
//
int visi_DataGrid::Valid(int metric, 
			 int resource){

  if((metric < 0) || (metric >= numMetrics)){
    visi_ErrorHandler(ERROR_SUBSCRIPT,"visi_HistoDataGrid::Valid");
    return(ERROR_SUBSCRIPT);
  }
  return(data_values[metric].Valid(resource));

}

//
// invalidates data_grid element indicated by metric#, resource#
// sets valid to 0 and frees histogram space 
//
int visi_DataGrid::Invalidate(int metric,
			      int resource){

  if((metric < 0) || (metric >= numMetrics)){
    visi_ErrorHandler(ERROR_SUBSCRIPT,"visi_HistoDataGrid::Invalidate");
    return(ERROR_SUBSCRIPT);
  }
  return(data_values[metric].Invalidate(resource));

}


//
// adds a new set of resources to the data grid
//
int visi_DataGrid::AddNewResource(int howmany,visi_resourceType *rlist){

Resource *temp;
int i,ok;

  // add new values to resource list
  temp = resources;
  resources = new Resource[numResources + howmany];

  for(i = 0; i < numResources; i++){
    resources[i].Resource(temp[i].Name(),temp[i].Identifier());
  }
  for(i = numResources; i < (numResources + howmany); i++){
    resources[i].Resource(rlist[i-numResources].name,
			  rlist[i-numResources].Id);
  }

  numResources += howmany;

  // add space to data grid for new resources
  for(i = 0; i < numMetrics; i++){
     if((ok = data_values[i].AddNewResources(howmany)) != OK)
       return(ok); 
  }
  return(OK);
}


//
//  adds a new set of resources to the data grid
//
int visi_DataGrid::AddNewMetrics(int howmany,visi_metricType *mlist){

visi_GridHistoArray *tempdata;
Metric *temp;
int i;

  // add new values to metric list
  temp = metrics;
  metrics = new Metric[numMetrics + howmany];

  for(i = 0; i < numMetrics; i++){
    metrics[i].Metric(temp[i].Units(),temp[i].Name(),
		      temp[i].Identifier(),temp[i].Aggregate());
  }
  for(i = numMetrics; i < (numMetrics + howmany); i++){
    metrics[i].Metric(mlist[i-numMetrics].units, mlist[i-numMetrics].name,
		       mlist[i-numMetrics].Id, mlist[i-numMetrics].aggregate);
  }


  // add space to data grid for new metrics

  tempdata = data_values;
  data_values = new visi_GridHistoArray[numMetrics + howmany];

  for(i=0; i < numMetrics; i++){
    if(data_values[i].AddNewValues(tempdata[i].Value(),tempdata[i].Size())
       != OK){
       return(ERROR_CREATEGRID); 
    }
  }

  for(i=numMetrics; i < (numMetrics + howmany); i++){
    data_values[i].visi_GridHistoArray(numResources);
  }

  numMetrics += howmany;
  tempdata = NULL;
  return(OK);

}


//
//  returns 1 if metric with Id equal to test_id is in the data grid
//
int visi_DataGrid::MetricInGrid(int test_id){

  for(int i = 0; i < numMetrics; i++){
    if (test_id == metrics[i].Identifier()){
      return(1);
    }
  }
  return(0);
}


//
//  returns 1 if resource with Id equal to test_id is in the data grid
//
int visi_DataGrid::ResourceInGrid(int test_id){

  for(int i = 0; i < numResources; i++){
    if (test_id == resources[i].Identifier()){
      return(1);
    }
  }
  return(0);
}
