
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

//
// What gets returned when a newValue is called in sampleInfo;
struct sampleInterval {
    Boolean	valid;
    timeStamp	start;
    timeStamp	end;
    sampleValue	value;
};

class sampleInfo {
  public:
    struct sampleInterval newValue(timeStamp wallTime, sampleValue value);
    struct sampleInterval newValue(List<sampleInfo *> peers, 
				   timeStamp wallTime, 
				   sampleValue value);
    sampleInfo() {
        firstSampleReceived = FALSE;
	value = 0.0;
	lastSampleStart = 0.0;
	lastSampleEnd = 0.0;
	lastSample = 0.0;
    }
    Boolean firstSampleReceived;        // has first sample been recorded
    sampleValue value;                  // cumlative value
    timeStamp   lastSampleStart;        // start time for last sample
    timeStamp   lastSampleEnd;          // end time for last sample
    sampleValue lastSample;             // what was the last sample increment
};

#endif
