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
/* Revision 1.39  1996/01/17 18:29:18  newhall
/* reorginization of visiLib
/*
 * Revision 1.38  1996/01/05 20:02:43  newhall
 * changed parameters to showErrorVisiCallback, so that visilib users are
 * not forced into using our string class
 *
 * Revision 1.37  1995/12/18 23:22:05  newhall
 * changed metric units type so that it can have one of 3 values (normalized,
 * unnormalized or sampled)
 *
 * Revision 1.36  1995/12/18 17:22:07  naim
 * Adding function showErrorVisiCallback to display error messages from
 * visis - naim
 *
 * Revision 1.35  1995/12/15  20:15:24  naim
 * Adding call back function to display error messages from visis - naim
 *
 * Revision 1.34  1995/11/17  17:28:40  newhall
 * added normalized member to Metric class which specifies units type
 * added MetricLabel, MetricAveLabel, and MetricSumLabel DG method functions
 *
 * Revision 1.33  1995/11/13  17:24:25  newhall
 * bug fix
 *
 * Revision 1.32  1995/11/12  23:29:55  newhall
 * removed warnings, removed error.C
 *
 * Revision 1.31  1995/11/12  00:45:19  newhall
 * added PARADYNEXITED event, added "InvalidSpans" dataGrid method
 *
 * Revision 1.30  1995/11/02  02:12:51  newhall
 * added class derived from igen generated visualization class that contains
 * a handle_error method that won't print an error msg when an error occurs
 *
 * Revision 1.29  1995/09/18  18:26:06  newhall
 * updated test subdirectory, added visilib routine GetMetRes()
 *
 * Revision 1.28  1995/09/08  19:47:00  krisna
 * stupid way to avoid the for-scope problem
 *
 * Revision 1.27  1995/08/05 17:12:18  krisna
 * use `0' instead of `NULL'
 *
 * Revision 1.26  1995/08/01 01:59:35  newhall
 * changes relating to phase interface stuff
 *
 * Revision 1.25  1995/06/02  21:02:06  newhall
 * changed type of metric and focus handles to u_int
 *
 * Revision 1.24  1995/03/31  15:56:11  jcargill
 * Changed malloc's to new's, so that constructors would get fired;
 * otherwise, bogus memory references/free's occur.
 *
 * Revision 1.23  1995/02/26  01:59:40  newhall
 * added phase interface functions
 *
 * Revision 1.22  1995/02/16  09:31:03  markc
 * Modified NaN generation code for machines that do not have nan.h.
 * This code has not been tested.
 *
 * Revision 1.21  1995/01/30  17:35:27  jcargill
 * Updated igen-generated includes to new naming convention
 *
 * Revision 1.20  1994/11/02  04:15:00  newhall
 * memory leak fixes
 *
 * Revision 1.19  1994/10/13  15:39:17  newhall
 * QuitVisi added
 *
 * Revision 1.18  1994/09/30  21:00:51  newhall
 * use datagrid method functions MetricId and ResourceId
 *
 * Revision 1.17  1994/09/25  02:00:29  newhall
 * changes to visi interface routines that take list of met/focus pairs:
 * AddMetricsResources, GetMetRes
 * and changes to support the new version of igen
 *
 * Revision 1.16  1994/09/22  03:14:41  markc
 * declared arrays at start
 * incremented version number
 *
 * Added stronger compiler warnings
 * removed compiler warnings
 *
 * Revision 1.15  1994/08/13  20:34:50  newhall
 * removed all code associated with class visi_MRList
 * removed mrlist src and obj
 * removed
 *
 * Revision 1.14  1994/08/11  02:52:11  newhall
 * removed calls to grid cell Deleted member functions
 *
 * Revision 1.13  1994/08/03  20:49:12  newhall
 * removed code for interface routines NewMetricsResources and Enabled
 * changed AddMetricsResources to set grid cell's enabled flag
 *
 * Revision 1.12  1994/07/30  03:27:27  newhall
 * added visi interface functions Enabled and BulkDataTransfer
 *
 * Revision 1.11  1994/07/20  22:41:11  rbi
 * Small arguments fix to make identification of wildcards easier.
 *
 * Revision 1.10  1994/07/07  22:40:31  newhall
 * fixed compile warnings
 *
 * Revision 1.9  1994/06/16  18:24:53  newhall
 * fix to visualization::Data
 *
 * Revision 1.8  1994/06/07  17:48:49  newhall
 * support for adding metrics and resources to existing visualization
 *
 * Revision 1.7  1994/05/23  20:56:48  newhall
 * To visi_GridCellHisto class: added deleted flag, SumValue
 * method function, and fixed AggregateValue method function
 *
 * Revision 1.6  1994/05/11  17:13:14  newhall
 * changed data type from double to float
 *
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
#include <stream.h> 
#include "visi/src/visualizationP.h"
#include "visi/src/datagridP.h"
#include "visi/src/visiTypesP.h"
#include "util/h/makenan.h"
#include "visi/h/visualization.h"

#define MAXSTRINGSIZE  16*1024
#define EVENTSIZE      FOLD+1

static visi_DataGrid  visi_dataGrid;
static int            visi_LastBucketSent = -1;
static int visi_fileDesc;
static int (*visi_fileDescCallbacks)();
static int (*visi_eventCallbacks[EVENTSIZE])(int);
static int visi_initDone = 0;
static visualization *visi_vp;

// TODO -- better error checking here?
int visi_callback(){
  return(visi_vp->waitLoop());
}

///////////////////////////////////////////////////////////
// paradyn initialization routine connects to parent socket,
// and registers the visualization::mainLoop routine as 
// callback on events on fileDesc[0]
///////////////////////////////////////////////////////////
int visi_Init(){

int i;

//  for(i=0;i<FILETABLESIZE;i++){
//    fileDescCallbacks[i] = NULL;
//    fileDesc[i] = -1;
//  }
  visi_fileDescCallbacks = NULL;
  visi_fileDesc = -1;
  for(i=0;i<EVENTSIZE;i++){
    visi_eventCallbacks[i] = NULL;
  }

  visi_vp = new visi_visualization(0);
//  visi_fileDesc[0] = 0;
//  visi_fileDescCallbacks[0] = visi_callback;
  visi_fileDesc = 0;
  visi_fileDescCallbacks = visi_callback;
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
   visi_vp->GetMetricResource(argv[1],(int)argv[2],0);
  else
   visi_vp->GetMetricResource("",0,0);
  return(OK);

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
    return(OK);
  }
  return(ERROR_INT);
}

///////////////////////////////////////////////////////////
// fd registration and callback routine registration for user
// to register callback routines when they use the provided main routine
///////////////////////////////////////////////////////////
//int RegFileDescriptors(int *, int (*)()){
//  return(OK);
//}

///////////////////////////////////////////////////////////
// invokes upcall to paradyn VISIthread associated with the visualization
// takes list of current metrics, list of foci, and type of data
// (0 for histogram, 1 for scalar). 
// currently, only the NULL string, type 0 case is supported 
///////////////////////////////////////////////////////////
void visi_GetMetsRes(char *metres,
		int numElements,
		int ){

  if(!visi_initDone)
    visi_Init();
  visi_vp->GetMetricResource(metres,numElements,0);
}

void visi_GetMetsRes(){
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
void visi_DefinePhase(visi_timeType, char *name){

  if(!visi_initDone)
    visi_Init();
  visi_vp->StartPhase((double)-1.0,name);
}

///////////////////////////////////////////////////////////
//  Visi interface routine.  Receives an array of data 
//  values from paradyn, adds them to the datagrid, and
//  invokes the callback routine associated with the
//  DATAVALUES event.
///////////////////////////////////////////////////////////
void visualization::Data(vector<T_visi::dataValue> data){

  if(!visi_initDone)
    visi_Init();

  int noMetrics = visi_dataGrid.NumMetrics();
  int noResources = visi_dataGrid.NumResources();

  // get metric and resource index into visi_dataGrid and add value if found
  for(unsigned i=0; i < data.size(); i++){
      int metric = visi_dataGrid.MetricIndex(data[i].metricId);
      int j = visi_dataGrid.ResourceIndex(data[i].resourceId);
      if((j >= 0) && (metric >= 0)){
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
  if((min > visi_LastBucketSent) //if new datagrid cross-section has been filled
     && (min != max)
     && (visi_eventCallbacks[DATAVALUES]!=NULL)){ //theres a callback routine 
       visi_LastBucketSent = min;
       visi_eventCallbacks[DATAVALUES](visi_LastBucketSent);
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
    visi_dataGrid.visi_DataGrid(numMet,
			   numRes,
			   mets,
			   res,
			   nobuckets,
			   (visi_timeType)bucketWidth,
			   (visi_timeType)start_time,
			   phase_handle);
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
int noMetrics, noResources;
int met,res;


    // find datagrid indicies associated with metricId, resourceId 
    noMetrics = visi_dataGrid.NumMetrics();
    noResources = visi_dataGrid.NumResources();
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
       fprintf(stderr,"in visualization::PhaseEnd: phase end not added\n");
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
u_int visi_NumPhases(){
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
int visi_GetPhaseHandle(){

    return (visi_dataGrid.GetPhaseHandle());
}

//
// returns the handle of the phase for which this visi is defined or
// -1 on error
//
int visi_GetPhaseHandle(u_int phase_num){

    PhaseInfo *p = visi_dataGrid.GetPhaseInfo(phase_num);
    if(p){
        return (p->getPhaseHandle());
    }
    return (-1);
}

//
// returns phase name for the ith phase, or returns 0 on error
//
const char *visi_GetPhaseName(u_int phase_num){

    PhaseInfo *p = visi_dataGrid.GetPhaseInfo(phase_num);
    if(p){
        return (p->getName());
    }
    return (0);
}

//
// returns phase start time for the ith phase, or returns -1.0 on error
//
visi_timeType visi_GetPhaseStartTime(u_int phase_num){

    PhaseInfo *p = visi_dataGrid.GetPhaseInfo(phase_num);
    if(p){
        return (p->getStartTime());
    }
    return (-1.0);
}

//
// returns phase end time for the ith phase, or returns -1.0 on error
//
visi_timeType visi_GetPhaseEndTime(u_int phase_num){

    PhaseInfo *p = visi_dataGrid.GetPhaseInfo(phase_num);
    if(p){
        return (p->getEndTime());
    }
    return (-1.0);
}

//
// returns phase bucket width for the ith phase, or returns -1.0 on error
//
visi_timeType visi_GetPhaseBucketWidth(u_int phase_num){

    PhaseInfo *p = visi_dataGrid.GetPhaseInfo(phase_num);
    if(p){
        return (p->getBucketWidth());
    }
    return (-1.0);
}

//
// returns phase info. for the ith phase, or returns 0 on error
//
PhaseInfo *visi_GetPhaseInfo(u_int phase_num){
    return visi_dataGrid.GetPhaseInfo(phase_num);
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
visi_sampleType *visi_DataValues(int metric_num, int resource_num){

    if((metric_num >= 0) && (metric_num < visi_dataGrid.NumMetrics())
       && (resource_num >= 0) && (resource_num < visi_dataGrid.NumResources())){
        return visi_dataGrid[metric_num][resource_num].Value();
    }
    return 0;
}

//
//  returns true if the data grid cell corresponding to metric_num
//  and resource_num contains data
//
bool visi_Valid(int metric_num, int resource_num){
    return visi_dataGrid.Valid(metric_num,resource_num);
}

//
//  returns true if the data collection has been enabled for metric_num
//  and resource_num
//
bool visi_Enabled(int metric_num, int resource_num){
    return visi_dataGrid[metric_num][resource_num].Enabled();
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
bool visi_InvalidSpans(int metric_num,int resource_num){
    return visi_dataGrid.InvalidSpans(metric_num, resource_num);
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
bool visi_SetUserData(int metric_num, int resource_num, void *data){

    if((metric_num >= 0) && (metric_num < visi_dataGrid.NumMetrics())
       && (resource_num >= 0) && (resource_num < visi_dataGrid.NumResources())){
        visi_dataGrid[metric_num][resource_num].userdata = data;
	return true;
    }
    return false;
}

