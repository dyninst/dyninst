
#ifndef INTERNAL_MET_HDR
#define INTERNAL_MET_HDR

#include "metric.h"
#include "util/h/list.h"

typedef float (*sampleValueFunc)();

//
// Metrics that are internal to a paradyn daemon.
//
class internalMetric {
  public:
    internalMetric(const string n, int style, int a, const string units,
		   sampleValueFunc f, resourcePredicate *r, bool really=true);

    float getValue() {
	if (func) {
	    return((func)());
	} else {
	    return(value);
	}
    }
    void enable(metricDefinitionNode *n) {
	node = n;
	activeInternalMetrics[n] = this;
    }
    void disable() {
	node = NULL;
    }
    bool enabled() {
	return(node != NULL);
    }
    metric metRec;
    float value;
    float cumulativeValue;
    sampleValueFunc func;
    metricDefinitionNode *node;
    static dictionary_hash<string, internalMetric*> allInternalMetrics;
    static dictionary_hash<metricDefinitionNode*, internalMetric*>
      activeInternalMetrics;

    string getName() const { return name;}
    bool reallyIsEventCounter;

  private:
    string name;
};

#endif
