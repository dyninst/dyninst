/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: datagrid.C,v 1.33 2004/03/23 01:12:46 eli Exp $

///////////////////////////////////////////////
// Member functions for the following classes:
//  Metric, Resource, visi_GridCellHisto,
//  visi_GridHistoArray, visi_DataGrid
///////////////////////////////////////////////
#include "visi/src/datagridP.h" 

Metric::Metric(pdstring metric_currUnits,
               pdstring metric_totUnits,
               pdstring metricName,
               u_int id,
               int foldMethod,
               visi_unitsType units_type){

  curr_units = metric_currUnits;
  tot_units = metric_totUnits;
  name = metricName;
  Id    = id;
  
  if(foldMethod == AVE)
    aggregate = foldMethod;
  else
    aggregate = SUM;
  unitstype = units_type;
  
  if( metric_totUnits != "" ) { // both units specifed, use as labels
    label = metric_currUnits; 
    total_label = metric_totUnits; 
  }
  else { // set labels based upon normalized flag
    if(unitstype == Normalized) {
      label = curr_units;
      total_label = curr_units;
      total_label += P_strdup("_seconds");
    }
    else if (unitstype == UnNormalized) {
      label = curr_units;
      label += P_strdup("/sec");
      total_label = curr_units; 
    }
    else {
      label = curr_units;
      total_label = curr_units; 
    }
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
Resource::Resource(pdstring resourceName,
                   u_int id){

  if(resourceName.c_str() != 0){
    name = resourceName; 
    Id = id;
  }
  else {
    name = "";
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
     value[i] = VISI_ERROR;
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

// returns a normalized value for EventCounter metrics which is (currently)
// what is stored in each bucket for these style of metrics returns the
// actual value for the SampledFunction metrics, which it does by summing
// across all the previous buckets and adding the initial actual value of the
// metric.  the implementation needs to be changed since the format of the
// histogram has been changed so that the change in sample value (instead of
// the actual value) is stored for each bucket for SampledFunction metrics
visi_sampleType visi_GridCellHisto::Value(int bkt, visi_unitsType unitstype) { 
  visi_sampleType retSample = 0;

  if((bkt < 0) || (bkt >= size)){
    retSample = VISI_ERROR;
  } else if(unitstype == Sampled) {
    if(isnan(value[bkt]))
      retSample = VISI_ERROR;
    else if(initActVal == -1) {
      retSample = VISI_ERROR;      
    } else {
      retSample = initActVal;
      for(int i=0; i<=bkt; i++) {
	if(!isnan(value[i])){
	  retSample += value[i];
	}
      }
    } 
  } else {
    retSample = value[bkt];
  }

  return retSample;
}

int visi_GridCellHisto::AddNewValues(visi_sampleType *temp, int arraySize,
				     int lbf, void *ud, int v, int e,
				     visi_sampleType initActVal) {
  if(temp == NULL){
    value = NULL;
    size = 0;
  }
  else{
    // initialize cell to temp values
    value = new visi_sampleType[arraySize];
    size = arraySize;
    for(int i=0;i<size;i++){
      if(!isnan(temp[i])){
	value[i] = temp[i]; 
	if(firstValidBucket == -1) {
	  firstValidBucket = i;
	}
      }
      else
	value[i] = VISI_ERROR; 
    }
  }
  lastBucketFilled = lbf;
  userdata = ud;
  valid = v;
  enabled = e;
  SetInitialActualValue(initActVal);
  return(VISI_OK);
}

int visi_GridCellHisto::AddValue(visi_sampleType x, int i, int numElements) {
  int j;
  
  if (!enabled){ // if this cell has not been enabled don't add values
    return(VISI_OK);
  }
  if (!valid){ // if this is the first value create a histo cell array 
    if(value == NULL)
      value = new visi_sampleType[numElements];
    size = numElements;
    valid = 1;
    enabled = 1;
    for(j=0;j<size;j++){
      value[j] = VISI_ERROR;
    }
  }
  if((i < 0) || (i >= size))
    return(VISI_ERROR_INT);
  value[i] = x;
  if(i > lastBucketFilled) {
    lastBucketFilled = i;
  }
  if(firstValidBucket == -1) {
    firstValidBucket = i;
  }
  return(VISI_OK);
}

void visi_GridCellHisto::Fold(visi_unitsType unitstype) {
  int i,j;
  if(valid){
    firstValidBucket = -1;
    for(i=0,j=0;(i< (lastBucketFilled+1)/2) // new bucket counter
	  && (j< (lastBucketFilled+1)); // old bucket counter
	i++,j+=2){
      // For the sampledFunction type of metrics, the buckets hold the change
      // in the sample value, so folding buckets with these delta sample
      // values involves just summing the buckets together.
      // For the eventCounter type of metrics (eg. timing style of metrics),
      // the buckets are normalized, so we need to take the average
      // of the buckets when folding.
      // These two metric types will be unified when the visis are converted
      // over.
      if((!isnan(value[j])) && (!isnan(value[j+1]))){
	if(unitstype == Sampled)
	  value[i] = value[j] + value[j+1];
	else //unitstype == EventCounter
	  value[i] = (value[j] + value[j+1]) / 2.0f;

	if(firstValidBucket == -1) {
	  firstValidBucket = i;
	}
      } else if(!isnan(value[j])) {
	if(unitstype == Sampled)
	  value[i] = value[j];
	else
	  value[i] = value[j] / 2.0f;

	if(firstValidBucket == -1) {
	  firstValidBucket = i;
	}
      } else if(!isnan(value[j+1])) {
	if(unitstype == Sampled)
	  value[i] = value[j+1];
	else
	  value[i] = value[j+1] / 2.0f;

	if(firstValidBucket == -1) {
	  firstValidBucket = i;
	}
      }
      else{
	value[i] = VISI_ERROR;
      }
    }
    for(i=(lastBucketFilled+1)/2; i < size; i++){
      value[i] = VISI_ERROR;
    }

    lastBucketFilled = ((lastBucketFilled+1)/2)-1;
  }
}

void visi_GridCellHisto::Value(visi_sampleType *samples, 
			       int firstBucket, int lastBucket,
			       visi_unitsType unitstype) 
{ 
  sampleVal_cerr << "Value2- unitstype: " 
		 <<(unitstype==Sampled?"Sampled":"NotSampled")
		 << ", firstBucket: " << firstBucket << ", lastBucket: "
		 << lastBucket << "\n";
  for(int i=firstBucket; i<=lastBucket; i++) {
    if(unitstype == Sampled)
      samples[i] = Value(i, unitstype);
    else
      samples[i] = value[i];
  }
}

visi_sampleType visi_GridCellHisto::SumValue(visi_timeType width,
					     visi_unitsType unitstype) {
  int i;
  visi_sampleType sum;
  
  if(value != NULL){
    for(sum=0.0,i=0; i< size; i++){
      if(!isnan(value[i])){
	sum += Value(i, unitstype);
      }
    }
    sampleVal_cerr << "  numBuckets: " << size << "  sum: " << sum 
		   << "  width: " << width << "  sum*width: " 
		   << sum*width << "\n"; 
    if(unitstype == Sampled)
      return sum;
    else
      return sum*width;
  }
  else{
    sampleVal_cerr << " value == NULL\n";
    return(VISI_ERROR);
  }
}

visi_sampleType visi_GridCellHisto::AggregateValue(visi_unitsType unitstype) {
  int i,num;
  visi_sampleType sum;
  if(value != NULL){
    for(sum=0.0,num=i=0; i< size; i++){
      if(!isnan(value[i])){
	sum += Value(i, unitstype);
	num++;
      }
    }
    
    if(num != 0){
      sampleVal_cerr << "  sum: " << sum << "  num: " << num 
		     << "  sum/num: " << (sum/(1.0*num)) << "\n";
      return(sum/(1.0f * num));
    }
    else{
      return(VISI_ERROR);
    }
  }
  else{
    return(VISI_ERROR);
  }
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
    return(VISI_ERROR_INT);  
  }
  return(values[i].Valid());

}


//
// invalidates the grid cell indexed by i 
//
int visi_GridHistoArray::Invalidate(int i){

  if ((i< 0) || (i>= size)){
    return(VISI_ERROR_INT);  
  }
  values[i].Invalidate();
  return(VISI_OK);
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
       if(values[i].AddNewValues(temp[i].GetValueRawData(),
				 temp[i].Size(),
				 temp[i].LastBucketFilled(),
				 temp[i].userdata,
				 temp[i].Valid(),
				 temp[i].Enabled(),
				 temp[i].GetInitialActualValue()) != VISI_OK)
	 {  return(VISI_ERROR_INT);  }
       temp[i].userdata = 0;
    }
    size += howmany;
    
  }
  delete[] temp;
  return(VISI_OK);

}


//
//  add new elements to the values array
//
int visi_GridHistoArray::AddNewValues(visi_GridCellHisto *rarray,int howmany){

  values = rarray;
  size   = howmany;
  rarray = 0;
  return(VISI_OK);

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
    metrics[i] = Metric(metricList[i].CurrUnits(),metricList[i].TotUnits(),
			metricList[i].Name(),metricList[i].Identifier(),
			metricList[i].Aggregate(),metricList[i].UnitsType());
  }
  for(i = 0; i < noResources; i++){
    resources[i] = Resource(resourceList[i].Name(),resourceList[i].Identifier());
  }

  data_values = new visi_GridHistoArray[noMetrics];
  for (i = 0; i < noMetrics; i++){
      visi_GridHistoArray *temp = new visi_GridHistoArray(noResources);
      data_values[i] = *temp;
      delete temp;
  }
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
    metrics[i] = Metric(metricList[i].curr_units,metricList[i].tot_units,
			metricList[i].name,metricList[i].Id,
			metricList[i].aggregate,metricList[i].unitstype);
  }
  for(i = 0; i < noResources; i++){
    resources[i] = Resource(resourceList[i].name,resourceList[i].Id);
  }
  data_values = new visi_GridHistoArray[noMetrics];
  for (i = 0; i < noMetrics; i++){
      visi_GridHistoArray *temp = new visi_GridHistoArray(noResources);
      data_values[i] = *temp;
      delete temp;
  }
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
        for(u_int i = 0; i < phases.size(); i++){
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
    return(metrics[i].CurrUnits());
  return(0);
}

const visi_unitsType visi_DataGrid::MetricUnitsType(int i) {
  if((i < numMetrics) && (i>=0))
    return(metrics[i].UnitsType());
  return Sampled;
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
  return(VISI_ERROR_INT);

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
    return(VISI_ERROR_INT);
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
    return(VISI_ERROR_INT);
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
      resources[i] = Resource(temp[i].Name(),temp[i].Identifier());
  }
  for(i = numResources; i < (numResources + howmany); i++){
      resources[i] = Resource(rlist[i-numResources].name,
			  rlist[i-numResources].Id);
  }

  numResources += howmany;

  // add space to data grid for new resources
  for(i = 0; i < numMetrics; i++){
      if((ok = data_values[i].AddNewResources(howmany)) != VISI_OK){
          delete[] temp;
          temp = 0;
          return(ok); 
      }
  }

  delete[] temp;
  temp = 0;
  return(VISI_OK);
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
    metrics[i] = Metric(temp[i].CurrUnits(),temp[i].TotUnits(),
			temp[i].Name(),temp[i].Identifier(),
			temp[i].Aggregate(),temp[i].UnitsType());
  }
  for(i = numMetrics; i < (numMetrics + howmany); i++){
    metrics[i] = Metric(mlist[i-numMetrics].curr_units, 
			mlist[i-numMetrics].tot_units, 
			mlist[i-numMetrics].name,
		        mlist[i-numMetrics].Id, mlist[i-numMetrics].aggregate,
			mlist[i-numMetrics].unitstype);
  }


  // add space to data grid for new metrics

  tempdata = data_values;
  data_values = new visi_GridHistoArray[numMetrics + howmany];

  for(i=0; i < numMetrics; i++){
    if(data_values[i].AddNewValues(tempdata[i].Value(),tempdata[i].Size())
       != VISI_OK){
       return(VISI_ERROR_INT); 
    }
  }

  for(i=numMetrics; i < (numMetrics + howmany); i++){
      visi_GridHistoArray *temp = new visi_GridHistoArray(numResources);
      data_values[i] = *temp;
      delete temp;
  }

  numMetrics += howmany;
  tempdata = 0;
  temp = 0;
  return(VISI_OK);
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

void visi_DataGrid::AddNewPhase(int handle, visi_timeType start,
                                visi_timeType end, visi_timeType width,
                                pdstring name){
    PhaseInfo *p = new PhaseInfo(handle,start,end,width,name.c_str());
    phases += p;
    phases.sort(phaseCompare);
}
