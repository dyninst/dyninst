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
 * PCfilter.h
 *
 * Data filter class performs initial processing of raw DM data arriving 
 * in the Performance Consultant.  
 *
 * $Log: PCfilter.h,v $
 * Revision 1.6  1996/05/06 04:35:10  karavan
 * Bug fix for asynchronous predicted cost changes.
 *
 * added new function find() to template classes dictionary_hash and
 * dictionary_lite.
 *
 * changed filteredDataServer::DataFilters to dictionary_lite
 *
 * changed normalized hypotheses to use activeProcesses:cf rather than
 * activeProcesses:tlf
 *
 * code cleanup
 *
 * Revision 1.5  1996/05/02 19:46:35  karavan
 * changed predicted data cost to be fully asynchronous within the pc.
 *
 * added predicted cost server which caches predicted cost values, minimizing
 * the number of calls to the data manager.
 *
 * added new batch version of ui->DAGconfigNode
 *
 * added hysteresis factor to cost threshold
 *
 * eliminated calls to dm->enable wherever possible
 *
 * general cleanup
 *
 * Revision 1.4  1996/04/30 06:26:51  karavan
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
 * Revision 1.3  1996/04/07 21:24:38  karavan
 * added phaseID parameter to dataMgr->enableDataCollection2.
 *
 * Revision 1.2  1996/02/22 18:30:05  karavan
 * bug fix to PC pause/resume so only filters active at time of pause
 * resubscribe to data
 *
 * Revision 1.1  1996/02/02 02:07:27  karavan
 * A baby Performance Consultant is born!
 *
 */

#ifndef pc_filter_h
#define pc_filter_h

#include <stream.h>
#include <assert.h>
//sys.h defines the following:
//  typedef double timeStamp;
//  typedef float sampleValue;
//  typedef struct {
//     timeStamp start;
//     timeStamp end;
//      sampleValue value;
//  } Interval;
#include "PCintern.h"
#include "PCdata.h"

//filter
//==========
//Receives and sends data in real-time

//#define MAX(a, b) ((a) > (b) ? (a):(b))

class filter;
class filteredDataServer;

class PCmetricInst;
typedef PCmetricInst* fdsSubscriber;
typedef metricInstanceHandle fdsDataID;
typedef resourceListHandle focus;

/* 
   The filter class serves as a base class for individual types of 
   filters, which differ in how data values are computed using the 
   raw  data which serves as input.

   Every filter receives data manager data as input, and sends output
   to its list of subscribers at defined time intervals.  The subscribers
   are managed by methods of the parent class, dataProvider, in data.C
   and data.h.
*/
class filter : public dataProvider
{
  friend ostream& operator <<(ostream &os, filter& f);
 public:
  filter(filteredDataServer *keeper,  
	 metricInstanceHandle mih, metricHandle met, focus focs,
	 bool costFlag);
  ~filter() { ; }  
  // all processing for a fresh hunk of raw data
  virtual void newData(sampleValue, timeStamp, timeStamp) = 0;
  //
  metricInstanceHandle getMI () {return mi;}
  metricHandle getMetric() {return metric;}
  focus getFocus() {return foc;}
  // active just means has at least one consumer; may not be 
  // instrumented i.e. if PC is paused
  bool pausable() {return (numConsumers > 0 && !costFlag);}
  void setcostFlag() {costFlag = true;}
 protected:  
  // these used in newData() to figure out intervals 
  void updateNextSendTime(timeStamp startTime);
  void getInitialSendTime(timeStamp startTime);
  // current length of a single interval
  timeStamp intervalLength;
  // when does the interval currently being collected end?
  timeStamp nextSendTime;
  // current time, according to this filter
  timeStamp lastDataSeen;   
  // when did the interval currently being collected start?
  timeStamp partialIntervalStartTime;
  // numerator of running average
  sampleValue workingValue; 
  // denominator of running average: sum of all time used in this 
  // average; time used may not start at 0; may not be contiguous
  timeStamp workingInterval;  
  // identifiers for this filter
  metricInstanceHandle mi;
  metricHandle metric;
  focus foc;
 private:
  // if set, this data is not disabled for a pause because it is used in 
  // cost observation by all phases
  bool costFlag;
  // collector for these filters
  filteredDataServer *server;          
};

ostream& operator <<(ostream &os, filter& f);

/* an avgFilter computes values based on the running average over all 
   data received from first enable of its metric instance.  The current
   running average is sent to each subscriber at the end of each time 
   interval. 
*/
class avgFilter : public filter
{
public:
  avgFilter(filteredDataServer *keeper,  
	    metricInstanceHandle mih, metricHandle met, focus focs,
	    bool costFlag):
	      filter(keeper, mih, met, focs, costFlag) {;}
  ~avgFilter(){;}
  // all processing for a fresh hunk of raw data
  void newData(sampleValue newVal, timeStamp start, timeStamp end);
};

/* a valFilter computes values based on the average over each time 
   interval.  The average for each time interval is sent to each 
   subscriber at the end of that interval.
*/
class valFilter : public filter
{
public:
  valFilter(filteredDataServer *keeper,  
	 metricInstanceHandle mih, metricHandle met, focus focs,
	 bool costFlag):
	   filter(keeper, mih, met, focs, costFlag) {;}
  ~valFilter() {;}
  // all processing for a fresh hunk of raw data
  void newData(sampleValue newVal, timeStamp start, timeStamp end);
};

class filterIndex;
class mfpair {
  friend class filterIndex;
 private:
  metricInstanceHandle m;
  filter *f;
};

//**
const unsigned NumMetrics = 25;
class filteredDataServer;
class ff {
  friend class filteredDataServer;
private:
  focus f;
  fdsDataID mih;
};

// contains variable number of filters; maintains subscribers to each
// filter; new subscription -> dm->enable() request and add filter; 
// end subscription -> if numSubscribers == 0, dm->disable request.
// 
class filteredDataServer
{
  friend class filter;
public:
  filteredDataServer(unsigned phID);
  ~filteredDataServer();
  // interface to subscribers (provider role)
  fdsDataID addSubscription(fdsSubscriber sub,
			    metricHandle mh,
			    focus f,
			    filterType ft,
			    bool *errFlag);
  void endSubscription(fdsSubscriber sub, fdsDataID subID);
  void unsubscribeAllData();
  void resubscribeAllData();
  // interface to raw data source (consumer role)
  void newBinSize(timeStamp newSize);
  void newData(metricInstanceHandle mih, sampleValue value, int bin);
  // 
  timeStamp getCurrentBinSize () {return currentBinSize;}
 private:
  static unsigned fdid_hash (fdsDataID& val) {return (unsigned)val % 19;} 
  // size of dm histogram bucket; used to convert data bin number into interval
  timeStamp currentBinSize;  
  // current size of each interval of output
  timeStamp intervalSize;
  // used by filters to align send times across filters
  timeStamp nextSendTime;
  phaseType phType;
  unsigned dmPhaseID;
  // starting interval size we never go below this
  timeStamp minGranularity;
  dictionary_lite<fdsDataID, filter*>DataFilters;
  vector<ff> miIndex [NumMetrics];
  vector<filter*> AllDataFilters;
};

#endif
