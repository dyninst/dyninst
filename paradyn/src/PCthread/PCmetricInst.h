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
 * PCmetricInst.h
 * 
 * The PCmetricInst class and the PCmetricInstServer class.
 * 
 * $Log: PCmetricInst.h,v $
 * Revision 1.10  1996/07/22 18:55:44  karavan
 * part one of two-part commit for new PC functionality of restarting searches.
 *
 * Revision 1.9  1996/05/15 04:35:15  karavan
 * bug fixes: changed pendingCost pendingSearches and numexperiments to
 * break down by phase type, so starting a new current phase updates these
 * totals correctly; fixed error in estimated cost propagation.
 *
 * Revision 1.8  1996/05/08 07:35:19  karavan
 * Changed enable data calls to be fully asynchronous within the performance consultant.
 *
 * some changes to cost handling, with additional limit on number of outstanding enable requests.
 *
 * Revision 1.7  1996/05/06 04:35:18  karavan
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
 * Revision 1.6  1996/05/02 19:46:44  karavan
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
 * Revision 1.5  1996/04/30 06:27:02  karavan
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
 * Revision 1.4  1996/04/07 21:29:38  karavan
 * split up search ready queue into two, one global one current, and moved to
 * round robin queue removal.
 *
 * eliminated startSearch(), combined functionality into activateSearch().  All
 * search requests are for a specific phase id.
 *
 * changed dataMgr->enableDataCollection2 to take phaseID argument, with needed
 * changes internal to PC to track phaseID, to avoid enable requests being handled
 * for incorrect current phase.
 *
 * added update of display when phase ends, so all nodes changed to inactive display
 * style.
 *
 * Revision 1.3  1996/03/18 07:13:05  karavan
 * Switched over to cost model for controlling extent of search.
 *
 * Added new TC PCcollectInstrTimings.
 *
 * Revision 1.2  1996/02/22 18:32:01  karavan
 * Changed data storage from queue to circular buffer
 *
 * Revision 1.1  1996/02/02 02:07:32  karavan
 * A baby Performance Consultant is born!
 *
 */

#ifndef pc_metric_inst_h
#define pc_metric_inst_h

#include "PCintern.h"
#include "PCfilter.h"
#include "PCmetric.h"
#include "PCdata.h"
#include "util/h/CircularBuffer.h"

typedef circularBuffer<Interval, PCdataQSize> dataQ;
typedef PCmetricInst* PCmetInstHandle;
class experiment;
class PCmetricInst;

class inPort {
  friend class PCmetricInst;
  friend ostream& operator <<(ostream &os, PCmetricInst &pcm);
private:
  unsigned portID;
  metricInstanceHandle mih;
  metricHandle met;
  focus foc;
  filterType ft;
  // set true when enable reply received from the filter 
  // changed to false again when disable request is made
  bool active;
  dataQ indataQ;
 };

class PCmetricInst;

class PCmetricInst : public dataProvider, public dataSubscriber 
{
  friend ostream& operator <<(ostream &os, PCmetricInst &pcm);

public:
  PCmetricInst (PCmetric *pcmet, focus f,  
		filteredDataServer *db, bool *flag);
  ~PCmetricInst();    

  // these services part of dataProvider role
  sampleValue getCurrentValue () {return currentValue;}
  // this asynchronous call makes a request for cost data; cost server returns
  // result via a call to updateEstimatedCost() 
  float getEstimatedCost(dataSubscriber *sub);

  // at the current time, we simply try to subscribe to all data
  // future work: separate essential from nonessential metrics and t/f based on that.
  void activate();

  // end subscriptions to all filtered data for this PCmetricInst, 
  // but we don't kill it entirely, so we can always reactivate it by 
  // resubscribing to the data.
  void deactivate();

  // these are called by filtered data source and cost server -- subscriber role
  void newData(PCmetDataID whichData, sampleValue newVal, 
	       timeStamp start, timeStamp end, sampleValue);
  void updateEstimatedCost(float costDiff);
  void enableReply (unsigned token1, unsigned token2, unsigned token3,
		    bool successful);
  void addSubscription(dataSubscriber *sub) 
    { addConsumer(sub); activate(); }
  void endSubscription(dataSubscriber *sub); 
private:
  void sendInitialCostEstimate (float costEstimate);
  bool alignTimes();
  void setDataReady(int portNum);
  void clearDataReady (int portNum);
  void setEnableReady(int portNum);
  void clearEnableReady(int portNum);

  focus foc;
  PCmetric *met;
  sampleValue currentValue;
  float costEstimate;
  int numCostEstimates;
  timeStamp startTime;    // time first value calculated
  timeStamp endTime;      // time most recent value calculated
  timeStamp totalTime;    // sum of time over which value is collected
                          //  (may not be contiguous)
  unsigned AllDataReady;  // compare this with DataStatus mask for all data in
  unsigned EnableStatus; // compare with AllDataReady to see when all ports enabled 
  unsigned DataStatus;    // updated by data arriving on each port
  int numInPorts;       // how many IN ports?
  vector<inPort> AllData;
  sampleValue *AllCurrentValues;
  unsigned TimesAligned;  // flag that beginning stage has passed
  bool active;            // is data collection active for this PCmetInst?
  bool costFlag;          // if true, data collection continues during pause
  filteredDataServer *db; // source of all raw data
  vector<dataSubscriber *> costWaitList; // waiting for initial cost estimate
};

ostream& operator <<(ostream &os, PCmetricInst &pcm);

class PCMRec {
  friend class PCmetricInstServer;
public:
  PCMRec(): pcm(NULL), f(0), pcmi(NULL){;}
  PCMRec (PCMRec &from) : pcm(from.pcm), f(from.f), pcmi(from.pcmi){;}
private:
  PCMRec (PCmetric *pp, focus ff, PCmetricInst *ii) : pcm(pp), f(ff), pcmi(ii) {;} 
  PCmetric *pcm;
  focus f;
  PCmetricInst *pcmi;
};

class PCmetricInstServer {
 public:
  PCmetricInstServer(unsigned phaseID);
  ~PCmetricInstServer();
  // interface to subscribers
  PCmetInstHandle createPcmi(PCmetric *pcm,
			     focus f,
			     bool *errFlag);
  // data
  void unsubscribeAllRawData()
    { datasource->unsubscribeAllData(); }
  void resubscribeAllRawData()
    { datasource->resubscribeAllData(); }
 private:
  vector<PCMRec> AllData;
  filteredDataServer *datasource;
};

#endif
