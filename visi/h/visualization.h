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

#if defined(__cplusplus)
extern "C" {
#endif

#include "visiTypes.h"
/////////////////////////////////////////////////////////////
// these functions invoke upcalls to a visi interface client
// (call from visualization process to paradyn)
/////////////////////////////////////////////////////////////
//
// get a new set of metrics and resources from Paradyn
// currently, passing a list of met-res pairs is not supported 
// (i.e. these parameters are ignored)
//
extern void visi_GetMetsRes(char *metres,  // predefined list met-res pairs
		       	    int numElements); 

//
// stop data collection for a metric/resource pair
// arguments are the datagrid indicies associated with the
// metric and resource to stop
//
extern void visi_StopMetRes(int metricIndex,int resourceIndex); 

//
// define a new phase to paradyn, can specify a name for the phase
// passing 0 for name argument will cause visiLib to create a 
// unique name for the phase
//
extern void visi_DefinePhase(char *name); // name of phase 

//
// request to Paradyn to display error message
//
extern void visi_showErrorVisiCallback(const char *msg);

////////////////////////////////////////////////////////////////
//   initialization routines 
////////////////////////////////////////////////////////////////
//
// Visi Initialization routine, returns a file descriptor that  
// is used to communicate with Paradyn.  This file descriptor 
// should be added by the visi as a source of input with the
// visiLib routine "visi_callback" as the callback routine 
// associated with this file descriptor.
// This routine should be called before entering the visualization's
// main loop, and before calling any other visi-interface routines
//
extern int  visi_Init();

//
// callback associated with paradyn-visualization interface routines
//
extern int visi_callback();

//
// registration callback routine for paradyn events
// sets eventCallbacks[event] to callback routine provided by user
// returns -1 on error, otherwise returns 0 
//
extern int visi_RegistrationCallback(visi_msgTag event,int (*callBack)(int));

//
// cleans up visi interface data structs
// Visualizations should call this routine before exiting 
extern void visi_QuitVisi();

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
extern int visi_GetMyPhaseHandle();

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
extern const visi_sampleType *visi_DataValues(int metric_num, int resource_num);

//
//  returns 1 if the data grid cell corresponding to metric_num   
//  and resource_num contains data, otherwise returns 0 
//
extern int visi_Valid(int metric_num, int resource_num);

//
//  returns 1 if the data collection has been enabled for metric_num   
//  and resource_num, otherwise returns 0  
//
extern int visi_Enabled(int metric_num, int resource_num);


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
// returns 1 if there are invalid spans of data between the first
// valid bucket and the last bucket filled, otherwise returns 0
//
extern int visi_InvalidSpans(int metric_num,int resource_num);

//
// returns the user data associated with metric_num and resource_num
// returns 0 on error
//
extern void *visi_GetUserData(int metric_num, int resource_num);

//
// sets the user data associated with metric_num and resource_num
// returns 1 on success, or returns 0 on error
//
extern int visi_SetUserData(int metric_num, int resource_num, void *data);

#if defined(__cplusplus)
};
#endif /* defined(__cplusplus) */

#endif
