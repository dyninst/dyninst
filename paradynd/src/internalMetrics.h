
#ifndef INTERNAL_MET_HDR
#define INTERNAL_MET_HDR

/*
 * $Log: internalMetrics.h,v $
 * Revision 1.8  1995/08/24 15:04:11  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 *
 */
#include "paradynd/src/metric.h"
#include "paradynd/src/im_preds.h"

typedef float (*sampleValueFunc)();

//
// Metrics that are internal to a paradyn daemon.
//

// Why use classes if everything is public?
class internalMetric {
 public:
  internalMetric(const string n, metricStyle style, int a, const string units,
		 sampleValueFunc f, im_pred_struct& im_preds) 
    : value(0.0), cumulativeValue(0.0), func(f), node(NULL), name_(n), agg_(a),
      style_(style), units_(units), pred(im_preds) { }

  inline float getValue();
  void enable(metricDefinitionNode *n) { node = n; }
  void disable() { node = NULL; }
  bool enabled() { return(node != NULL); }
  metricStyle style() { return style_; }
  string name() const { return name_;}

  static vector<internalMetric*> allInternalMetrics;
  static internalMetric *newInternalMetric(const string n, metricStyle style, int a,
					   const string units,
					   sampleValueFunc f, im_pred_struct& preds);
  float value;
  float cumulativeValue;
  sampleValueFunc func;
  metricDefinitionNode *node;
  int aggregate() const { return agg_; }

  T_dyninstRPC::metricInfo getInfo() {
    T_dyninstRPC::metricInfo ret;
    ret.name = name_; ret.style = style_;
    ret.aggregate = agg_; ret.units = units_;
    return ret;
  }
  bool legalToInst(vector< vector<string> >& focus);

private:
  string name_;
  int agg_;
  metricStyle style_;
  string units_;
  im_pred_struct pred;
};

float internalMetric::getValue() {
  if (func)
    return((func)());
  else
    return(value);
}

#endif
