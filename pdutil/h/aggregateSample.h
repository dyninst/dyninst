
//
// Define a class for a sample value.
//    Sample values can be a single value or may be the result of an aggregate.
//    This is in the util library because both paradynd's and paradyn need to
//      use it.
//

#ifndef UTIL_SAMPLE
#define UTIL_SAMPLE

#include "util/h/list.h"
#include "util/h/hist.h"
#include "util/h/Vector.h"
#include "util/h/aggregation.h"

//
// What gets returned when a newValue is called in sampleInfo;
struct sampleInterval {
    bool valid;
    timeStamp	start;
    timeStamp	end;
    sampleValue	value;
};

// class sampleInfo: define a class for sample values. This class should be used
// with class aggregatesample. sampleInfo objects are allocated and deallocated
// by class aggregateSample.
// Objects are allocated by aggregateSample::newSampleInfo.
// All sampleInfo object must be a component of at least one aggregateSample, 
// and they can of more many aggregateSamples.

class sampleInfo {
  friend class aggregateSample;

  public:
 
    bool firstValueReceived() { return firstSampleReceived; }

    void startTime(timeStamp startTime_);

    struct sampleInterval newValue(timeStamp wallTime, sampleValue value, 
                                   unsigned weight_ = 1);

    timeStamp lastSampleTime() { return lastSampleEnd; }

    // constructor and destructor are private. 
    // They should only be used by class aggregateSample.

private:

    sampleInfo() {
        firstSampleReceived = false;
	lastSampleStart = 0.0;
	lastSampleEnd = 0.0;
	lastSample = 0.0;
	weight = 1;
        numAggregators = 0;
    }

    ~sampleInfo() {};

    sampleInfo &operator=(const sampleInfo &src) {
       firstSampleReceived = src.firstSampleReceived;
       lastSampleStart = src.lastSampleStart;
       lastSampleEnd = src.lastSampleEnd;
       lastSample = src.lastSample;
       weight = src.weight;
       numAggregators = src.numAggregators;
       return *this;
    }

    bool firstSampleReceived;        // has first sample been recorded
    timeStamp   lastSampleStart;        // start time for last sample
    timeStamp   lastSampleEnd;          // end time for last sample
    sampleValue lastSample;             // what was the last sample increment
    unsigned numAggregators;            // number of aggregateSample this is a part of
    unsigned weight;                    // weight of this sample
};


// aggregateSample: aggregate values for samples. aggregate samples can have
// one or more components. Components can be added or removed at any time.

class aggregateSample {

public:
  
  aggregateSample(int aggregateOp) {
    assert(aggregateOp == aggSum || aggregateOp == aggAvg || aggregateOp == aggMin
	   || aggregateOp == aggMax);
    aggOp = aggregateOp;
    lastSampleStart = 0.0;
    lastSampleEnd = 0.0;
  }

  ~aggregateSample() {
    unsigned u;
    for (u = 0; u < newParts.size(); u++)
      if (--newParts[u]->numAggregators == 0)
        delete newParts[u];
    for (u = 0; u < parts.size(); u++)
      if (--parts[u]->numAggregators == 0)
        delete parts[u];
  }

  sampleInfo *newComponent() {
    sampleInfo *comp = new sampleInfo();
    addComponent(comp);
    return comp;
  }

  void addComponent(sampleInfo *comp) {
    newParts += comp;
    removedNewParts += false;
    comp->numAggregators++;
  }

  // remove a component. The sampleInfo object will be deallocated once
  // its value has been aggregated.
  void removeComponent(sampleInfo *comp) {
    for (unsigned u = 0; u < parts.size(); u++) {
      if (parts[u] == comp) {
        removedParts[u] = true;
        return;
      }
    }
    for (unsigned u = 0; u < newParts.size(); u++) {
      if (newParts[u] == comp) {
        removedNewParts[u] = true;
        return;
      }
    }
    assert(0);
  }

  // aggregate the values for all components.
  struct sampleInterval aggregateValues();

  // return the number of components included in the last valid aggregate value.
  inline unsigned numComponents() {
    return parts.size();
  }

  timeStamp currentTime() { return lastSampleEnd; }

private:

  int aggOp;                        // the aggregate operator (sum, avg, min, max)
  timeStamp lastSampleStart;        // start time of last sample
  timeStamp lastSampleEnd;          // end time of last sample
  vector<sampleInfo *> parts;       // the parts that are being aggregated.
                                    // For all u, 
                                    //    parts[u]->lastSampleStart == lastSampleEnd
  vector<bool> removedParts;        // true if parts[u] has been removed.
                                    // Once parts[u]->lastSampleStart ==
                                    //                 parts[u]->lastSampleEnd
                                    // parts[u] will be deleted.
  vector<sampleInfo *> newParts;    // new parts that have been added to the 
                                    // aggregation, but cannot be aggregated yet
                                    // because their start time is not aligned with
                                    // lastSampleEnd.
  vector<bool> removedNewParts;     // new parts that have been removed.
};

#endif

