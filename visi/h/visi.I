
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

/* $Log: visi.I,v $
/* Revision 1.10  1995/02/26 01:59:28  newhall
/* added phase interface functions
/*
# Revision 1.9  1995/02/16  09:32:03  markc
# Modified to support machines which do not have NaN(x).
# This code has not been tested, but it compiles.
#
# Revision 1.8  1994/09/25  01:58:13  newhall
# changed interface definitions to work for new version of igen
# changed AddMetricsResources def. to take array of metric/focus pairs
#
# Revision 1.7  1994/09/22  03:12:24  markc
# declared arrays at start
# incremented version number
#
# Revision 1.6  1994/08/17  18:21:09  markc
# Incremented the version number because a new version of igen is in place.
#
# Revision 1.5  1994/08/03  20:47:28  newhall
# removed interface routines Enabled and NewMetricsResources
#
# Revision 1.4  1994/07/30  03:26:18  newhall
# added visi interface routines Enabled and BulkDataTransfer
#
# Revision 1.3  1994/05/11  17:11:07  newhall
# changed data values from double to float
#
# Revision 1.2  1994/04/13  21:23:09  newhall
# *** empty log message ***
#
# Revision 1.1  1994/03/29  02:42:13  newhall
# exported services of the visualization process
#
# Revision 1.3  1994/03/17  05:20:35  newhall
# changed bucket width and time value's type to double
#
# Revision 1.2  1994/03/14  20:28:52  newhall
# changed visi subdirectory structure
#  */ 


$remote struct metricType {
     string units;    // how units are measured  i.e. "ms" 
     string name;     // for y-axis labeling  
     int     Id;       // unique metric Id
     int     aggregate;  //either SUM or AVE
};

$remote struct resourceType {
     string name;     // obj. name for graph labeling
     int    Id;       // unique resource id
};

$remote struct dataValue {
  int 	metricId;
  int 	resourceId;
  int 	bucketNum;
  float data;
};

$remote struct visi_matrix {
    resourceType res;
    metricType	 met;
};

$remote struct phase_info{
  double start;
  double end;
  double  bucketWidth;
  int handle;
  string name;
};

$remote visualization {
  $base  7000;
  $version  9;

  // data: list of new data values from Paradyn 
  $async void Data(vector<dataValue> data);  

  // a fold event from paradyn
  $async void Fold(double newBucketWidth);

  //
  // A metric/focus pair has become invalid (from Paradyn)
  // m: metricId of invalid metric-focus combination
  // r: resourceId of invalid metric-focus combination
  //
  $async void InvalidMR(int m,int r);

  //
  // A new set of metrics and resources from Paradyn
  // newElements: a list of new metric-focus matrices
  // bucketWidth: current bucket width for histograms
  // nobuckets:   max number of buckets for histograms
  $async void AddMetricsResources(vector<visi_matrix> newElements,
				  double bucketWidth,
				  int nobuckets);
  //
  // A new phase has been defined by paradyn
  //
  $async void PhaseStart(double begin,  // phase start time
		    double end,		// -1 means phase has not ended yet
		    double bucketWidth,  // for histogram data values
		    string name,	// string name for phase 
		    int handle);	// unique phase id 

  //
  // A phase specified by "handle" has ended
  // currently this is just used to fill in the end value in the phase table
  //
  $async void PhaseEnd(double end,
		       int handle);


  //
  // A list of phase data from paradyn
  //
  $async void PhaseData(vector<phase_info> phases);


  //
  // An array of data values from Paradyn starting with bucket #0
  //
  $async void BulkDataTransfer(vector<float> values, // array of data values 
			       int metricId,	  // id of met. assoc. w/data
			       int resourceId);   // id of res. assoc. w/data

  //
  // A request from the visualization to Paradyn for new data
  //
  $upcall $async void GetMetricResource(string mets_res,
					int numElements,
					int type);
  //
  // A request to Paradyn to stop data associated with metricId & resourceId
  //
  $upcall $async void StopMetricResource(int metricId,
					 int resourceId);

  // request a new phase start from paradyn 
  // begin: phase start time (default of -1 means now)
  // name:  phase name (default of NULL means let paradyn provide name)
  $upcall $async void StartPhase(double begin, 
				string name);
};
