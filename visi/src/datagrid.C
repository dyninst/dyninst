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
/* Revision 1.21  1996/01/19 20:55:40  newhall
/* more chages to visiLib interface
/*
 * Revision 1.20  1996/01/17 18:29:13  newhall
 * reorginization of visiLib
 *
 * Revision 1.19  1995/12/18 23:22:03  newhall
 * changed metric units type so that it can have one of 3 values (normalized,
 * unnormalized or sampled)
 *
 * Revision 1.18  1995/11/17  17:28:37  newhall
 * added normalized member to Metric class which specifies units type
 * added MetricLabel, MetricAveLabel, and MetricSumLabel DG method functions
 *
 * Revision 1.17  1995/11/12  23:29:48  newhall
 * removed warnings, removed error.C
 *
 * Revision 1.16  1995/08/01  01:59:33  newhall
 * changes relating to phase interface stuff
 *
 * Revision 1.15  1995/06/02  21:02:04  newhall
 * changed type of metric and focus handles to u_int
 *
 * Revision 1.14  1995/02/26  01:59:37  newhall
 * added phase interface functions
 *
 * Revision 1.13  1994/11/02  04:14:57  newhall
 * memory leak fixes
 *
 * Revision 1.12  1994/08/11  02:52:09  newhall
 * removed calls to grid cell Deleted member functions
 *
 * Revision 1.11  1994/07/30  03:25:35  newhall
 * added enabled member to gridcell to indicate that the metric associated
 * w/ this cell has been enabled and data will arrive for it eventually
 * updated member functions affected by this addition
 *
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
#include "visi/src/datagridP.h" 

Metric::Metric(string metricUnits,
	       string metricName,
	       u_int id,
	       int foldMethod,
	       visi_unitsType units_type){

  units = metricUnits;
  name = metricName;
  Id    = id;
  if(foldMethod == AVE)
    aggregate = foldMethod;
  else
    aggregate = SUM;
  unitstype = units_type;
  if(unitstype == Normalized) {
    label = units;
    total_label = units;
    total_label += P_strdup("_seconds");
  }
  else if (unitstype == UnNormalized) {
    label = units;
    label += P_strdup("/sec");
    total_label = units; 
  }
  else {
    label = units;
    total_label = units; 
  }
}

//
//  Metric destructor
//
Metric::~Metric(){
  Id = 0;
}

///////////////////////////////////////////
//
//  Resource constructor
//
Resource::Resource(string resourceName,
		   u_int id){

  if(resourceName.string_of() != 0){
    name = resourceName; 
    Id = id;
  }
  else {
    name = 0;
    Id = 0;
  }
}

//
//  Resource destructor
//
Resource::~Resource(){
}

///////////////////////////////////////////
//
//  visi_GridCellHisto constructor
//
visi_GridCellHisto::visi_GridCellHisto(int numElements){

 int i;
    
 if(numElements > 0){  
   value = new visi_sampleType[numElements];
   for(i = 0; i < numElements; i++)
     value[i] = ERROR;
   valid      = 1;
 }
 enabled = 0;
 size       = numElements;
 lastBucketFilled = -1;
 firstValidBucket = -1;
}

//
// destructor for class visi_GridCellHisto
//
visi_GridCellHisto::~visi_GridCellHisto(){

  if(value) delete[] value;
  value = 0;
  valid = 0;
  enabled = 0;
  size = 0;
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
    return(ERROR_INT);  
  }
  return(values[i].Valid());

}


//
// invalidates the grid cell indexed by i 
//
int visi_GridHistoArray::Invalidate(int i){

  if ((i< 0) || (i>= size)){
    return(ERROR_INT);  
  }
  values[i].Invalidate();
  return(OK);
}


//
// add new elements to the values array
//
int visi_GridHistoArray::AddNewResources(int howmany){

visi_GridCellHisto *temp = 0;

  if(howmany > 0){
    temp = values;
    values = new visi_GridCellHisto[howmany + size];
    for(int i = 0; i < size; i++){
       if(values[i].AddNewValues(temp[i].Value(),
				  temp[i].Size(),
				  temp[i].LastBucketFilled(),
				  temp[i].userdata,
				  temp[i].Valid(),
				  temp[i].Enabled()) != OK){
	 return(ERROR_INT);
       }
       temp[i].userdata = 0;
    }
    size += howmany;
    
  }
  delete[] temp;
  return(OK);

}


//
//  add new elements to the values array
//
int visi_GridHistoArray::AddNewValues(visi_GridCellHisto *rarray,int howmany){

  values = rarray;
  size   = howmany;
  rarray = 0;
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
			     visi_timeType width,
			     visi_timeType startTime,
			     int phaseHandle){
int i;

  numMetrics   = noMetrics;
  numResources = noResources;
  metrics      = new Metric[noMetrics];
  resources    = new Resource[noResources];

  for(i = 0; i < noMetrics; i++){
    metrics[i].Metric(metricList[i].Units(),metricList[i].Name(),
		      metricList[i].Identifier(),metricList[i].Aggregate(),
		      metricList[i].UnitsType());
  }
  for(i = 0; i < noResources; i++){
    resources[i].Resource(resourceList[i].Name(),resourceList[i].Identifier());
  }

  data_values = new visi_GridHistoArray[noMetrics];
  for (i = 0; i < noMetrics; i++)
    data_values[i].visi_GridHistoArray(noResources);
  numBins  = noBins;
  binWidth = width;
  start_time = startTime;
  phase_handle = phaseHandle;

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
			     visi_timeType width,
			     visi_timeType startTime,
			     int phaseHandle){
int i;

  numMetrics   = noMetrics;
  numResources = noResources;
  metrics      = new Metric[noMetrics];
  resources    = new Resource[noResources];

  for(i = 0; i < noMetrics; i++){
    metrics[i].Metric(metricList[i].units,metricList[i].name,
		      metricList[i].Id,metricList[i].aggregate,
		      metricList[i].unitstype);
  }
  for(i = 0; i < noResources; i++){
    resources[i].Resource(resourceList[i].name,resourceList[i].Id);
  }
  data_values = new visi_GridHistoArray[noMetrics];
  for (i = 0; i < noMetrics; i++)
    data_values[i].visi_GridHistoArray(noResources);
  numBins  = noBins;
  binWidth = width;
  start_time = startTime;
  phase_handle = phaseHandle;

}



//
//  DataGrid destructor 
//
visi_DataGrid::~visi_DataGrid(){

  delete[] resources;
  delete[] metrics;
  delete[] data_values;
}

const char *visi_DataGrid::GetMyPhaseName(){

    if (phase_handle == -1) return ("Global");
    if (phase_handle >= 0){
        for(unsigned i = 0; i < phases.size(); i++){
	    if(phase_handle == (int)phases[i]->getPhaseHandle()){
                return phases[i]->getName();
    }}}
    return 0;
}

// 
// returns metric name for metric number i 
//
const char   *visi_DataGrid::MetricName(int i){
  if((i < numMetrics) && (i>=0))
    return(metrics[i].Name());
  return(0);
}

// 
// returns metric units for metric number i 
//
const char *visi_DataGrid::MetricUnits(int i){

  if((i < numMetrics) && (i>=0))
    return(metrics[i].Units());
  return(0);
}

// 
// returns metric label for data values 
//
const char *visi_DataGrid::MetricLabel(int i){

  if((i < numMetrics) && (i>=0)){
    return(metrics[i].Label());
  }
  return(0);
}

// 
// returns metric label for AVE aggregation over data buckets 
//
const char *visi_DataGrid::MetricAveLabel(int i){

  if((i < numMetrics) && (i>=0)) {
    return(metrics[i].AveLabel());
  }
  return(0);
}

// 
// returns metric label for SUM aggregation over data buckets 
//
const char *visi_DataGrid::MetricSumLabel(int i){

  if((i < numMetrics) && (i>=0)) {
    return(metrics[i].SumLabel());
  }
  return(0);
}

// 
// returns resource name for resource number j 
//
const char     *visi_DataGrid::ResourceName(int j){

  if((j < numResources) && (j>=0))
    return(resources[j].Name());
  return(0);
}


// 
//  returns fold method for metric i 
//
int  visi_DataGrid::FoldMethod(int i){

  if((i < numMetrics) && (i >= 0))
    return(metrics[i].Aggregate());
  return(ERROR_INT);

}

// 
// returns metric identifier associated with metric number i 
//
u_int  visi_DataGrid::MetricId(int i,bool &error){

  error = true;
  if((i < numMetrics) && (i >= 0)){
    error = false;
    return (metrics[i].Identifier());
  }
  return(0);
}

// 
// returns resource identifier associated with resource number j 
//
u_int  visi_DataGrid::ResourceId(int j, bool &error){

  error = true;
  if((j < numResources) && (j >= 0)){
    error = false;
    return(resources[j].Identifier());
  }
  return(0);
}

//
// returns 1 if datagrid element indicated by metric#, resource#
// contains histogram values, otherwise returns false
//
int visi_DataGrid::Valid(int metric, 
			 int resource){

  if((metric < 0) || (metric >= numMetrics)){
    return(ERROR_INT);
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
    return(ERROR_INT);
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
      if((ok = data_values[i].AddNewResources(howmany)) != OK){
          delete[] temp;
          temp = 0;
          return(ok); 
      }
  }

  delete[] temp;
  temp = 0;
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
		      temp[i].Identifier(),temp[i].Aggregate(),
		      temp[i].UnitsType());
  }
  for(i = numMetrics; i < (numMetrics + howmany); i++){
    metrics[i].Metric(mlist[i-numMetrics].units, mlist[i-numMetrics].name,
		       mlist[i-numMetrics].Id, mlist[i-numMetrics].aggregate,
		       mlist[i-numMetrics].unitstype);
  }


  // add space to data grid for new metrics

  tempdata = data_values;
  data_values = new visi_GridHistoArray[numMetrics + howmany];

  for(i=0; i < numMetrics; i++){
    if(data_values[i].AddNewValues(tempdata[i].Value(),tempdata[i].Size())
       != OK){
       return(ERROR_INT); 
    }
  }

  for(i=numMetrics; i < (numMetrics + howmany); i++){
    data_values[i].visi_GridHistoArray(numResources);
  }

  numMetrics += howmany;
  tempdata = 0;
  temp = 0;
  return(OK);

}


//
//  returns 1 if metric with Id equal to test_id is in the data grid
//
int visi_DataGrid::MetricInGrid(u_int test_id){

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
int visi_DataGrid::ResourceInGrid(u_int test_id){

  for(int i = 0; i < numResources; i++){
    if (test_id == resources[i].Identifier()){
      return(1);
    }
  }
  return(0);
}

int phaseCompare(const void *p1, const void *p2) {
   const PhaseInfo *ph1 = *((const PhaseInfo **)p1);
   const PhaseInfo *ph2 = *((const PhaseInfo **)p2);
   return(ph1->getPhaseHandle() - ph2->getPhaseHandle());
}

void visi_DataGrid::AddNewPhase(int handle, visi_timeType start, visi_timeType end,
		      visi_timeType width, string name){
    PhaseInfo *p = new PhaseInfo(handle,start,end,width,name.string_of());
    phases += p;
    phases.sort(phaseCompare);

}
								       
