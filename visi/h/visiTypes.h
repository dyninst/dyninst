#ifndef _visiTypes_h
#define _visiTypes_h
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
#include <stdio.h>

// 
// note: add new event types before FOLD (ie. FOLD is always last type)
// event types associated with events from Paradyn to a visualization
// DATAVALUES:  a new set of data has arrived in the datagrid
// INVALIDMETRICSRESOURCES:  a metric resource combination has become invalid
// ADDMETRICSRESOURCES:  new metrics have become enabled for a resource
// PHASESTART:  a new phase has been defined
// PHASEEND:    a phase has ended
// PHASEDATA:  data about the current set of phases has arrived
// PARADYNEXITED: the paradyn process has exited
// FOLD:  the histogram has folded; binWidth has doubled
//
typedef enum {DATAVALUES,INVALIDMETRICSRESOURCES,ADDMETRICSRESOURCES,
	      PHASESTART,PHASEEND,PHASEDATA,PARADYNEXITED,FOLD} visi_msgTag;

typedef float visi_sampleType;
typedef double visi_timeType;
typedef enum {UnNormalized, Normalized, Sampled} visi_unitsType;

struct metricStruct {
     const char *units;    // how units are measured
     const char *name;     // metric name for graph labeling  
     unsigned  Id;       // unique metric Id
     int    aggregate;  //either SUM or AVE
     visi_unitsType    unitstype;  // specifies units type
};
typedef struct metricStruct visi_metricType;


struct resourceStruct{
     const char *name;     // resource name for graph labeling
     unsigned  Id;       // unique resource id
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
