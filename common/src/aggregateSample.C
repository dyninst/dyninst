
/*
 * 
 * $Log: aggregateSample.C,v $
 * Revision 1.12  1996/02/09 22:15:28  mjrg
 * fixed aggregation to handle first samples and addition of new components
 *
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

struct sampleInterval sampleInfo::startTime(timeStamp startTime_) {
  struct sampleInterval ret;

  firstSampleReceived = true;
  lastSampleStart = startTime_;
  // The remaining fields should be zero.
  // It is important that the value of lastSampleEnd is zero, otherwise the
  // aggregation code in newValue will not work.

  ret.valid = false;
  return ret;
}

struct sampleInterval sampleInfo::firstValue(timeStamp startTime, timeStamp endTime,
					     sampleValue value_) {
  struct sampleInterval ret;

  firstSampleReceived = true;
  lastSampleStart = startTime;
  lastSampleEnd = endTime;
  lastSample = value_;
  value = value_;
  
  ret.valid = true;
  ret.start = lastSampleStart;
  ret.end =lastSampleEnd;
  ret.value = lastSample;
  return ret;
}

struct sampleInterval sampleInfo::newValue(timeStamp sampleTime, 
						     sampleValue newVal)
{
    struct sampleInterval ret;

    assert(firstSampleReceived);
    // use the first sample to define a baseline in time and value.
    //if (!firstSampleReceived) {
    //	firstSampleReceived = true;
    //	lastSampleStart = sampleTime;
    //	lastSampleEnd = sampleTime;
    //	lastSample = 0.0;
    //	value = newVal;
    //	ret.valid = false;
    //	return(ret);
    //}

    ret.valid = true;
    ret.start = lastSampleEnd;
    ret.end = sampleTime;
    ret.value = newVal;

    // used when it's a component of an aggregate.
    lastSample += newVal;
    lastSampleEnd = sampleTime;

    return(ret);
}

#ifdef notdef
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
#endif

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
	for(unsigned i=0; i < parts.size(); i++){
	    if(parts[i]->lastSampleEnd < earlyestTime){
		earlyestTime = parts[i]->lastSampleEnd;
	    }
	}

	if (earlyestTime > lastSampleEnd + 0.0001) {
	    /* eat the first one to get a good interval basis */
	    if (!firstSampleReceived) {
	        // The start times for the components are probably not
	        // aligned. To simplify things, we find the latest start time
	        // for any component, and pick this as the start time for
	        // the aggregation. The samples of each component are truncated 
	        // to the new start time.
	        // 

		// find the latest start time
		timeStamp latestStartTime = parts[0]->lastSampleStart;
		for (unsigned j = 0; j < parts.size(); j++) {
		  if (parts[j]->lastSampleStart > latestStartTime)
		    latestStartTime = parts[j]->lastSampleStart;
		}

		if (latestStartTime >= earlyestTime) {
		  // We are not ready to do the aggregation yet.
		  // firstSampleReceived is still false
		  ret.valid = false;
		  return(ret);
		}

		// We are ready to aggregate from latestStartTime to earlyestTime.
		firstSampleReceived = true;
		lastSampleEnd = latestStartTime;
		nparts = parts.size();

		// latestStartTime is the initial start time for the aggregation.
		// Truncate lastSample of all components accordingly.
		for (unsigned j = 0; j < parts.size(); j++) {
		    timeStamp diff = parts[j]->lastSampleEnd - latestStartTime;
		    if (diff > 0) {
		      parts[j]->lastSample *= ( diff / 
				 (parts[j]->lastSampleEnd - parts[j]->lastSampleStart));
		      parts[j]->lastSampleStart = latestStartTime;
		    }
		    else {
		      parts[j]->lastSampleStart = parts[j]->lastSampleEnd;
		      parts[j]->lastSample = 0;
		    }
		}
	    }


	    if (parts.size() < nparts) {
	      // one part was deleted.
	      nparts = parts.size();
	    }
	    else if (parts.size() > nparts) {

	      // number of parts has changed
	      // to simplify things, we make the first samples of the new parts 
	      // have the same initial time.

	      // we can find which are the samples for the new parts by checking 
	      // the value of lastSampleStart for each part. The new ones have
	      // lastSampleStart >= this->lastSampleEnd.

	      // find the initial time for the new samples
	      timeStamp newInitialTime = lastSampleEnd;
	      for (unsigned j = 0; j < parts.size(); j++) {
		if (parts[j]->lastSampleStart > newInitialTime) {
		  newInitialTime = parts[j]->lastSampleStart;
		}
	      }

	      // make all new samples have the same initial time newInitialTime
	      for (unsigned j = 0; j < parts.size(); j++) {
		if (parts[j]->lastSampleStart > lastSampleEnd) {
		  // this is a new component
		  timeStamp diff = parts[j]->lastSampleEnd - newInitialTime;
		  if (diff > 0) {
		    parts[j]->lastSample *= ( diff / 
				  (parts[j]->lastSampleEnd - parts[j]->lastSampleStart));
		    parts[j]->lastSampleStart = newInitialTime;
		  }
		  else {
		    parts[j]->lastSampleStart = parts[j]->lastSampleEnd;
		    parts[j]->lastSample = 0;
		  }
		}
	      }

	      // Check if we can return a valid sample now.
	      if (earlyestTime < newInitialTime) {
		// The new samples are not part of the aggregation yet.
		// We still have to handle the special case of new samples,
		// so we don't set nparts here, so next time we still execute
		// this code.
	      } else if (newInitialTime > lastSampleEnd) {
		// We must aggregate the old parts until newInitialTime.
		// New samples still are not part of the aggregation, so
		// we don't set nparts here either.
		earlyestTime = newInitialTime;
	      } else {
		// New sample are part of the aggregation.
		nparts = parts.size();
	      }
	    }

	    sampleValue aggregateVal = 0.0;

	    int first = 1;
	    unsigned nSamples = 0; // number of samples being aggregated

	    for (unsigned i2=0; i2< parts.size(); i2++) {
	      
	      // We must check if this part should be aggregated, since we may have
	      // samples for new parts that aren't ready to aggregate yet.
	      if (parts[i2]->lastSampleStart < earlyestTime) {

		++nSamples;

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
	    }

	    // nSamples is the number of samples being aggregated
	    if (aggOp == aggAvg)
	      aggregateVal /= nSamples;

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
	        // The start times for the components are probably not
	        // aligned. To simplify things, we find the latest start time
	        // for any component, and pick this as the start time for
	        // the aggregation. The samples of each component are truncated 
	        // to the new start time.
	        // 

		// find the latest start time
		timeStamp latestStartTime = parts[0]->lastSampleStart;
		for (unsigned j = 0; j < parts.size(); j++) {
		  if (parts[j]->lastSampleStart > latestStartTime)
		    latestStartTime = parts[j]->lastSampleStart;
		}

		if (latestStartTime >= earlyestTime) {
		  // We are not ready to do the aggregation yet.
		  // firstSampleReceived is still false
		  ret.valid = false;
		  return(ret);
		}

		// We are ready to aggregate from latestStartTime to earlyestTime.
		firstSampleReceived = true;
		lastSampleEnd = latestStartTime;
		nparts = parts.size();

		// latestStartTime is the initial start time for the aggregation.
		// Truncate lastSample of all components accordingly.
		for (unsigned j = 0; j < parts.size(); j++) {
		    timeStamp diff = parts[j]->lastSampleEnd - latestStartTime;
		    if (diff > 0) {
		      parts[j]->lastSample *= ( diff / 
				 (parts[j]->lastSampleEnd - parts[j]->lastSampleStart));
		      parts[j]->lastSampleStart = latestStartTime;
		    }
		    else {
		      parts[j]->lastSampleStart = parts[j]->lastSampleEnd;
		      parts[j]->lastSample = 0;
		    }
		}
	    }

	    if (parts.size() > nparts) {

	      // number of parts has changed
	      // to simplify things, we make the first samples of the new parts 
	      // have the same initial time.

	      // we can find which are the samples for the new parts by checking 
	      // the value of lastSampleStart for each part. The new ones have
	      // lastSampleStart >= this->lastSampleEnd.

	      // find the initial time for the new samples
	      timeStamp newInitialTime = lastSampleEnd;
	      for (unsigned j = 0; j < parts.size(); j++) {
		if (parts[j]->lastSampleStart > newInitialTime) {
		  newInitialTime = parts[j]->lastSampleStart;
		}
	      }

	      // make all new samples have the same initial time newInitialTime
	      for (unsigned j = 0; j < parts.size(); j++) {
		if (parts[j]->lastSampleStart > lastSampleEnd) {
		  // this is a new component
		  timeStamp diff = parts[j]->lastSampleEnd - newInitialTime;
		  if (diff > 0) {
		    parts[j]->lastSample *= ( diff / 
				  (parts[j]->lastSampleEnd - parts[j]->lastSampleStart));
		    parts[j]->lastSampleStart = newInitialTime;
		  }
		  else {
		    parts[j]->lastSampleStart = parts[j]->lastSampleEnd;
		    parts[j]->lastSample = 0;
		  }
		}
	      }

	      // Check if we can return a valid sample now.
	      if (earlyestTime < newInitialTime) {
		// The new samples are not part of the aggregation yet.
		// We still have to handle the special case of new samples,
		// so we don't set nparts here, so next time we still execute
		// this code.
	      } else if (newInitialTime > lastSampleEnd) {
		// We must aggregate the old parts until newInitialTime.
		// New samples still are not part of the aggregation, so
		// we don't set nparts here either.
		earlyestTime = newInitialTime;
	      } else {
		// New sample are part of the aggregation.
		nparts = parts.size();
	      }
	    }

	    sampleValue aggregateVal = 0.0;

	    int first = 1;

	    unsigned total_weight = 0;
	    for (unsigned i2=0; i2< parts.size(); i2++) {
	      
	      // We must check if this part should be aggregated, since we may have
	      // samples for new parts that aren't ready to aggregate yet.
	      if (parts[i2]->lastSampleStart < earlyestTime) {

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
