#ifndef COST_METRICS_HDR 
#define  COST_METRICS_HDR

#include "paradynd/src/metric.h"
#include "paradynd/src/process.h"
#include "paradynd/src/im_preds.h"
#include "paradynd/src/internalMetrics.h"
#include "paradynd/src/resource.h"

#define PAST_LIMIT 6
class costMetric {
  friend sampleInterval costMetricValueUpdate(costMetric *met, 
					      process *proc,
					      sampleValue value,
					      timeStamp endTime,
					      timeStamp processTime);
 public:
  costMetric(const string n,
	     metricStyle style, 
	     int a,   // how paradyn combines values from daemons
	     const string units,
	     im_pred_struct& preds,
	     bool developerMode,
	     daemon_MetUnitsType unitstype,
	     int combiner_op); // how daemons combine values from processes
  ~costMetric(){}
  void enable(metricDefinitionNode *n) { node = n; }
  void disable() { node = NULL; }
  bool enabled() { return(node != NULL); }
  metricStyle style() { return style_; }
  string name() const { return name_;}
  const char *getName() const { return name_.string_of();}
  int aggregate() const { return agg_; }
  sampleValue getValue() { return (sample.value); }
  static costMetric *newCostMetric(const string n, 
			       metricStyle style, 
			       int a,   // how paradyn combines values
			       const string units,
			       im_pred_struct& preds,
			       bool developerMode,
			       daemon_MetUnitsType unitstype,
			       int combiner_op); // how daemons combine values
  
  T_dyninstRPC::metricInfo getInfo() {
    T_dyninstRPC::metricInfo ret;
    ret.name = name_; ret.style = style_;
    ret.aggregate = agg_; ret.units = units_;
    ret.developerMode = developermode_;
    ret.unitstype = 0;
    if(unitstype_ == UnNormalized) ret.unitstype = 0;
    else if (unitstype_ == Normalized) ret.unitstype = 1; 
    else if (unitstype_ == Sampled) ret.unitstype = 2; 
    return ret;
  }

  bool legalToInst(vector< vector<string> >& focus);
  bool isDeveloperMode() { return developermode_; }


  // add new entries to components, lastProcessTime, and parts
  bool addProcess(process *p);
  static bool addProcessToAll(process *newproc);
  // remove new entries to components, lastProcessTime, and parts
  bool removeProcess(process *p);
  static bool removeProcessFromAll(process *proc);

  timeStamp getLastSampleTime(process *proc){
      for(unsigned i=0; i < components.size(); i++){
	  if(proc == components[i]){
              return(parts[i]->lastSampleEnd);
      } }
      return 0.0;
  }
  timeStamp getLastSampleProcessTime(process *proc){
      for(unsigned i = 0; i < components.size(); i++){
	  if(proc == components[i]){
              return(lastProcessTime[i]);
      } }
      return 0.0;
  }
  sampleValue getCumulativeValue(process *proc){
      for(unsigned i = 0; i < components.size(); i++){
	  if(proc == components[i]){
	      return(cumulative_values[i]);
      } }
      return 0.0;
  }

  // updates the value of the cost metric 
  void updateValue(process *proc,        // process sending cost data
		   sampleValue newValue,  // new value 
		   timeStamp endTime,    // wall time
		   timeStamp processTime);  // CPU time
  // updates the smoothed value of the cost metric
  void updateSmoothValue(process *, sampleValue ,timeStamp, timeStamp ); 

  static vector<costMetric*> allCostMetrics;
  metricDefinitionNode *node;

private:
  // list of processes and values contributing to metric value
  vector<process *> components;
  vector<sampleInfo *> parts;
  vector<sampleValue> cumulative_values;

  // process times associated with each component wall times are contained
  // in the parts vector
  vector<timeStamp> lastProcessTime;

  // circular list of past values for the metric (for computing smoothed costs)
  // these values are normalized by time
  float past[PAST_LIMIT];
  int  past_head;
  sampleValue smooth_sample;
  sampleInfo sample;

  string name_;
  int agg_;
  metricStyle style_;
  string units_;
  im_pred_struct pred;
  bool developermode_;
  daemon_MetUnitsType unitstype_;
  int combinerop_;
};

#endif
