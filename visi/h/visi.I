
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
/* Revision 1.8  1994/09/25 01:58:13  newhall
/* changed interface definitions to work for new version of igen
/* changed AddMetricsResources def. to take array of metric/focus pairs
/*
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


$remote typedef struct {
     char *units;    // how units are measured  i.e. "ms" 
     char *name;     // for y-axis labeling  
     int     Id;       // unique metric Id
     int     aggregate;  //either SUM or AVE
} metricType;

$remote typedef struct{
     char *name;     // obj. name for graph labeling
     int    Id;       // unique resource id
} resourceType;

$remote typedef struct{
  int 	metricId;
  int 	resourceId;
  int 	bucketNum;
  float data;
}dataValue;

$remote typedef struct {
    resourceType res;
    metricType	 met;
}visi_matrix;


$remote array visi_matrix  visi_matrix_Array;
$remote array dataValue    dataValue_Array;
$remote array float	    float_Array;

$remote visualization {
  $base  7000;
  $version  7;

  // data: list of new data values from Paradyn 
  $async void Data(dataValue_Array data);  

  $async void Fold(double newBucketWidth);

  // m: metricId of invalid metric-focus combination
  // r: resourceId of invalid metric-focus combination
  $async void InvalidMR(int m,int r);

  // newElements: a list of new metric-focus matrices
  // bucketWidth: current bucket width for histograms
  // nobuckets:   max number of buckets for histograms
  $async void AddMetricsResources(visi_matrix_Array newElements,
				  double bucketWidth,
				  int nobuckets);
  // begin: phase start time
  // end:   phase end time
  // name:  phase name
  $async void Phase(double begin,
		    double end,
		    char *name);

  // values: array of data values starting from 0th element
  // metricId:   id of metric assoc. with data values
  // resourceId: id of focus assoc. with data values
  $async void BulkDataTransfer(float_Array values,
			       int metricId,
			       int resourceId);


  $upcall $async void GetMetricResource(char *mets_res,
					int numElements,
					int type);

  // metricId, resourceId: id's of metric-focus combination to stop
  $upcall $async void StopMetricResource(int metricId,
					 int resourceId);

  // begin: phase start time
  // end:   phase end time
  // name:  phase name
  $upcall $async void PhaseName(double begin,
				double end,
				char *name);
};
