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

#define PCMETINSTARTNUMCONSUMERS 20

typedef struct {
  unsigned portID;
  metricInstanceHandle mih;
  metricHandle met;
  focus foc;
  dataQ indataQ;
} inPortStruct;
typedef struct inPortStruct inPort;

class PCmetricInst;

class PCmetricInst : public dataProvider, public dataSubscriber 
{
  friend ostream& operator <<(ostream &os, PCmetricInst &pcm);

public:
  PCmetricInst (PCmetric *pcmet, focus f,  
		filteredDataServer *db, bool *err);
  ~PCmetricInst();    

  // these services part of dataProvider role
  sampleValue getCurrentValue () {return currentValue;}
  float getEstimatedCost();

  // returns false if no dm data was subscribed; true otherwise.
  // at the current time, we simply try to subscribe to all data, and 
  // return if any succeed.  
  // ** separate essential from nonessential metrics and t/f based on that.
  bool activate();

  // end subscriptions to all filtered data for this PCmetricInst, 
  // but we don't kill it entirely, so we can always reactivate it by 
  // resubscribing to the data.
  void deactivate();

  // these are called by filtered data source -- subscriber role
  void newData(PCmetDataID whichData, sampleValue newVal, 
	       timeStamp start, timeStamp end, sampleValue);
  void updateEstimatedCost(float costDiff) {estimatedCost += costDiff;}
private:
  bool alignTimes();
  void setDataReady(int portNum);
  void clearDataReady (int portNum);

  focus foc;
  PCmetric *met;
  sampleValue currentValue;
  timeStamp startTime;    // time first value calculated
  timeStamp endTime;      // time most recent value calculated
  timeStamp totalTime;    // sum of time over which value is collected
                          //  (may not be contiguous)
  unsigned AllDataReady;  // compare this with DataStatus mask for all data in
  unsigned DataStatus;    // updated by data arriving on each port
  int numInPorts;       // how many IN ports?
  vector<inPort*> AllData;
  sampleValue *AllCurrentValues;
  unsigned TimesAligned;  // flag that beginning stage has passed
  bool active;            // is data collection active for this PCmetInst?
  filteredDataServer *db; // source of all raw data
};

ostream& operator <<(ostream &os, PCmetricInst &pcm);

struct PCMRec {
  PCmetric *pcm;
  focus f;
  PCmetricInst *pcmi;
};
typedef struct PCMRec PCMRec;

class PCmetricInstServer {
 public:
  PCmetricInstServer(phaseType phase_type);
  ~PCmetricInstServer();
  // interface to subscribers
  PCmetInstHandle addSubscription(PCmetSubscriber sub,
				  PCmetric *pcm,
				  focus f,
				  bool *errFlag);
  void endSubscription(PCmetSubscriber sub, PCmetInstHandle id);
  // data
  PCmetInstHandle addPersistentMI (PCmetric *pcm,
				   focus f,
				   bool *errFlag);
  void changeBinSize(timeStamp newSize) {datasource->newBinSize(newSize);}
  void newRawData(metricInstanceHandle mih, sampleValue value, int bin)
    { 
      // cout << "RAWDATA at mserver: mih=" << mih << " value =" << value 
	//<< " bin =" << bin << endl; 
      datasource->newData (mih, value, bin); }
  void unsubscribeAllRawData()
    { datasource->unsubscribeAllData(); }
  void resubscribeAllRawData()
    { datasource->resubscribeAllData(); }
 private:
  vector<PCMRec*> AllData;
  filteredDataServer *datasource;
};

#endif
