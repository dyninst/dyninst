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

/* $Id: visi.I,v 1.23 2003/07/15 22:47:43 schendel Exp $ */ 


$remote struct metricType {
     pdstring curr_units; // how units are measured  i.e. "ms" 
     pdstring tot_units;  // how total units are measured
     pdstring name;       // for y-axis labeling  
     u_int  Id;         // unique metric Id
     int    aggregate;  // either SUM or AVE
     int    unitstype;  // specifies units type 0: unnormalized 1: normalized
			//                      2: sampled
};

$remote struct resourceType {
     pdstring name;     // obj. name for graph labeling
     u_int  Id;       // unique resource id
};

$remote struct dataValue {
  u_int metricId;
  u_int resourceId;
  int 	bucketNum;
  float data;
};

// trace data streams
$remote struct traceDataValue {
  int   metricId;
  int   resourceId;
  byteArray traceDataRecord;
};

$remote struct visi_matrix {
    resourceType res;
    metricType	 met;
};

$remote struct phase_info{
  double start;
  double end;
  double  bucketWidth;
  u_int handle;
  pdstring name;
};

$remote visualization {
  $base  7000;
  $version  16;

  // data: list of new data values from Paradyn 
  $async void Data(pdvector<dataValue> data);  

  // data: list of new trace data values from Paradyn
  $async void TraceData(pdvector<traceDataValue> data);

  // a fold event from paradyn
  $async void Fold(double newBucketWidth);

  // a fold event from paradyn
  $async void setInitialActualValue(u_int metricId, u_int resourceId,
				    double initActVal);

  //
  // A metric/focus pair has become invalid (from Paradyn)
  // m: metricId of invalid metric-focus combination
  // r: resourceId of invalid metric-focus combination
  //
  $async void InvalidMR(u_int m,u_int r);

  // Disable a metric/focus pair: doesn't fry existing data in the visilib
  // (unlike InvalidMR()), but does prevent future data from being added to it.
  // Folding will still be allowed on the curve.
  $async void disableMR(u_int m, u_int r);

  //
  // A new set of metrics and resources from Paradyn
  // newElements: a list of new metric-focus matrices
  // bucketWidth: current bucket width for histograms
  // nobuckets:   max number of buckets for histograms
  $async void AddMetricsResources(pdvector<visi_matrix> newElements,
				  double bucketWidth,
				  int nobuckets,
				  double start_time,
				  int phase_handle);
  //
  // A new phase has been defined by paradyn
  //
  $async void PhaseStart(double begin,  // phase start time
		    double end,		// -1 means phase has not ended yet
		    double bucketWidth,  // for histogram data values
		    pdstring name,	// string name for phase 
		    u_int handle);	// unique phase id 

  //
  // A phase specified by "handle" has ended
  // currently this is just used to fill in the end value in the phase table
  //
  $async void PhaseEnd(double end,
		       u_int handle);


  //
  // A list of phase data from paradyn (response to GetPhaseInfo upcall) 
  //
  $async void PhaseData(pdvector<phase_info> phases);


  //
  // An array of data values from Paradyn starting with bucket #0
  //
  $async void BulkDataTransfer(pdvector<float> values, // array of data values 
			       u_int metricId,	  // id of met. assoc. w/data
			       u_int resourceId);   // id of res. assoc. w/data


  //
  //  request from the visi to Paradyn for list of phases currently defined
  //  in system
  //
  $upcall $async void GetPhaseInfo();

  //
  // A request from the visualization to Paradyn for new data
  //
  $upcall $async void GetMetricResource(pdstring mets_res,
					int numElements,
					int type);
  //
  // A request to Paradyn to stop data associated with metricId & resourceId
  //
  $upcall $async void StopMetricResource(u_int metricId,
					 u_int resourceId);

  // request a new phase start from paradyn 
  // begin: phase start time (default of -1 means now)
  // name:  phase name (default of NULL means let paradyn provide name)
  $upcall $async void StartPhase(double begin, pdstring name,
	                         bool withPerfConsult,
	                         bool withVisis);

  //
  // call back to Paradyn to display error message
  //
  $upcall $async void showError(int code, pdstring msg);
};
