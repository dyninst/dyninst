/*
 * Copyright (c) 1996-1999 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

#if !defined(i386_unknown_nt4_0)
#include <stream.h> 
#endif // !defined(i386_unknown_nt4_0)

#include "visi/src/visualizationP.h"
#include "visi/src/datagridP.h"
#include "visi/src/visiTypesP.h"
#include "pdutil/h/makenan.h"
#include "visi/h/visualization.h"
#include "common/h/Types.h" // Address

#define MAXSTRINGSIZE  16*1024
#define EVENTSIZE      FOLD+1

static visi_DataGrid  visi_dataGrid;
// trace data streams
static visi_TraceData visi_traceData;
static int            visi_LastBucketSent = -1;
static PDSOCKET visi_fileDesc;
static int (*visi_eventCallbacks[EVENTSIZE])(int);
static int visi_initDone = 0;
static visualization *visi_vp;

#ifdef SAMPLEVALUE_DEBUG
debug_ostream sampleVal_cerr(cerr, true);
#else
debug_ostream sampleVal_cerr(cerr, false);
#endif

int
visi_callback( void )
{
	int ret = 0;
	bool done = false;

	// handle all available data on the XDR stream
	//
	// Note that with this arrangement, the visi hands
	// control over to the visi library until it has
	// processed all available data.  If the sender (Paradyn front end)
	// is producing data at a faster rate than the visi can consume it,
	// the visi GUI will be unresponsive while it is handling the
	// data.  However, even in this case, the data will eventually
	// back up the connection to the front end, and the front end
	// will block writing data on this synchronous connection.
	//
	// The alternative would be to return some indication to the 
	// caller that more data is available, so that the caller can
	// decide when to continue consuming XDR records.
	// 
	while( !done )
	{
		// handle the XDR record
		ret = visi_vp->waitLoop();

		// check if more data is available on the stream
		if( xdrrec_eof( visi_vp->net_obj() ) )
		{
			done = true;
		}
	}

	return ret;
}

	
///////////////////////////////////////////////////////////
// paradyn initialization routine connects to parent socket,
// and registers the visualization::mainLoop routine as 
// callback on events on fileDesc[0]
///////////////////////////////////////////////////////////
PDSOCKET visi_Init(){

int i;

  visi_fileDesc = INVALID_PDSOCKET;
  for(i=0;i<EVENTSIZE;i++){
    visi_eventCallbacks[i] = NULL;
  }

#if !defined(i386_unknown_nt4_0)
  PDSOCKET sock = 0;
#else
  PDSOCKET sock = _get_osfhandle(0);
#endif

  visi_vp = new visi_visualization(sock);
  visi_fileDesc = sock;
  visi_initDone = 1;
 
  // make request for info. about all phases defined so far 
  visi_vp->GetPhaseInfo();

  return(visi_fileDesc);
}

///////////////////////////////////////////////////////////
// makes initial call to get Metrics and Resources 
// for visualizations that do not provide an event that 
// invokes the GetMetsRes upcall, this routine should be
// called by the visiualizaiton before entering the mainloop
///////////////////////////////////////////////////////////
int visi_StartVisi(int argc,
	      char *argv[]){

  if(!visi_initDone)
    visi_Init();

  // call GetMetricResources with initial metric resource lists
  if(argc == 3)
   visi_vp->GetMetricResource(argv[1],(int)(Address)argv[2],0);
  else
   visi_vp->GetMetricResource("",0,0);
  return(VISI_OK);

}

///////////////////////////////////////////////////////////
// cleans up visi interface data structs 
// Visualizations should call this routine before exiting 
///////////////////////////////////////////////////////////
void visi_QuitVisi(){

    delete visi_vp;
}

//
// call back to Paradyn to display error message
//
void visi_showErrorVisiCallback(const char *msg)
{
  int string_size;
  string new_msg = msg; 
  string_size = (new_msg.length())*sizeof(char);
  if (string_size < MAXSTRINGSIZE)
    visi_vp->showError(87,new_msg);
  else {
    string errmsg;
    errmsg = string("Internal Error: error message has exceeded maximum length of ") + string(MAXSTRINGSIZE) + string(" bytes. Please, make your error message shorter.");
    visi_vp->showError(87,errmsg);
  }
}  

///////////////////////////////////////////////////////////
// registration callback routine for paradyn events
// sets eventCallbacks[event] to callback routine provided by user
///////////////////////////////////////////////////////////
int visi_RegistrationCallback(visi_msgTag event,
			 int (*callBack)(int)){

  if((event < EVENTSIZE)){
    visi_eventCallbacks[event] = callBack;
    return(VISI_OK);
  }
  return(VISI_ERROR_INT);
}

///////////////////////////////////////////////////////////
// invokes upcall to paradyn VISIthread associated with the visualization
// takes list of current metrics, list of foci, and type of data
// (0 for histogram, 1 for scalar). 
// currently, only the NULL string, type 0 case is supported 
///////////////////////////////////////////////////////////
void visi_GetMetsRes(char *, int){

  if(!visi_initDone)
    visi_Init();
  visi_vp->GetMetricResource(0,0,0);
}


///////////////////////////////////////////////////////////
// invokes upcall to paradyn.  Request to stop data for the 
// metric associated with metricIndex and resource associated with
// resourceIndex
///////////////////////////////////////////////////////////
void visi_StopMetRes(int metricIndex,
		     int resourceIndex){

  if(!visi_initDone)
    visi_Init();
  if((metricIndex < visi_dataGrid.NumMetrics()) 
      && (metricIndex >= 0)
      && (resourceIndex >= 0)
      && (resourceIndex <visi_dataGrid.NumResources())){
    visi_dataGrid[metricIndex][resourceIndex].ClearEnabled();
    visi_dataGrid[metricIndex][resourceIndex].Invalidate();
    bool met_error = false;
    bool res_error = false;
    u_int m = visi_dataGrid.MetricId(metricIndex,met_error);
    u_int r = visi_dataGrid.ResourceId(resourceIndex,res_error);
    if((!met_error) && (!res_error)){
        visi_vp->StopMetricResource(m,r);
    }
  }
}

///////////////////////////////////////////////////////////
// invokes upcall to paradyn.  Visualization sends phase
// definition to paradyn.  
///////////////////////////////////////////////////////////
void visi_DefinePhase(char *name, unsigned withPerfConsult,
		      unsigned withVisis) {
  if(!visi_initDone)
    visi_Init();
  if(withPerfConsult){
      if(withVisis){
          visi_vp->StartPhase((double)-1.0,name,true,true);
      }
      else {
	  visi_vp->StartPhase((double)-1.0,name,true,false);
      }
  }
  else if(withVisis) {
      visi_vp->StartPhase((double)-1.0,name,false,true);
  }
  else {
      visi_vp->StartPhase((double)-1.0,name,false,false);
  }
}

///////////////////////////////////////////////////////////
//  Visi interface routine.  Receives an array of data 
//  values from paradyn, adds them to the datagrid, and
//  invokes the callback routine associated with the
//  DATAVALUES event.
///////////////////////////////////////////////////////////
void visualization::Data(vector<T_visi::dataValue> data){
  sampleVal_cerr << "visualization::Data\n";
  if(!visi_initDone)
    visi_Init();

  int noMetrics = visi_dataGrid.NumMetrics();
  int noResources = visi_dataGrid.NumResources();

  // get metric and resource index into visi_dataGrid and add value if found
  for(unsigned i=0; i < data.size(); i++){
      int metric = visi_dataGrid.MetricIndex(data[i].metricId);
      int j = visi_dataGrid.ResourceIndex(data[i].resourceId);
      if((j >= 0) && (metric >= 0)){
	sampleVal_cerr << "  " << i <<" "<<visi_dataGrid.MetricName(metric)
		       << " - " << visi_dataGrid.ResourceName(j) 
		       << "   AddValue  bucket: " << data[i].bucketNum 
		       << "  data: " << data[i].data << "\n";
         visi_dataGrid.AddValue(metric,j,
		         data[i].bucketNum,
		         data[i].data);
  }} 

  int min;
  int max = visi_dataGrid.NumBins()+1;
  min = max;
  for(int i2=0; i2 < noMetrics; i2++){
      for(int k=0; k < noResources; k++){
          if(visi_dataGrid.Valid(i2,k)){
              int temp = visi_dataGrid.LastBucketFilled(i2,k);  
              if((temp > -1) && (temp < min))
              min = temp; 
  }}}

  //call user registered callback routine assoc. w/event DATAVALUES
  if(/*(min > visi_LastBucketSent) //if new datagrid cross-section has been filled
     && (min != max) 
     && */ 
     (visi_eventCallbacks[DATAVALUES]!=NULL)){ //theres a callback routine 
       visi_LastBucketSent = min;
       visi_eventCallbacks[DATAVALUES](visi_LastBucketSent);
  }
}

///////////////////////////////////////////////////////////
//  Visi interface routine.  Receives trace data values from paradyn and
//  invokes the callback routine associated with the
//  TRACEDATAVALUES event.
///////////////////////////////////////////////////////////
void visualization::TraceData(vector<T_visi::traceDataValue> traceData){

  if(!visi_initDone)
    visi_Init();

  // int noMetrics = visi_dataGrid.NumMetrics();        // unused
  // int noResources = visi_dataGrid.NumResources();    // unused

  // get metric and resource index into visi_dataGrid and add value if found
  for(unsigned i=0; i < traceData.size(); i++){
    if (traceData[i].traceDataRecord.length()) {
      int metric = visi_dataGrid.MetricIndex(traceData[i].metricId);
      int j = visi_dataGrid.ResourceIndex(traceData[i].resourceId);
      if((j >= 0) && (metric >= 0)){
         visi_traceData.metricIndex = metric;
         visi_traceData.resourceIndex = j;
         visi_traceData.dataRecord =
              new byteArray(traceData[i].traceDataRecord.getArray(),
              traceData[i].traceDataRecord.length());
      }


  //call user registered callback routine assoc. w/event TRACEDATAVALUES
  if ((visi_eventCallbacks[TRACEDATAVALUES]!=NULL)){ //theres a callback routine
       visi_eventCallbacks[TRACEDATAVALUES](visi_traceData.dataRecord->length());
  }
  delete visi_traceData.dataRecord;
    }
  }
}

///////////////////////////////////////////////////////////
//  Visi interface routine.  Receives notification of a
//  fold event, and the new bucket width.  Invokes
//  a fold operation on the datagrid
///////////////////////////////////////////////////////////
void visualization::Fold(double newBucketWidth){
  
  int ok;

  if(!visi_initDone)
    visi_Init();

  visi_dataGrid.Fold(newBucketWidth);
  // assume a fold can only occur when datagrid histogram buckets are full
  visi_LastBucketSent = (visi_dataGrid.NumBins()/2) - 1;

  //call user registered callback routine assoc. w/event FOLD
  if(visi_eventCallbacks[FOLD] !=  NULL){
     ok = visi_eventCallbacks[FOLD](0);
  }
}

///////////////////////////////////////////////////////////
// Visi interface routine.  Receives notification of an
// invalid metric/resource pair.  Invalidataes the datagrid
// cell associated with the metricId m and resourceId r.
///////////////////////////////////////////////////////////
void visualization::InvalidMR(u_int m, u_int r){

int i,j;
int ok;

  if(!visi_initDone)
    visi_Init();

  // search for indices associated with metricId m and
  // resourceId r
  bool error = false;
  for(i=0; (i<visi_dataGrid.NumMetrics()) &&(m != visi_dataGrid.MetricId(i,error)); i++){
      if(error) return;
  } 

  for(j=0;(j<visi_dataGrid.NumResources())&&(r!= visi_dataGrid.ResourceId(j,error));j++){
      if(error) return;
  }

  visi_dataGrid[i][j].ClearEnabled();
  ok  = visi_dataGrid.Invalidate(i,j);

  //call callback routine assoc. w/event INVALIDMETRICSRESOURCES 
  if(visi_eventCallbacks[INVALIDMETRICSRESOURCES] != NULL){
     ok = visi_eventCallbacks[INVALIDMETRICSRESOURCES](0);
  }
}

///////////////////////////////////////////////////////////
// Visi interface routine.  Receives a list of metrics and
// resources to add to the datagrid.
///////////////////////////////////////////////////////////
void visualization::AddMetricsResources(vector<T_visi::visi_matrix> newElements,
			 		double bucketWidth,
			 		int nobuckets,
					double start_time,
    				        int phase_handle){


  int ok;
  visi_metricType *mets = 0;
  visi_resourceType *res = 0;
  int numRes, numMet;

  if(!visi_initDone)
    visi_Init();

  // this is first set of metrics/resources, construct new visi_dataGrid
  if(!visi_dataGrid.NumMetrics()){
    // create list of all unique metric and resource entries
    // in newElements
    numRes = 0;
    numMet = 0;
    if((res= new visi_resourceType [newElements.size()]) == NULL){ 
	return;
    }				   
    if((mets= new visi_metricType [newElements.size()]) == NULL){
        return;
    }				   
    for(unsigned i = 0; i < newElements.size(); i++){
        ok = 0;
	for(int j=0; (j < numMet) && (!ok);j++){
	   if(newElements[i].met.Id == mets[j].Id)
	     ok = 1;
  	}
	if(!ok){
	    if(!newElements[i].met.name.length())
	        mets[numMet].name = NULL;
            else
	        mets[numMet].name = newElements[i].met.name.string_of();
            if(!newElements[i].met.units.length())
	        mets[numMet].units = NULL;
            else
	        mets[numMet].units = newElements[i].met.units.string_of();
            mets[numMet].Id = newElements[i].met.Id;
	    if(newElements[i].met.unitstype == 0){
		mets[numMet].unitstype = UnNormalized;
	    }
	    else if(newElements[i].met.unitstype == 1){
		mets[numMet].unitstype = Normalized;
	    }
	    else {
                mets[numMet].unitstype = Sampled;
	    }
	    mets[numMet++].aggregate = newElements[i].met.aggregate;
	}
	  ok = 0;
	for(int j2=0; (j2 < numRes) && (!ok);j2++){
	   if(newElements[i].res.Id == res[j2].Id)
	     ok = 1;
	}
	if(!ok){
	    if(!newElements[i].res.name.length())
	        res[numRes].name = NULL;
            else
                res[numRes].name = newElements[i].res.name.string_of();
            res[numRes++].Id = newElements[i].res.Id;
	}
    }

    // construct new visi_dataGrid 
    visi_dataGrid = visi_DataGrid(numMet,
			   numRes,
			   mets,
			   res,
			   nobuckets,
			   (visi_timeType)bucketWidth,
			   (visi_timeType)start_time,
			   phase_handle);
    // trace data streams
    // construct new visi_traceData
    visi_traceData = visi_TraceData();
  }
  else{ // add elements to existing data grid

    // create list of new resources and add them to resource list
    res= new visi_resourceType [newElements.size()];
    numRes = 0;
    for(unsigned i=0; i < newElements.size(); i++){
      if(!visi_dataGrid.ResourceInGrid(newElements[i].res.Id)){
          ok = 0;
          for(int k=0; (k < numRes) && !ok; k++){
	     if(newElements[i].res.Id == res[k].Id)
	       ok = 1;
	  }
	  if(!ok){
	      if(!newElements[i].res.name.length())
	        res[numRes].name = NULL;
              else
                res[numRes].name = newElements[i].res.name.string_of();
              res[numRes++].Id = newElements[i].res.Id;
          }
    }}

    // add new resources to visi_dataGrid
    if(numRes > 0)
      visi_dataGrid.AddNewResource(numRes,res);

    // create list of new metrics and add them to metricsList
    mets = new visi_metricType [newElements.size()];
    numMet = 0;
    for(unsigned i2=0; i2 < newElements.size(); i2++){
      if(!visi_dataGrid.MetricInGrid(newElements[i2].met.Id)){

          ok = 0;
          for(int k2=0; (k2 < numMet) && !ok; k2++){
	     if(newElements[i2].met.Id == mets[k2].Id)
	       ok = 1;
	  }
	  if(!ok){
	      if(!newElements[i2].met.name.length())
	          mets[numMet].name = NULL;
              else
	          mets[numMet].name = newElements[i2].met.name.string_of();
              if(!newElements[i2].met.units.length())
	          mets[numMet].units = NULL;
              else
	          mets[numMet].units = newElements[i2].met.units.string_of();
            mets[numMet].Id = newElements[i2].met.Id;
	    if(newElements[i2].met.unitstype == 0){
		mets[numMet].unitstype = UnNormalized;
	    }
	    else if(newElements[i2].met.unitstype == 1){
		mets[numMet].unitstype = Normalized;
	    }
	    else {
                mets[numMet].unitstype = Sampled;
	    }
	    mets[numMet++].aggregate = newElements[i2].met.aggregate;
	}
    }}

    // add new metrics to visi_dataGrid
    if(numMet > 0)
      visi_dataGrid.AddNewMetrics(numMet,mets);
  }

  // set enabled for every element of newElements list 
  for(unsigned r = 0; r < newElements.size(); r++){
     visi_dataGrid[visi_dataGrid.MetricIndex(newElements[r].met.Id)][visi_dataGrid.ResourceIndex(newElements[r].res.Id)].SetEnabled();
  }
 
  delete [] mets;
  delete [] res;
  //call callback routine assoc. w/event ADDMETRICSRESOURCES 
  if(visi_eventCallbacks[ADDMETRICSRESOURCES] !=  NULL){
     ok = visi_eventCallbacks[ADDMETRICSRESOURCES](0);
  }
}

///////////////////////////////////////////////////////////
// Visi interface routine.   Receives an array of histogram 
// values for the datagrid cell indicated by metricId and 
// resourceId 
///////////////////////////////////////////////////////////
void visualization::BulkDataTransfer(vector<float> values,
				     u_int metricId,
				     u_int resourceId){
int lastBucket, temp, j;
int met=-1, res=-1;
int noMetrics = visi_dataGrid.NumMetrics();
int noResources = visi_dataGrid.NumResources();

    // find datagrid indicies associated with metricId, resourceId 
    bool found = false;
    bool error = false;
    for(int i = 0; i < noMetrics; i++){
	u_int m_id = visi_dataGrid.MetricId(i,error);
	if(!error){
            if(m_id == metricId){
	        met = i;
	        found = true;
            }
        }
    }

    if(!found) return;
    found = false;
    error = false;
    for(int i1 = 0; i1 < noResources; i1++){
	u_int r_id = visi_dataGrid.ResourceId(i1,error);  
	if(!error){
            if(r_id == resourceId){
	        res = i1;
	        found = true;
            }
        }
    }
    if(!found) return;

    // add new data values to datagrid
    for(unsigned i2 = 0; i2 < values.size(); i2++){
       if(!isnan(values[i2])){
           visi_dataGrid.AddValue(met, res, i2, values[i2]);
       }
    }
   
    // find last full cross section for new visi_dataGrid 
    lastBucket = visi_dataGrid.NumBins()+1;
    for(int i3=0; i3 < noMetrics; i3++){
        for(j=0; j < noResources; j++){
            if(visi_dataGrid.Valid(i3,j)){
                temp = visi_dataGrid.LastBucketFilled(i3,j);  
                if((temp > -1) && (temp < lastBucket))
                lastBucket = temp; 
            }
        }
    }

    // call DATAVALUES callback routine
    if(visi_eventCallbacks[DATAVALUES] !=  NULL){
       visi_eventCallbacks[DATAVALUES](lastBucket);
    }

}


///////////////////////////////////////////////////////////
// Visi interface routine.  Visualization recieves Phase
// start information from Paradyn.
///////////////////////////////////////////////////////////
void visualization::PhaseStart(double begin,
			  double end,
			  double bucketWidth,
			  string name,
			  u_int handle){

  if(!visi_initDone)
    visi_Init();
   
   // add new phase to phase vector
   visi_dataGrid.AddNewPhase(handle,(visi_timeType)begin,(visi_timeType)end,
			(visi_timeType)bucketWidth,name);

  //call callback routine assoc. w/event PHASESTART
  if(visi_eventCallbacks[PHASESTART] !=  NULL){
     visi_eventCallbacks[PHASESTART](visi_dataGrid.NumPhases()-1);
  }
}

///////////////////////////////////////////////////////////
// Visi interface routine.  Visualization recieves Phase
// end information from Paradyn.
///////////////////////////////////////////////////////////
void visualization::PhaseEnd(double end, u_int handle){


  if(!visi_initDone)
    visi_Init();

   // update phase end time for phase assoc w/handle
   int ok;
   if(!(ok = visi_dataGrid.AddEndTime(end,handle))){
       // fprintf(stderr,"in visualization::PhaseEnd: phase end not added\n");
   }
   
  //call callback routine assoc. w/event PHASEEND
  if(visi_eventCallbacks[PHASEEND] !=  NULL){
     visi_eventCallbacks[PHASEEND](visi_dataGrid.NumPhases()-1);
  }
}

///////////////////////////////////////////////////////////
// Visi interface routine.  Visualization recieves list  
// of all Phase info from Paradyn.
///////////////////////////////////////////////////////////
void visualization::PhaseData(vector<T_visi::phase_info> phases){

  if(!visi_initDone)
    visi_Init();

  // add an new phase object to the visi_dataGrid's vector of phases
   for (unsigned i=0; i < phases.size(); i++){ 
     visi_dataGrid.AddNewPhase(phases[i].handle,
                (visi_timeType)phases[i].start,
		(visi_timeType)phases[i].end,
		(visi_timeType)phases[i].bucketWidth,
		phases[i].name.string_of());
   }

  //call callback routine assoc. w/event PHASEDATA
  if(visi_eventCallbacks[PHASEDATA] !=  NULL){
     visi_eventCallbacks[PHASEDATA](0);
  }
}

void visi_visualization::handle_error(){
   // call user registered callback routine assoc. w/event PARADYNEXITED
   if(visi_eventCallbacks[PARADYNEXITED] !=  NULL){
      visi_eventCallbacks[PARADYNEXITED](0);
   }
   // otherwise, exit
   else {
      exit(-1);
   }
}

//***************************** Data Grid Routines ************

//
// returns the ith metric name or 0 on error
//
const char *visi_MetricName(int metric_num){ 
    return visi_dataGrid.MetricName(metric_num);
}

//
// returns the ith metric units name or 0 on error
//
const char *visi_MetricUnits(int metric_num){
    return visi_dataGrid.MetricUnits(metric_num);
}

//
// returns the ith metric units label for data values or 0 on error
//
const char *visi_MetricLabel(int metric_num){
    return visi_dataGrid.MetricLabel(metric_num);
}

//
// returns the ith metric units label for average aggregate data values,
// or 0 on error
//
const char *visi_MetricAveLabel(int metric_num){
    return visi_dataGrid.MetricAveLabel(metric_num);
}

//
// returns the ith metric units label for sum aggregate data values,
// or 0 on error
//
const char *visi_MetricSumLabel(int metric_num){
    return visi_dataGrid.MetricSumLabel(metric_num);
}

//
// returns the ith resource's name,  or 0 on error
//
const char *visi_ResourceName(int resource_num){
    return visi_dataGrid.ResourceName(resource_num);
}

//
//  returns the number of metrics in the data grid
//
int visi_NumMetrics(){
    return visi_dataGrid.NumMetrics();
}

//
//  returns the number of resources in the data grid
//
int visi_NumResources(){
    return visi_dataGrid.NumResources();
}

//
//  returns the number of phases currently defined in the system
//
unsigned visi_NumPhases(){
    return visi_dataGrid.NumPhases();
}

//
// returns the start time of the phase for which this visi is defined
//
visi_timeType visi_GetStartTime(){
    return visi_dataGrid.GetStartTime();
}

//
// returns the name of the phase for which this visi is defined
//
const char *visi_GetMyPhaseName(){
    return visi_dataGrid.GetMyPhaseName();
}

//
// returns the handle of the phase for which this visi is defined or
// -1 on error
//
int visi_GetMyPhaseHandle(){

    return (visi_dataGrid.GetPhaseHandle());
}

//
// returns the handle of the phase for which this visi is defined or
// -1 on error
//
int visi_GetPhaseHandle(unsigned phase_num){

    const PhaseInfo *p = visi_dataGrid.GetPhaseInfo(phase_num);
    if(p){
        return (p->getPhaseHandle());
    }
    return (-1);
}

//
// returns phase name for the ith phase, or returns 0 on error
//
const char *visi_GetPhaseName(unsigned phase_num){

    const PhaseInfo *p = visi_dataGrid.GetPhaseInfo(phase_num);
    if(p){
        return (p->getName());
    }
    return (0);
}

//
// returns phase start time for the ith phase, or returns -1.0 on error
//
visi_timeType visi_GetPhaseStartTime(unsigned phase_num){

    const PhaseInfo *p = visi_dataGrid.GetPhaseInfo(phase_num);
    if(p){
        return (p->getStartTime());
    }
    return (-1.0);
}

//
// returns phase end time for the ith phase, or returns -1.0 on error
//
visi_timeType visi_GetPhaseEndTime(unsigned phase_num){

    const PhaseInfo *p = visi_dataGrid.GetPhaseInfo(phase_num);
    if(p){
        return (p->getEndTime());
    }
    return (-1.0);
}

//
// returns phase bucket width for the ith phase, or returns -1.0 on error
//
visi_timeType visi_GetPhaseBucketWidth(unsigned phase_num){

    const PhaseInfo *p = visi_dataGrid.GetPhaseInfo(phase_num);
    if(p){
        return (p->getBucketWidth());
    }
    return (-1.0);
}

//
// returns the average of all the data bucket values for the metric/resource
// pair "metric_num" and "resource_num", returns NaN value on error
//
visi_sampleType visi_AverageValue(int metric_num, int resource_num){
    return visi_dataGrid.AggregateValue(metric_num,resource_num);
}

//
// returns the sum of all the data bucket values for the metric/resource
// pair "metric_num" and "resource_num", returns NaN value on error
//
visi_sampleType visi_SumValue(int metric_num, int resource_num){
    return visi_dataGrid.SumValue(metric_num,resource_num);
}

//
// returns the data value in bucket "bucket_num" for the metric/resource pair
// "metric_num" and "resource_num", returns NaN value on error
//
visi_sampleType visi_DataValue(int metric_num, int resource_num, int bucket_num){
    return visi_dataGrid[metric_num][resource_num][bucket_num];
}

//
// returns the data values for the metric/resource pair "metric_num" 
// and "resource_num", returns NaN value on error
//
const visi_sampleType *visi_DataValues(int metric_num, int resource_num){

    if((metric_num >= 0) && (metric_num < visi_dataGrid.NumMetrics())
       && (resource_num >= 0) && (resource_num < visi_dataGrid.NumResources())){
        return visi_dataGrid[metric_num][resource_num].Value();
    }
    return 0;
}

// trace data streams
//
// returns the pointer to the trace data record 
//

const visi_TraceData *visi_TraceDataValues(){

    return &visi_traceData;
}

//
//  returns true if the data grid cell corresponding to metric_num
//  and resource_num contains data
//
int visi_Valid(int metric_num, int resource_num){
    if (visi_dataGrid.Valid(metric_num,resource_num))
	return 1;
    else
	return 0;
}

//
//  returns true if the data collection has been enabled for metric_num
//  and resource_num
//
int visi_Enabled(int metric_num, int resource_num){
    if (visi_dataGrid[metric_num][resource_num].Enabled())
	return 1;
    else
	return 0;
}


//
//  returns the number of buckets in each data grid cell's histogram
//
int visi_NumBuckets(){
    return visi_dataGrid.NumBins();
}

//
//  returns the buckets width (in seconds) of each data grid cell's histogram
//
visi_timeType visi_BucketWidth(){
    return visi_dataGrid.BinWidth();
}

//
// returns the first data bucket with valid data values
//
int visi_FirstValidBucket(int metric_num, int resource_num){
    return visi_dataGrid[metric_num][resource_num].FirstValidBucket();
}

//
// returns the last data bucket with valid data values
//
int visi_LastBucketFilled(int metric_num,int resource_num){
    return visi_dataGrid.LastBucketFilled(metric_num, resource_num);
}

//
// returns true if there are invalid spans of data between the first
// valid bucket and the last bucket filled
//
int visi_InvalidSpans(int metric_num,int resource_num){
    if(visi_dataGrid.InvalidSpans(metric_num, resource_num)) return 1;
    else return 0;
}

//
// returns the user data associated with metric_num and resource_num
// returns 0 on error
//
void *visi_GetUserData(int metric_num, int resource_num){

    if((metric_num >= 0) && (metric_num < visi_dataGrid.NumMetrics())
       && (resource_num >= 0) && (resource_num < visi_dataGrid.NumResources())){
        return visi_dataGrid[metric_num][resource_num].userdata;
    }
    return 0;
}

//
// sets the user data associated with metric_num and resource_num
//
int visi_SetUserData(int metric_num, int resource_num, void *data){

    if((metric_num >= 0) && (metric_num < visi_dataGrid.NumMetrics())
       && (resource_num >= 0) && (resource_num < visi_dataGrid.NumResources())){
        visi_dataGrid[metric_num][resource_num].userdata = data;
	return 1;
    }
    return 0;
}

//
// sets the trace data associated with metric_num and resource_num
//
int visi_SetTraceData(int metric_num, int resource_num, visi_sampleType data){
      if((resource_num >= 0) && (metric_num >= 0)){
         visi_dataGrid.AddValue(metric_num,resource_num,
                visi_dataGrid.LastBucketFilled(metric_num, resource_num) + 1,
                data);
         return 1;
      }
      return 0;
}

void visi_PrintDataBuckets(int step){

    int noMetrics = visi_dataGrid.NumMetrics();
    int noResources = visi_dataGrid.NumResources();
    int noBuckets = visi_dataGrid.NumBins();
    for(int i = 0; i < noMetrics; i++){
	for(int j=0; j < noResources; j++){
            if (visi_dataGrid[i][j].Valid()){
		cerr << visi_dataGrid.MetricName(i) << "/" 
		     << visi_dataGrid.ResourceName(j) << endl;
		for(int k=0; k < noBuckets; k+=step){
		    cerr << "value(" << k << ") = " << visi_dataGrid[i][j][k]
			 << endl;
		}
	    }
	}
    }

}

