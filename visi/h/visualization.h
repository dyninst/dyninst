/*
 * Copyright (c) 1996 Barton P. Miller
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

#ifndef _visualization_h
#define _visualization_h

/**********************************************************************
 *  This file should be included in all visualizations. It 
 *  contains definitions for all the Paradyn visi interface routines.  
 * 
 *  This file should not contain any C++ code (nor C++-style comments!). 
 *  It is intended to be used by either C or C++ visis, and therefore 
 *  it should be able to be compiled by a C compiler.
 **********************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

#include "visiTypes.h"
/***********************************************************************
 * these functions invoke upcalls to a visi interface client
 * (call from visualization process to paradyn)
 ***********************************************************************/

/* get a new set of metrics and resources from Paradyn
 * currently, passing a list of met-res pairs is not supported 
 * (i.e. these parameters are ignored)
 */
extern void visi_GetMetsRes(char *metres,  /* predefined met-res pair list */
		       	    int numElements); 

/* stop data collection for a metric/resource pair
 * arguments are the datagrid indicies associated with the
 * metric and resource to stop
 */
extern void visi_StopMetRes(int metricIndex, int resourceIndex); 

/* define a new phase to paradyn, can specify a name for the phase:
 * passing 0 for name argument will cause visiLib to create a 
 * unique name for the phase,
 * passing 1 for withPerfConsult or withVisis will cause a new
 * new PerfConsult search or visi to be started with the new phase, 
 * passing 0 will just start a new phase.
 */
extern void visi_DefinePhase(char *phase_name,
			     u_int withPerfConsult,
			     u_int withVisis);

/* request to Paradyn to display error message
 */
extern void visi_showErrorVisiCallback(const char *msg);

/************************************************************************
 *   initialization routines 
 ************************************************************************/

/* Visi Initialization routine, returns a file descriptor that  
 * is used to communicate with Paradyn.  This file descriptor 
 * should be added by the visi as a source of input with the
 * visiLib routine "visi_callback" as the callback routine 
 * associated with this file descriptor.
 * This routine should be called before entering the visualization's
 * main loop, and before calling any other visi-interface routines.
 */
extern PDSOCKET visi_Init(void);

/* callback associated with paradyn-visualization interface routines
 * Call this function when data is available on the PDSOCKET returned
 * from visi_Init.  Note that this function will consume all XDR records
 * available on the socket before returning to the caller.
 */
extern int visi_callback(void);

/* registration callback routine for Paradyn events
 * sets eventCallbacks[event] to callback routine provided by user
 * returns -1 on error, otherwise returns 0 
 */
extern int visi_RegistrationCallback(visi_msgTag event, int (*callBack)(int));

/* clean up visi interface data structs:
 * Visualizations should call this routine before exiting 
 */
extern void visi_QuitVisi(void);

/*******************************************************************
 *  Data Grid Routines
 *******************************************************************/

/* returns the ith metric name or 0 on error   
 */
extern const char *visi_MetricName(int metric_num);

/* returns the ith metric units name or 0 on error   
 */
extern const char *visi_MetricUnits(int metric_num);

/* returns the ith metric units label for data values or 0 on error   
 */
extern const char *visi_MetricLabel(int metric_num);

/* returns the ith metric units label for average aggregate data values,
 * or 0 on error   
 */
extern const char *visi_MetricAveLabel(int metric_num);

/* returns the ith metric units label for sum aggregate data values,
 * or 0 on error   
 */
extern const char *visi_MetricSumLabel(int metric_num);

/* returns the ith resource's name,  or 0 on error   
 */
extern const char *visi_ResourceName(int resource_num);

/* returns the number of metrics in the data grid
 */
extern int visi_NumMetrics(void);

/* returns the number of resources in the data grid
 */
extern int visi_NumResources(void);

/* returns the number of phases currently defined in the system   
 */
extern u_int visi_NumPhases(void);

/* returns the start time of the phase for which this visi is defined
 */
extern visi_timeType visi_GetStartTime(void);

/* returns the name of the phase for which this visi is defined
 */
extern const char *visi_GetMyPhaseName(void);

/* returns the handle of the phase for which this visi is defined or
 * -1 on error
 */
extern int visi_GetMyPhaseHandle(void);

/* returns the handle of the ith phase or -1 on error
 */
extern int visi_GetPhaseHandle(u_int phase_num);

/* returns phase name for the ith phase, or returns 0 on error
 */
extern const char *visi_GetPhaseName(u_int phase_num);

/* returns phase start time for the ith phase, or returns -1.0 on error
 */
extern visi_timeType visi_GetPhaseStartTime(u_int phase_num);

/* returns phase end time for the ith phase, or returns -1.0 on error
 */
extern visi_timeType visi_GetPhaseEndTime(u_int phase_num);

/* returns phase bucket width for the ith phase, or returns -1.0 on error
 */
extern visi_timeType visi_GetPhaseBucketWidth(u_int phase_num);

/* returns the average of all the data bucket values for the metric/resource
 * pair "metric_num" and "resource_num", returns NaN value on error 
 */
extern visi_sampleType visi_AverageValue(int metric_num, int resource_num);

/* returns the sum of all the data bucket values for the metric/resource
 * pair "metric_num" and "resource_num", returns NaN value on error 
 */
extern visi_sampleType visi_SumValue(int metric_num, int resource_num);

/* returns the data value in bucket "bucket_num" for the metric/resource pair 
 * "metric_num" and "resource_num", returns NaN value on error 
 */
extern visi_sampleType visi_DataValue(int metric_num, int resource_num, 
				      int bucket_num);

/* returns the data values for the metric/resource pair "metric_num" 
 * and "resource_num", returns NaN value on error 
 */
extern int visi_DataValues(int metric_num, int resource_num, 
			   visi_sampleType *samples, int firstBucket, 
			   int lastBucket);

/* returns 1 if the data grid cell corresponding to metric_num   
 * and resource_num contains data, otherwise returns 0 
 */
extern int visi_Valid(int metric_num, int resource_num);

/* returns 1 if the data collection has been enabled for metric_num   
 * and resource_num, otherwise returns 0  
 */
extern int visi_Enabled(int metric_num, int resource_num);

/* returns the number of buckets in each data grid cell's histogram  
 */
extern int visi_NumBuckets(void);

/* returns the bucket width (in seconds) of each data grid cell's histogram  
 */
extern visi_timeType visi_BucketWidth(void);

/* returns the first data bucket with valid data values 
 */
extern int visi_FirstValidBucket(int metric_num, int resource_num);

/* returns the last data bucket with valid data values 
 */
extern int visi_LastBucketFilled(int metric_num,int resource_num);

/* returns 1 if there are invalid spans of data between the first
 * valid bucket and the last bucket filled, otherwise returns 0
 */
extern int visi_InvalidSpans(int metric_num,int resource_num);

/* returns the user data associated with metric_num and resource_num
 * returns 0 on error
 */
extern void *visi_GetUserData(int metric_num, int resource_num);

/* sets the user data associated with metric_num and resource_num
 * returns 1 on success, or returns 0 on error
 */
extern int visi_SetUserData(int metric_num, int resource_num, void *data);

/* print every stepth bucket to stderr
 */
extern void visi_PrintDataBuckets(int step);

#if defined(__cplusplus)
};
#endif /* defined(__cplusplus) */

#endif
