
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

class sampleInfo {
  public:
 
    bool firstValueReceived() { return firstSampleReceived; }

    struct sampleInterval startTime(timeStamp startTime_);

    struct sampleInterval firstValue(timeStamp startTime, timeStamp endTime, 
                                     sampleValue value);

    struct sampleInterval newValue(timeStamp wallTime, sampleValue value);

    //struct sampleInterval newValue(List<sampleInfo *> peers, 
    //				   timeStamp wallTime, 
    //				   sampleValue value);
    struct sampleInterval newValue(vector<sampleInfo *> peers, 
				   timeStamp wallTime, 
				   sampleValue value);
    struct sampleInterval newValue(vector<sampleInfo *> &parts, 
				   vector<unsigned> &weight_of_part,
				   timeStamp wallTime, 
				   sampleValue value);
    sampleInfo( int aOp = aggSum) {
        firstSampleReceived = false;
	value = 0.0;
	lastSampleStart = 0.0;
	lastSampleEnd = 0.0;
	lastSample = 0.0;
	aggOp = aOp;
        nparts = 0;
    }
    bool firstSampleReceived;        // has first sample been recorded
    sampleValue value;                  // cumlative value
    timeStamp   lastSampleStart;        // start time for last sample
    timeStamp   lastSampleEnd;          // end time for last sample
    sampleValue lastSample;             // what was the last sample increment
    int aggOp;

private:

    unsigned nparts;                    // number of parts for an aggregate value

};

#endif
