/*
 * Copyright (c) 1996-1998 Barton P. Miller
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

/*
 * $Id: PCfilter.h,v 1.19 2003/07/18 15:44:28 schendel Exp $
 * Data filter class performs initial processing of raw DM data arriving 
 * in the Performance Consultant.  
 */

#ifndef pc_filter_h
#define pc_filter_h

#include <iostream>
#include <assert.h>
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
  friend class filteredDataServer;
  enum FilterStatus { Active, ActivationRequestPending, Inactive };
 public:
  filter(filteredDataServer *keeper,  
	 metricHandle met, focus focs,
	 bool costFlag);
  ~filter() { ; }  
  // all processing for a fresh hunk of raw data
  virtual void newData(pdRate, relTimeStamp, relTimeStamp) = 0;
  //
  metricInstanceHandle getMI () {return mi;}
  metricHandle getMetric() {return metric;}
  focus getFocus() {return foc;}
  bool isActive() {return status == Active;}
  bool isPending() {return status == ActivationRequestPending;}
  // true means we want to disable this as part of a PC pause event
  bool pausable() {return (numConsumers > 0 && !costFlag);}
  void setCurActualValue(pdSample v) {  curActualVal = v; }
  void incCurActualValue(pdSample d) {  curActualVal += d; }
  pdSample getCurActualValue() { return curActualVal; }
  void setcostFlag() {costFlag = true;}
 protected:  
  // activate filter by enabling met/focus pair
  void wakeUp();
  void inactivate() {status = Inactive;}
  // these used in newData() to figure out intervals 
  void updateNextSendTime(relTimeStamp startTime);
  void getInitialSendTime(relTimeStamp startTime);
  // (re)enable data for an existing filter
  void activate();
  // current length of a single interval
  timeLength intervalLength;
  // when does the interval currently being collected end?
  relTimeStamp nextSendTime;
  // current time, according to this filter
  relTimeStamp lastDataSeen;   
  // when did the interval currently being collected start?
  relTimeStamp partialIntervalStartTime;
  // numerator of running average
  pdSample workingValue; 
  // denominator of running average: sum of all time used in this 
  // average; time used may not start at 0; may not be contiguous
  timeLength workingInterval;  
  // the current actual value for this filter; used by PC when using
  // metrics that require using the actual value like num_cpus, etc.
  // initialized to initialActualValue as passed from the performance
  // stream
  pdSample curActualVal;
  // identifiers for this filter
  metricInstanceHandle mi;
  metricHandle metric;
  focus foc;
 private:
  FilterStatus status;
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
	    metricHandle met, focus focs,
	    bool costFlag):
	      filter(keeper, met, focs, costFlag) {;}
  ~avgFilter(){;}
  // all processing for a fresh hunk of raw data
  void newData(pdRate newVal, relTimeStamp start, relTimeStamp end);
};

/* a valFilter computes values based on the average over each time 
   interval.  The average for each time interval is sent to each 
   subscriber at the end of that interval.
*/
class valFilter : public filter
{
public:
  valFilter(filteredDataServer *keeper,  
	 metricHandle met, focus focs,
	 bool costFlag):
	   filter(keeper, met, focs, costFlag) {;}
  ~valFilter() {;}
  // all processing for a fresh hunk of raw data
  void newData(pdRate newVal, relTimeStamp start, relTimeStamp end);
};

class filteredDataServer;

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
  void addSubscription(fdsSubscriber sub,
			    metricHandle mh,
			    focus f,
			    filterType ft,
			    bool costFlag);
  void endSubscription(fdsSubscriber sub, fdsDataID subID);
  void endSubscription(fdsSubscriber sub, metricHandle met, focus foc);
  // cancel pending enable request for this met/foc pair
  void cancelSubRequest (fdsSubscriber sub, metricHandle met, focus foc);
  void unsubscribeAllData();

  // interface to raw data source (consumer role)
  void newBinSize(timeLength newSize);
  void newData (metricInstanceHandle mih, pdSample deltaVal, 
		int bucketNumber, phaseType ptype);
  void newDataEnabled(pdvector<metricInstInfo>* newlyEnabled);

  // miscellaneous  
  timeLength getCurrentBinSize () {return currentBinSize;}
  void setCurActualValue(metricInstanceHandle mih, pdSample v);
 private:
  void printPendings(); 
  filter *findFilter(metricHandle mh, focus f);
  static unsigned fdid_hash (const fdsDataID& val) {return (unsigned)val % 19;} 
  void inActivateFilter (filter *fil);
  void makeEnableDataRequest (metricHandle met, focus foc);
  unsigned getPCphaseID () {
    if (phType == CurrentPhase) return dmPhaseID+1;
    else return 0;
  }
  // size of dm histogram bucket; used to convert data bin number into interval
  timeLength currentBinSize;  
  // current size of each interval of output
  timeLength intervalSize;
  // used by filters to align send times across filters
  relTimeStamp nextSendTime;
  phaseType phType;
  unsigned dmPhaseID;
  // starting interval size we never go below this
  timeLength minGranularity;

  // DataFilters  contain all filters which are now successfully enabled.  
  dictionary_hash<fdsDataID, filter*>DataFilters;

  // miIndex and AllDataFilters contain every filter ever created for this server
  dictionary_hash<focus, filter*> **miIndex;
  pdvector<filter*> AllDataFilters;
};

#endif
