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

/*
 * PCmetric.h
 * 
 * The PCmetric class and the PCmetricInst class.
 * 
 * $Log: PCmetric.h,v $
 * Revision 1.21  1996/04/30 06:26:59  karavan
 * change PC pause function so cost-related metric instances aren't disabled
 * if another phase is running.
 *
 * fixed bug in search node activation code.
 *
 * added change to treat activeProcesses metric differently in all PCmetrics
 * in which it is used; checks for refinement along process hierarchy and
 * if there is one, uses value "1" instead of enabling activeProcesses metric.
 *
 * changed costTracker:  we now use min of active Processes and number of
 * cpus, instead of just number of cpus; also now we average only across
 * time intervals rather than cumulative average.
 *
 * Revision 1.20  1996/02/02 02:07:30  karavan
 * A baby Performance Consultant is born!
 *
 */

#ifndef pc_metric_h
#define pc_metric_h

#include "PCintern.h"
#include "PCfilter.h"
//sys.h defines the following:
//  typedef double timeStamp;
//  typedef float sampleValue;
//  typedef struct {
//     timeStamp start;
//     timeStamp end;
//      sampleValue value;
//  } Interval;

typedef enum PCmetricType {derived, simple};
typedef metricInstanceHandle PCmetDataID;
typedef bool (*initPCmetricInstFunc)(focus foo);
typedef sampleValue (*evalPCmetricInstFunc)(focus foo, sampleValue *data,
					    int dataSize); 
typedef enum focusType {cf, tlf};
typedef struct {
  const char *mname;
  focusType whichFocus;
  filterType ft;
} metNameFocusStruct;
typedef struct metNameFocusStruct metNameFocus;

typedef struct {
  metricHandle mh;
  focusType fType;
  filterType ft;
} PCMetInfoStruct;
typedef struct PCMetInfoStruct PCMetInfo;

class PCmetric;

class PCmetric {
  friend class PCmetricInst;
public:
  static unsigned pauseTimeMetricHandle;
  // (future) user-defined derived metric
  PCmetric (const char *thisName, metNameFocus *dataSpecs, int dataSpecsSize, 
	    initPCmetricInstFunc setupFun, evalPCmetricInstFunc calcFun,
	    bool withPause);
  // wrapper for dm metrics
  PCmetric (char *DMmetName, focusType ftype, bool *success); 
  const char *getName() {return metName.string_of();}
  static dictionary_hash<string, PCmetric*> AllPCmetrics;
  metricHandle getMh () {return mh;}
private:
  string metName;
  bool InstWithPause;
  vector <PCMetInfo*> *DMmetrics; 
  metricHandle mh;
  focusType ft;
  initPCmetricInstFunc setup;
  evalPCmetricInstFunc calc;
};


#endif


