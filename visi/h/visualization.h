#ifndef _visualization_h
#define _visualization_h
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
/////////////////////////////////////////////////////////
//  This file should be included in all visualizations.
//  It contains definitions for all the Paradyn visi
//  interface routines.  
/////////////////////////////////////////////////////////
#include "visiTypes.h"

//
// callback associated with paradyn-visualization interface routines
//
extern int visi_callback();

/////////////////////////////////////////////////////////////
// these functions invoke upcalls to a visi interface client
// (call from visualization process to paradyn)
/////////////////////////////////////////////////////////////
//
// get a new set of metrics and resources from Paradyn
//
extern void visi_GetMetsRes(char *metres,  // predefined list met-res pairs 
		       int numElements,  
		       int type);      // 0-histogram, 1-scalar

//
// equivalent to a call to GetMetRes(0,0,0)
//
extern void visi_GetMetsRes();

//
// stop data collection for a metric/resource pair
// arguments are the datagrid indicies associated with the
// metric and resource to stop
//
extern void visi_StopMetRes(int metricIndex,    // datagrid index of metric
		       int resourceIndex); // datagrid index of resource

//
// define a new phase to paradyn, can specify some time in the future
// for the phase to begin or a begin value of -1 means now
//
extern void visi_DefinePhase(visi_timeType begin,  // in seconds 
		        char *name);     // name of phase 

////////////////////////////////////////////////////////////////
//   initialization routines 
////////////////////////////////////////////////////////////////
//
// connects to parent socket and registers the mainLoop routine
// as a callback on events on fileDesc[0]
// This routine should be called before entering the visualization's
// main loop, and before calling any other visi-interface routines
//
extern int  visi_Init();


//
//  Makes initial call to get Metrics and Resources.
//  For visualizations that do not provide an event that
//  invokes the GetMetsRes upcall this routine should be
//  called by the visualization before entrering the mainloop.
//  (this will be the only chance to get metrics and resources
//  for the visualization).
//  For other visualizaitons, calling this routine before
//  entering the mainloop is optional. 
//
extern int  visi_StartVisi(int argc,char *argv[]);

//
// cleans up visi interface data structs
// Visualizations should call this routine before exiting 
extern void visi_QuitVisi();


//
// registration callback routine for paradyn events
// sets eventCallbacks[event] to callback routine provided by user
//
extern int visi_RegistrationCallback(visi_msgTag event,int (*callBack)(int));

//
// request to Paradyn to display error message
//
extern void visi_showErrorVisiCallback(const char *msg);

// **********************************************
//
//  Data Grid Routines
//
// **********************************************
//
// returns the ith metric name or 0 on error   
//
extern const char *visi_MetricName(int metric_num);

//
// returns the ith metric units name or 0 on error   
//
extern const char *visi_MetricUnits(int metric_num);

//
// returns the ith metric units label for data values or 0 on error   
//
extern const char *visi_MetricLabel(int metric_num);

//
// returns the ith metric units label for average aggregate data values,
// or 0 on error   
//
extern const char *visi_MetricAveLabel(int metric_num);

//
// returns the ith metric units label for sum aggregate data values,
// or 0 on error   
//
extern const char *visi_MetricSumLabel(int metric_num);

//
// returns the ith resource's name,  or 0 on error   
//
extern const char *visi_ResourceName(int resource_num);

//
//  returns the number of metrics in the data grid
//
extern int visi_NumMetrics();

//
//  returns the number of resources in the data grid
//
extern int visi_NumResources();

//
//  returns the number of phases currently defined in the system   
//
extern u_int visi_NumPhases();

//
// returns the start time of the phase for which this visi is defined
//
extern visi_timeType visi_GetStartTime();

//
// returns the name of the phase for which this visi is defined
//
extern const char *visi_GetMyPhaseName();

//
// returns the handle of the phase for which this visi is defined or
// -1 on error
//
extern int visi_GetPhaseHandle();

//
// returns the handle of the ith phase or -1 on error
//
extern int visi_GetPhaseHandle(u_int phase_num);

//
// returns phase name for the ith phase, or returns 0 on error
//
extern const char *visi_GetPhaseName(u_int phase_num);

//
// returns phase start time for the ith phase, or returns -1.0 on error
//
extern visi_timeType visi_GetPhaseStartTime(u_int phase_num);

//
// returns phase end time for the ith phase, or returns -1.0 on error
//
extern visi_timeType visi_GetPhaseEndTime(u_int phase_num);

//
// returns phase bucket width for the ith phase, or returns -1.0 on error
//
extern visi_timeType visi_GetPhaseBucketWidth(u_int phase_num);

//
// returns the average of all the data bucket values for the metric/resource
// pair "metric_num" and "resource_num", returns NaN value on error 
//
extern visi_sampleType visi_AverageValue(int metric_num, int resource_num);

//
// returns the sum of all the data bucket values for the metric/resource
// pair "metric_num" and "resource_num", returns NaN value on error 
//
extern visi_sampleType visi_SumValue(int metric_num, int resource_num);

//
// returns the data value in bucket "bucket_num" for the metric/resource pair 
// "metric_num" and "resource_num", returns NaN value on error 
//
extern visi_sampleType visi_DataValue(int metric_num, int resource_num, 
				 int bucket_num);

//
// returns the data values for the metric/resource pair "metric_num" 
// and "resource_num", returns NaN value on error 
//
extern visi_sampleType *visi_DataValues(int metric_num, int resource_num);

//
//  returns true if the data grid cell corresponding to metric_num   
//  and resource_num contains data 
//
extern bool visi_Valid(int metric_num, int resource_num);

//
//  returns true if the data collection has been enabled for metric_num   
//  and resource_num  
//
extern bool visi_Enabled(int metric_num, int resource_num);


//
//  returns the number of buckets in each data grid cell's histogram  
//
extern int visi_NumBuckets();

//
//  returns the buckets width (in seconds) of each data grid cell's histogram  
//
extern visi_timeType visi_BucketWidth();

//
// returns the first data bucket with valid data values 
//
extern int visi_FirstValidBucket(int metric_num, int resource_num);

//
// returns the last data bucket with valid data values 
//
extern int visi_LastBucketFilled(int metric_num,int resource_num);

//
// returns true if there are invalid spans of data between the first
// valid bucket and the last bucket filled
//
extern bool visi_InvalidSpans(int metric_num,int resource_num);

//
// returns the user data associated with metric_num and resource_num
// returns 0 on error
//
extern void *visi_GetUserData(int metric_num, int resource_num);

//
// sets the user data associated with metric_num and resource_num
//
extern bool visi_SetUserData(int metric_num, int resource_num, void *data);

#endif
