
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
		   sampleValueFunc f, resourcePredicate *r);

    float getValue() {
	if (func) {
	    return((func)());
	} else {
	    return(value);
	}
    }
    void enable(metricDefinitionNode *n) {
	node = n;
	activeInternalMetrics.add(this, n);
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
    static dictionary_hash<string, internalMetric*> allInternalMetrics;
    static List<internalMetric*> activeInternalMetrics;
    metricDefinitionNode *node;
    string getName() const { return name;}

  private:
    string name;
};

#endif
