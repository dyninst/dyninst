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

class filter : public dataProvider
{
  friend ostream& operator <<(ostream &os, filter& f);
 public:
  filter(filteredDataServer *keeper,  
	 metricInstanceHandle mih, metricHandle met, focus focs);
  ~filter();
  // all processing for a fresh hunk of raw data
  void newData(sampleValue newVal, timeStamp start, timeStamp end);
  //
  metricInstanceHandle getMI () {return mi;}
  metricHandle getMetric() {return metric;}
  focus getFocus() {return foc;}
  float getNewEstimatedCost();
  // active just means has at least one consumer; may not be 
  // instrumented i.e. if PC is paused
  bool isActive() {return (numConsumers > 0);}
 private:  
  // these used in newData() to figure out intervals 
  void updateNextSendTime(timeStamp startTime);
  void getInitialSendTime(timeStamp startTime);
  // send out new complete interval data to all subscribers
  //void sendValue(sampleValue runAvg, timeStamp start, timeStamp end);
  // identifiers for this filter
  metricInstanceHandle mi;
  metricHandle metric;
  focus foc;
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
  float estimatedCost;    
  // collector for these filters
  filteredDataServer *server;          
};

ostream& operator <<(ostream &os, filter& f);

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
			    bool *errFlag);
  void endSubscription(fdsSubscriber sub, fdsDataID subID);
  void unsubscribeAllData();
  void resubscribeAllData();
  float getEstimatedCost(metricHandle mh, focus f);
  float getEstimatedCost(fdsDataID mih);
  // interface to raw data source (consumer role)
  static void initPStoken(perfStreamHandle psh) {pstream = psh;}
  void newBinSize(timeStamp newSize);
  void newData(metricInstanceHandle mih, sampleValue value, int bin);
  // 
  timeStamp getCurrentBinSize () {return currentBinSize;}
 private:
  static unsigned fdid_hash (fdsDataID& val) {return (unsigned)val;} 
  // size of dm histogram bucket; used to convert data bin number into interval
  timeStamp currentBinSize;  
  // current size of each interval of output
  timeStamp intervalSize;
  // used by filters to align send times across filters
  timeStamp nextSendTime;
  static perfStreamHandle pstream;
  phaseType phType;
  unsigned dmPhaseID;
  // starting interval size we never go below this
  timeStamp minGranularity;
  dictionary_hash<fdsDataID, filter*>DataFilters;
  dictionary_hash<metricHandle, dictionary_hash<focus, filter*>*> DataByMetFocus;
  vector<filter*> AllDataFilters;
};

#endif
