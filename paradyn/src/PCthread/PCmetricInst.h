/*
 * Copyright (c) 1996-1999 Barton P. Miller
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

// $Id: PCmetricInst.h,v 1.15 2000/10/17 17:27:50 schendel Exp $
// The PCmetricInst class and the PCmetricInstServer class.

#ifndef pc_metric_inst_h
#define pc_metric_inst_h

#include "PCintern.h"
#include "PCfilter.h"
#include "PCmetric.h"
#include "PCdata.h"
#include "pdutilOld/h/CircularBuffer.h"

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
