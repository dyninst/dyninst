/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

#ifndef _visiTypes_h
#define _visiTypes_h

#include <stdio.h>

/* This file should not contain any C++ code, comments or class definitions.  
 * It is intended for use by either C or C++ visis.
 * 
 * Note: add new event types before FOLD (ie. FOLD is always last type)
 * event types associated with events from Paradyn to a visualization
 *   DATAVALUES: a new set of data has arrived in the datagrid
 *   TRACEDATAVALUES: a new set of trace data has arrived
 *   INVALIDMETRICSRESOURCES: a metric resource combination has become invalid
 *   ADDMETRICSRESOURCES: new metrics have become enabled for a resource
 *   PHASESTART: a new phase has been defined
 *   PHASEEND: a phase has ended
 *   PHASEDATA: data about the current set of phases has arrived
 *   PARADYNEXITED: the paradyn process has exited
 *   FOLD: the histogram has folded; binWidth has doubled
 */
typedef enum {DATAVALUES,TRACEDATAVALUES,
              INVALIDMETRICSRESOURCES,ADDMETRICSRESOURCES,
	      PHASESTART,PHASEEND,PHASEDATA,PARADYNEXITED,FOLD} visi_msgTag;

typedef float visi_sampleType;
typedef double visi_timeType;
typedef enum {UnNormalized, Normalized, Sampled} visi_unitsType;

struct metricStruct {
  const char *curr_units;   /* how units are measured */
  const char *tot_units;    /* how total units are measured */
  const char *name;         /* metric name for graph labeling */
  unsigned  Id;             /* unique metric Id */
  int aggregate;            /* either SUM or AVE */
  visi_unitsType unitstype; /* specifies units type */
};
typedef struct metricStruct visi_metricType;


struct resourceStruct{
     const char *name;                  /* resource name for graph labeling */
     unsigned  Id;                      /* unique resource identifier */
};
typedef struct resourceStruct visi_resourceType;


struct dataValueStruct{
     unsigned metricId;
     unsigned resourceId;
     int   bucketNum;
     visi_sampleType data;
};
typedef struct dataValueStruct visi_dataValue;

#endif
