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
/* Revision 1.28  1995/09/08 19:47:00  krisna
/* stupid way to avoid the for-scope problem
/*
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
#include "visi.xdr.SRVR.h"
#include "visi/h/visualization.h"

visi_DataGrid  dataGrid;
int            LastBucketSent = -1;
int fileDesc[FILETABLESIZE];
int (*fileDescCallbacks[FILETABLESIZE])();
int (*eventCallbacks[EVENTSIZE])(int);
int initDone = 0;

visualization *vp;

// TODO -- better error checking here?
int visi_callback(){
  return(vp->waitLoop());
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

  vp = new visualization(0, NULL, NULL, false);
  fileDesc[0] = 0;
  fileDescCallbacks[0] = visi_callback;
  initDone = 1;
 
  // make request for info. about all phases defined so far 
  vp->GetPhaseInfo();

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
  if(argc == 3)
   vp->GetMetricResource(argv[1],(int)argv[2],0);
  else
   vp->GetMetricResource("",0,0);
  return(OK);

}

///////////////////////////////////////////////////////////
// cleans up visi interface data structs 
// Visualizations should call this routine before exiting 
///////////////////////////////////////////////////////////
void QuitVisi(){

    delete vp;

}




///////////////////////////////////////////////////////////
// registration callback routine for paradyn events
// sets eventCallbacks[event] to callback routine provided by user
///////////////////////////////////////////////////////////
int RegistrationCallback(msgTag event,
			 int (*callBack)(int)){

  if((event < EVENTSIZE)){
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
void GetMetsRes(char *metres,
		int numElements,
		int type){

  if(!initDone)
    VisiInit();
  vp->GetMetricResource(metres,numElements,0);
}


///////////////////////////////////////////////////////////
// invokes upcall to paradyn.  Request to stop data for the 
// metric associated with metricIndex and resource associated with
// resourceIndex
///////////////////////////////////////////////////////////
void StopMetRes(int metricIndex,
		int resourceIndex){

  if(!initDone)
    VisiInit();
  if((metricIndex < dataGrid.NumMetrics()) 
      && (metricIndex >= 0)
      && (resourceIndex >= 0)
      && (resourceIndex <dataGrid.NumResources())){
    dataGrid[metricIndex][resourceIndex].ClearEnabled();
    dataGrid[metricIndex][resourceIndex].Invalidate();
    u_int *r, *m;
    m = dataGrid.MetricId(metricIndex);
    r = dataGrid.ResourceId(resourceIndex);
    if(m && r)
        vp->StopMetricResource(*m,*r);
  }
}

///////////////////////////////////////////////////////////
// invokes upcall to paradyn.  Visualization sends phase
// definition to paradyn.  
///////////////////////////////////////////////////////////
void DefinePhase(timeType begin,
	         char *name){

  if(!initDone)
    VisiInit();
  //vp->StartPhase((double)begin,name);
  vp->StartPhase((double)-1.0,name);
}

///////////////////////////////////////////////////////////
//  Visi interface routine.  Receives an array of data 
//  values from paradyn, adds them to the datagrid, and
//  invokes the callback routine associated with the
//  DATAVALUES event.
///////////////////////////////////////////////////////////
void visualization::Data(vector<T_visi::dataValue> data){

int noMetrics, noResources;
int i,j,metric,ok;
int temp,min,max;


  if(!initDone)
    VisiInit();

  noMetrics = dataGrid.NumMetrics();
  noResources = dataGrid.NumResources();


  for(i=0; i < data.size(); i++){

      // get metric and resource index into dataGrid and add value if found
      metric = dataGrid.MetricIndex(data[i].metricId);
      j = dataGrid.ResourceIndex(data[i].resourceId);

      if((j >= 0) && (metric >= 0)){

         dataGrid.AddValue(metric,j,
		         data[i].bucketNum,
		         data[i].data);
      }
  } 

  min = max = dataGrid.NumBins()+1;
  for(i=0; i < noMetrics; i++){
      for(j=0; j < noResources; j++){
          if(dataGrid.Valid(i,j)){
              temp = dataGrid.LastBucketFilled(i,j);  
              if((temp > -1) && (temp < min))
              min = temp; 
          }
      }
  }

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
void visualization::InvalidMR(u_int m, u_int r){

int i,j;
int ok;

  if(!initDone)
    VisiInit();

  // search for indices associated with metricId m and
  // resourceId r
  for(i=0;
     (i<dataGrid.NumMetrics())
     &&(m!= *(dataGrid.MetricId(i))); i++) ;
  for(j=0;
      (j<dataGrid.NumResources())
      &&(r!= *(dataGrid.ResourceId(j))); j++) ;

  dataGrid[i][j].ClearEnabled();
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
void visualization::AddMetricsResources(vector<T_visi::visi_matrix> newElements,
			 		double bucketWidth,
			 		int nobuckets,
					double start_time,
    				        int phase_handle){


  int ok,i,j,k;
  visi_metricType *mets = 0;
  visi_resourceType *res = 0;
  int numRes, numMet;

  if(!initDone)
    VisiInit();

  // this is first set of metrics/resources, construct new dataGrid
  if(!dataGrid.NumMetrics()){
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
    for(i = 0; i < newElements.size(); i++){
        ok = 0;
	for(j=0; (j < numMet) && (!ok);j++){
	   if(newElements[i].met.Id == mets[j].Id)
	     ok = 1;
  	}
	if(!ok){
	    if(!newElements[i].met.name.length())
	        mets[numMet].name = NULL;
            else
	        mets[numMet].name = newElements[i].met.name;
            if(!newElements[i].met.units.length())
	        mets[numMet].units = NULL;
            else
	        mets[numMet].units = newElements[i].met.units;
            mets[numMet].Id = newElements[i].met.Id;
	    mets[numMet++].aggregate = newElements[i].met.aggregate;
	}
	  ok = 0;
	for(j=0; (j < numRes) && (!ok);j++){
	   if(newElements[i].res.Id == res[j].Id)
	     ok = 1;
	}
	if(!ok){
	    if(!newElements[i].res.name.length())
	        res[numRes].name = NULL;
            else
                res[numRes].name = newElements[i].res.name;
            res[numRes++].Id = newElements[i].res.Id;
	}
    }

    // construct new dataGrid 
    dataGrid.visi_DataGrid(numMet,
			   numRes,
			   mets,
			   res,
			   nobuckets,
			   (timeType)bucketWidth,
			   (timeType)start_time,
			   phase_handle);
  }
  else{ // add elements to existing data grid

    // create list of new resources and add them to resource list
    res= new visi_resourceType [newElements.size()];
    numRes = 0;
    for(i=0; i < newElements.size(); i++){
      if(!dataGrid.ResourceInGrid(newElements[i].res.Id)){
          ok = 0;
          for(k=0; (k < numRes) && !ok; k++){
	     if(newElements[i].res.Id == res[k].Id)
	       ok = 1;
	  }
	  if(!ok){
	      if(!newElements[i].res.name.length())
	        res[numRes].name = NULL;
              else
                res[numRes].name = newElements[i].res.name;
              res[numRes++].Id = newElements[i].res.Id;
          }
      }
    }

    // add new resources to dataGrid
    if(numRes > 0)
      dataGrid.AddNewResource(numRes,res);

    // create list of new metrics and add them to metricsList
    mets = new visi_metricType [newElements.size()];
    numMet = 0;
    for(i=0; i < newElements.size(); i++){
      if(!dataGrid.MetricInGrid(newElements[i].met.Id)){

          ok = 0;
          for(k=0; (k < numMet) && !ok; k++){
	     if(newElements[i].met.Id == mets[k].Id)
	       ok = 1;
	  }
	  if(!ok){
	      if(!newElements[i].met.name.length())
	          mets[numMet].name = NULL;
              else
	          mets[numMet].name = newElements[i].met.name;
              if(!newElements[i].met.units.length())
	          mets[numMet].units = NULL;
              else
	          mets[numMet].units = newElements[i].met.units;
            mets[numMet].Id = newElements[i].met.Id;
	    mets[numMet++].aggregate = newElements[i].met.aggregate;
	}
      }
    }


    // add new metrics to dataGrid
    if(numMet > 0)
      dataGrid.AddNewMetrics(numMet,mets);
  }

  // set enabled for every element of newElements list 
  for(k = 0; k < newElements.size(); k++){
     dataGrid[dataGrid.MetricIndex(newElements[k].met.Id)][dataGrid.ResourceIndex(newElements[k].res.Id)].SetEnabled();
  }
 
  delete [] mets;
  delete [] res;
  //call callback routine assoc. w/event ADDMETRICSRESOURCES 
  if(eventCallbacks[ADDMETRICSRESOURCES] !=  NULL){
     ok = eventCallbacks[ADDMETRICSRESOURCES](0);
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
    noMetrics = dataGrid.NumMetrics();
    noResources = dataGrid.NumResources();
    bool found = false;
    for(int i = 0; i < noMetrics; i++){
	u_int *m_id = dataGrid.MetricId(i);
	if(m_id)
            if(*m_id == metricId){
	        met = i;
	        found = true;
            }
    }

    if(!found) return;
    found = false;
    for(int i1 = 0; i1 < noResources; i1++){
	u_int *r_id = dataGrid.ResourceId(i1);  
	if(r_id)
            if(*r_id == resourceId){
	        res = i1;
	        found = true;
            }
    }
    if(!found) return;

    // add new data values to datagrid
    for(int i2 = 0; i2 < values.size(); i2++){
       if(!isnan(values[i2])){
           dataGrid.AddValue(met, res, i2, values[i2]);
       }
    }
   
    // find last full cross section for new dataGrid 
    lastBucket = dataGrid.NumBins()+1;
    for(int i3=0; i3 < noMetrics; i3++){
        for(j=0; j < noResources; j++){
            if(dataGrid.Valid(i3,j)){
                temp = dataGrid.LastBucketFilled(i3,j);  
                if((temp > -1) && (temp < lastBucket))
                lastBucket = temp; 
            }
        }
    }

    // call DATAVALUES callback routine
    if(eventCallbacks[DATAVALUES] !=  NULL){
       eventCallbacks[DATAVALUES](lastBucket);
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

  if(!initDone)
    VisiInit();
   
   // add new phase to phase vector
   dataGrid.AddNewPhase(handle,(timeType)begin,(timeType)end,
			(timeType)bucketWidth,name);

  //call callback routine assoc. w/event PHASESTART
  if(eventCallbacks[PHASESTART] !=  NULL){
     eventCallbacks[PHASESTART](dataGrid.NumPhases()-1);
  }
}

///////////////////////////////////////////////////////////
// Visi interface routine.  Visualization recieves Phase
// end information from Paradyn.
///////////////////////////////////////////////////////////
void visualization::PhaseEnd(double end, u_int handle){


  if(!initDone)
    VisiInit();

   // update phase end time for phase assoc w/handle
   int ok;
   if(!(ok = dataGrid.AddEndTime(end,handle))){
       fprintf(stderr,"in visualization::PhaseEnd: phase end not added\n");
   }
   
  //call callback routine assoc. w/event PHASEEND
  if(eventCallbacks[PHASEEND] !=  NULL){
     eventCallbacks[PHASEEND](dataGrid.NumPhases()-1);
  }
}

///////////////////////////////////////////////////////////
// Visi interface routine.  Visualization recieves list  
// of all Phase info from Paradyn.
///////////////////////////////////////////////////////////
void visualization::PhaseData(vector<T_visi::phase_info> phases){

  if(!initDone)
    VisiInit();

  // add an new phase object to the dataGrid's vector of phases
   for (int i=0; i < phases.size(); i++){ 
     dataGrid.AddNewPhase(phases[i].handle,
                (timeType)phases[i].start,
		(timeType)phases[i].end,
		(timeType)phases[i].bucketWidth,
		phases[i].name.string_of());
   }

  //call callback routine assoc. w/event PHASEDATA
  if(eventCallbacks[PHASEDATA] !=  NULL){
     eventCallbacks[PHASEDATA](0);
  }
}
