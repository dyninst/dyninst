
#include "metric.h"

typedef float (*sampleValueFunc)();

//
// Metrics that are internal to a paradyn daemon.
//
class internalMetric {
  public:
    internalMetric(const char *n, int style, int a, const char *units, sampleValueFunc f);
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
    Boolean enabled() {
	return(node != NULL);
    }
    metric metRec;
    float value;
    float cumlativeValue;
    sampleValueFunc func;
    static List<internalMetric*> allInternalMetrics;
    static List<internalMetric*> activeInternalMetrics;
    metricDefinitionNode *node;
};
