
/*
 * 
 * $Log: aggregateSample.C,v $
 * Revision 1.11  1996/01/31 19:47:37  newhall
 * added a newValue method that takes a vector of weights for each part
 *
 * Revision 1.10  1995/09/08  19:44:56  krisna
 * stupid way to avoid the for-scope problem
 *
 * Revision 1.9  1995/06/02  21:00:07  newhall
 * added a NaN value generator
 * fixed memory leaks in Histogram class
 * added newValue member with a vector<sampleInfo *> to class sampleInfo
 *
 * Revision 1.8  1995/02/16  09:27:59  markc
 * Removed compiler warnings.
 * Changed Boolean to bool
 *
 * Revision 1.7  1994/07/02  01:47:31  markc
 * Rename aggregation operator defines.
 * Rename aggregation operator defines.
 *
 * Revision 1.6  1994/06/14  15:36:49  markc
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
	firstSampleReceived = true;
	lastSampleStart = sampleTime;
	lastSampleEnd = sampleTime;
	lastSample = 0.0;
	value = newVal;
	ret.valid = false;
	return(ret);
    }

    ret.valid = true;
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

    assert((aggOp == aggSum) || (aggOp == aggAvg) ||
	   (aggOp == aggMin) || (aggOp == aggMax));

    if (parts.count() <= 1) {
       // not an aggregate.
       return(newValue(sampleTime, newVal));
    } else {
	earlyestTime = (*parts)->lastSampleEnd;
	for (cp=parts; (curr = *cp); cp++) {
	    len++;
	    if (curr->lastSampleEnd < earlyestTime) {
		earlyestTime = curr->lastSampleEnd;
	    }
	}


	if (earlyestTime > lastSampleEnd + 0.0001) {
	    /* eat the first one to get a good interval basis */
	    if (!firstSampleReceived) {
		firstSampleReceived = true;
		ret.valid = false;
		lastSampleEnd = earlyestTime;

               // this gives all of the samples the same initial starting
	       // time
	       // It is very important for them to have the same time
	       // if this is not done, fract that is calculated below
	       // will fail the assertions
	       // You may want to zero the lastSample values here too

		for (cp=parts; (curr=*cp); cp++) 
		    curr->lastSampleStart = earlyestTime;

		return(ret);
	    }

	    aggregateVal = 0.0;

	    int first = 1;

	    for (cp=parts; (curr=*cp); cp++) {
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
		  case aggSum:
		  case aggAvg:
		    aggregateVal += component;
		    break;
		  case aggMin:
		    if (first) {
		      aggregateVal = component;
		      first = 0;
		    } else if (component < aggregateVal)
		      aggregateVal = component;
		    break;
		  case aggMax:
		    if (component > aggregateVal)
		      aggregateVal = component;
		    break;
		  }

		/* move forward our time of our earliest sample */
		curr->lastSampleStart = earlyestTime;
	      }

	    // len is the number of samples on the list
	    if (aggOp == aggAvg)
	      aggregateVal /= len;

	    ret.valid = true;
	    ret.start = lastSampleEnd;
	    ret.end = earlyestTime;
	    ret.value = aggregateVal;
	    assert(ret.value >= 0.0);

	    lastSampleStart = lastSampleEnd;
	    lastSampleEnd = earlyestTime;
	} else {
	    ret.valid = false;
	}
    }
    return(ret);
}

struct sampleInterval sampleInfo::newValue(vector<sampleInfo *> parts,
					   timeStamp sampleTime, 
					   sampleValue newVal)
{
    struct sampleInterval ret;
    assert((aggOp == aggSum) || (aggOp == aggAvg) ||
	   (aggOp == aggMin) || (aggOp == aggMax));

    if (!parts.size()) {
       // not an aggregate.
       return(newValue(sampleTime, newVal));
    } else {
	timeStamp earlyestTime = parts[0]->lastSampleEnd;
	int len=0;
	for(unsigned i=0; i < parts.size(); i++){
	    len++;
	    if(parts[i]->lastSampleEnd < earlyestTime){
		earlyestTime = parts[i]->lastSampleEnd;
	    }
	}

	if (earlyestTime > lastSampleEnd + 0.0001) {
	    /* eat the first one to get a good interval basis */
	    if (!firstSampleReceived) {
		firstSampleReceived = true;
		ret.valid = false;
		lastSampleEnd = earlyestTime;

               // this gives all of the samples the same initial starting
	       // time
	       // It is very important for them to have the same time
	       // if this is not done, fract that is calculated below
	       // will fail the assertions
	       // You may want to zero the lastSample values here too

		for (unsigned i1=0; i1 < parts.size(); i1++) 
		    parts[i1]->lastSampleStart = earlyestTime;

		return(ret);
	    }

	    sampleValue aggregateVal = 0.0;

	    int first = 1;

	    for (unsigned i2=0; i2< parts.size(); i2++) {
		// assert(earlyestTime >= parts[i2]->lastSampleStart);

		double fract = (earlyestTime - lastSampleEnd)/
		 (parts[i2]->lastSampleEnd - parts[i2]->lastSampleStart);
		sampleValue component_val = (parts[i2]->lastSample) * fract;

		assert(fract > 0.0);
		assert(fract <= 1.0);
		assert(component_val >= -0.01);

		parts[i2]->lastSample -= component_val;

		// each list entry comes from a separate reporter
		switch (aggOp)
		  {
		  case aggSum:
		  case aggAvg:
		    aggregateVal += component_val;
		    break;
		  case aggMin:
		    if (first) {
		      aggregateVal = component_val;
		      first = 0;
		    } else if (component_val < aggregateVal)
		      aggregateVal = component_val;
		    break;
		  case aggMax:
		    if (component_val > aggregateVal)
		      aggregateVal = component_val;
		    break;
		  }

		/* move forward our time of our earliest sample */
		parts[i2]->lastSampleStart = earlyestTime;
	      }

	    // len is the number of samples on the list
	    if (aggOp == aggAvg)
	      aggregateVal /= len;

	    ret.valid = true;
	    ret.start = lastSampleEnd;
	    ret.end = earlyestTime;
	    ret.value = aggregateVal;
	    assert(ret.value >= 0.0);

	    lastSampleStart = lastSampleEnd;
	    lastSampleEnd = earlyestTime;
	} else {
	    ret.valid = false;
	}
    }
    return(ret);
}

struct sampleInterval sampleInfo::newValue(vector<sampleInfo *> &parts,
			                   vector<unsigned> &weight_of_part,
			                   timeStamp sampleTime,
			                   sampleValue newVal)

{
    struct sampleInterval ret;
    assert((aggOp == aggSum) || (aggOp == aggAvg) ||
	   (aggOp == aggMin) || (aggOp == aggMax));

    
    if (!parts.size()) {
       // not an aggregate.
       return(newValue(sampleTime, newVal));
    } else {
	assert(parts.size() == weight_of_part.size());
	timeStamp earlyestTime = parts[0]->lastSampleEnd;
	for(unsigned i=0; i < parts.size(); i++){
	    if(parts[i]->lastSampleEnd < earlyestTime){
		earlyestTime = parts[i]->lastSampleEnd;
	    }
	}

	if (earlyestTime > lastSampleEnd + 0.0001) {
	    /* eat the first one to get a good interval basis */
	    if (!firstSampleReceived) {
		firstSampleReceived = true;
		ret.valid = false;
		lastSampleEnd = earlyestTime;

               // this gives all of the samples the same initial starting
	       // time
	       // It is very important for them to have the same time
	       // if this is not done, fract that is calculated below
	       // will fail the assertions
	       // You may want to zero the lastSample values here too

		for (unsigned i1=0; i1 < parts.size(); i1++) 
		    parts[i1]->lastSampleStart = earlyestTime;

		return(ret);
	    }

	    sampleValue aggregateVal = 0.0;

	    int first = 1;

            unsigned total_weight = 0;
	    for (unsigned i2=0; i2< parts.size(); i2++) {
		// assert(earlyestTime >= parts[i2]->lastSampleStart);

		double fract = (earlyestTime - lastSampleEnd)/
		 (parts[i2]->lastSampleEnd - parts[i2]->lastSampleStart);
		sampleValue component_val = (parts[i2]->lastSample) * fract;

		assert(fract > 0.0);
		assert(fract <= 1.0);
		assert(component_val >= -0.01);

		parts[i2]->lastSample -= component_val;

		// each list entry comes from a separate reporter
		switch (aggOp)
		  {
		  case aggSum:
		    aggregateVal += component_val;
		    break;
		  case aggAvg:
		    aggregateVal += (weight_of_part[i2]*component_val);
		    total_weight += weight_of_part[i2];
		    break;
		  case aggMin:
		    if (first) {
		      aggregateVal = component_val;
		      first = 0;
		    } else if (component_val < aggregateVal)
		      aggregateVal = component_val;
		    break;
		  case aggMax:
		    if (component_val > aggregateVal)
		      aggregateVal = component_val;
		    break;
		  }

		/* move forward our time of our earliest sample */
		parts[i2]->lastSampleStart = earlyestTime;
	      }

	    // total_weight is the weighted count of the samples on the list  
	    if (aggOp == aggAvg){
	      aggregateVal /= total_weight;
            }

	    ret.valid = true;
	    ret.start = lastSampleEnd;
	    ret.end = earlyestTime;
	    ret.value = aggregateVal;
	    assert(ret.value >= 0.0);

	    lastSampleStart = lastSampleEnd;
	    lastSampleEnd = earlyestTime;
	} else {
	    ret.valid = false;
	}
    }
    return(ret);
}
