
/*
 * 
 * $Log: aggregateSample.C,v $
 * Revision 1.6  1994/06/14 15:36:49  markc
 * Added support for different types of aggregation (max, min, sum, avg).
 * Previously, only summation had been done.
 *
 * Revision 1.5  1994/06/02  23:36:10  markc
 * Added assertion to ensure values reported are positive.
 *
 * Revision 1.4  1994/05/17  00:14:44  hollings
 * added rcs log entry.
 *
 *
 */
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
    int len=0;

    assert((aggOp == Sum) || (aggOp == Avg) ||
	   (aggOp == Min) || (aggOp == Max));

    if (parts.count() <= 1) {
       // not an aggregate.
       return(newValue(sampleTime, newVal));
    } else {
	earlyestTime = (*parts)->lastSampleEnd;
	for (cp=parts; curr = *cp; cp++) {
	    len++;
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

               // this gives all of the samples the same initial starting
	       // time
	       // It is very important for them to have the same time
	       // if this is not done, fract that is calculated below
	       // will fail the assertions
	       // You may want to zero the lastSample values here too

		for (cp=parts; curr=*cp; cp++) 
		    curr->lastSampleStart = earlyestTime;

		return(ret);
	    }

	    aggregateVal = 0.0;

	    int first = 1;

	    for (cp=parts; curr=*cp; cp++) {
		// assert(earlyestTime >= curr->lastSampleStart);

		fract = (earlyestTime - lastSampleEnd)/
		    (curr->lastSampleEnd - curr->lastSampleStart);
		component = (curr->lastSample) * fract;

		assert(fract > 0.0);
		assert(fract <= 1.0);
		assert(component >= -0.01);

		curr->lastSample -= component;

		// each list entry comes from a separate reporter
		switch (aggOp)
		  {
		  case Sum:
		  case Avg:
		    aggregateVal += component;
		    break;
		  case Min:
		    if (first) {
		      aggregateVal = component;
		      first = 0;
		    } else if (component < aggregateVal)
		      aggregateVal = component;
		    break;
		  case Max:
		    if (component > aggregateVal)
		      aggregateVal = component;
		    break;
		  }

		/* move forward our time of our earliest sample */
		curr->lastSampleStart = earlyestTime;
	      }

	    // len is the number of samples on the list
	    if (aggOp == Avg)
	      aggregateVal /= len;

	    ret.valid = TRUE;
	    ret.start = lastSampleEnd;
	    ret.end = earlyestTime;
	    ret.value = aggregateVal;
	    assert(ret.value >= 0.0);

	    lastSampleStart = lastSampleEnd;
	    lastSampleEnd = earlyestTime;
	} else {
	    ret.valid = FALSE;
	}
    }
    return(ret);
}
