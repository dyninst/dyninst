
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
/* Revision 1.5  1994/08/03 20:47:28  newhall
/* removed interface routines Enabled and NewMetricsResources
/*
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
     String  units;    // how units are measured  i.e. "ms" 
     String  name;     // for y-axis labeling  
     int     Id;       // unique metric Id
     int     aggregate;  //either SUM or AVE
}metricType;


$remote typedef struct{
     String name;     // obj. name for graph labeling
     int    Id;       // unique resource id
}resourceType;

$remote typedef struct{
  int 	metricId;
  int 	resourceId;
  int 	bucketNum;
  float data;
}dataValue;


$remote visualization {
  $base  7000;
  $version  3;
  $async void Data($array dataValue data);  
  $async void Fold(double newBucketWidth);
  $async void InvalidMR(int m,int r);
  $async void AddMetricsResources($array metricType,
				$array resourceType,
				double,int);

  $async void Phase(double begin,
		    double end,
		    String name);

  $async void BulkDataTransfer($array float values,
			       int metricId,
			       int resourceId);

  $upcall $async void GetMetricResource(String metric,
					String resource,
					int type);

  $upcall $async void StopMetricResource(int metricId,
					 int resourceId);

  $upcall $async void PhaseName(double begin,
				double end,
				String name);
};
