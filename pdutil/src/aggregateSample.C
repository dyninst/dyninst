
#include <assert.h>

#include "util/h/aggregateSample.h"

struct sampleInterval sampleInfo::newValue(timeStamp sampleTime, 
						     sampleValue newVal)
{
    struct sampleInterval ret;

    // use the first sample to define a baseline in time and value.
    if (!firstSampleReceived) {
	firstSampleReceived = True;
	lastSampleStart = sampleTime;
	lastSampleEnd = sampleTime;
	lastSample = 0.0;
	value = newVal;
	ret.valid = FALSE;
	return(ret);
    }

    ret.valid = TRUE;
    ret.start = lastSampleEnd;
    ret.end = sampleTime;
    ret.value = newVal;

    // used when it's a component of an aggregate.
    lastSample += newVal;
    lastSampleEnd = sampleTime;

    return(ret);
}

struct sampleInterval sampleInfo::newValue(List<sampleInfo *> parts,
					   timeStamp sampleTime, 
					   sampleValue newVal)
{
    double fract;
    sampleInfo *curr;
    List<sampleInfo*> cp;
    sampleValue component;
    timeStamp earlyestTime;
    struct sampleInterval ret;
    sampleValue aggregateVal;

    if (parts.count() <= 1) {
       // not an aggregate.
       return(newValue(sampleTime, newVal));
    } else {
	earlyestTime = (*parts)->lastSampleEnd;
	for (cp=parts; curr = *cp; cp++) {
	    if (curr->lastSampleEnd < earlyestTime) {
		earlyestTime = curr->lastSampleEnd;
	    }
	}


	if (earlyestTime > lastSampleEnd + 0.0001) {
	    /* eat the first one to get a good interval basis */
	    if (!firstSampleReceived) {
		firstSampleReceived = True;
		ret.valid = FALSE;
		lastSampleEnd = earlyestTime;
		return(ret);
	    }

	    aggregateVal = 0.0;
	    for (cp=parts; curr=*cp; cp++) {
		// assert(earlyestTime >= curr->lastSampleStart);

		fract = (earlyestTime - lastSampleEnd)/
		    (curr->lastSampleEnd - curr->lastSampleStart);
		component = (curr->lastSample) * fract;

		assert(fract > 0.0);
		assert(fract <= 1.0);
		assert(component >= -0.01);

		curr->lastSample -= component;
		aggregateVal += component;
		/* move forward our time of our earliest sample */
		curr->lastSampleStart = earlyestTime;
	    }

	    ret.valid = TRUE;
	    ret.start = lastSampleEnd;
	    ret.end = earlyestTime;
	    ret.value = aggregateVal;

	    lastSampleStart = lastSampleEnd;
	    lastSampleEnd = earlyestTime;
	} else {
	    ret.valid = FALSE;
	}
    }
    return(ret);
}
