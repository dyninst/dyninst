/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: costmetrics.C,v 1.17 2000/10/17 17:42:32 schendel Exp $

#include "paradynd/src/costmetrics.h"
#include "dyninstAPI/src/process.h"
#include "pdutil/h/pdDebugOstream.h"

vector<costMetric*> costMetric::allCostMetrics;
extern vector<process*> processVec;
extern pdDebug_ostream sampleVal_cerr;

costMetric::costMetric(const string n,
		     metricStyle style,
		     int a,
		     const string units,
		     im_pred_struct& preds,
		     bool developerMode,
		     daemon_MetUnitsType unitstype,
		     int combiner_op): 
  aggSample((combiner_op != -1) ? combiner_op : a)
{

   name_ = n; style_ = style; agg_ = a; units_ = units; pred = preds; 
   developermode_ = developerMode; unitstype_ = unitstype;
   node = 0;
   if((combiner_op >= 0) && (combiner_op <= 3)) 
       combinerop_ = combiner_op;
   else combinerop_ = -1;

   for(unsigned i2 = 0; i2 < processVec.size(); i2++){
       if((processVec[i2])->status_ != exited){
           components += processVec[i2];
           //sampleInfo *s = new sampleInfo;
           sampleInfo *s = aggSample.newComponent();
           parts += s;
           lastProcessTime += timeStamp::ts1970(); 
           cumulative_values += pdSample(0);
       }
   }
}


costMetric *costMetric::newCostMetric(const string n,
				     metricStyle style,
				     int a,
				     const string units,
				     im_pred_struct& preds,
				     bool developerMode,
				     daemon_MetUnitsType unitstype,
				     int combiner_op){


    costMetric *newcm = new costMetric(n,style,a,units,preds,
				      developerMode,unitstype,combiner_op);
    allCostMetrics += newcm; 
    return newcm;
}


bool costMetric::legalToInst(vector< vector<string> >& focus) {

    if (!processVec.size()) {
        // we don't enable metrics if there are no process to run
	return false;
    }

    bool all_exited = true;
    for(u_int i=0; i < processVec.size(); i++){
	if((processVec[i])->status_ != exited){
	    all_exited = false;
    } }
    if(all_exited) return false;

    switch (focus[resource::machine].size()) {
        case 1: break;
        case 2:
	case 3:
            switch(pred.machine) {
	        case pred_invalid: return false;
	        case pred_null: break;
	        default: return false;
            }
        default: return false;
    }
    switch (focus[resource::procedure].size()) {
	case 1: break;
	case 2:
	case 3:
	    switch(pred.procedure) {
		case pred_invalid: return false;
		case pred_null: break;
		default: return false;
	    }
        default: return false;
    }
    switch (focus[resource::sync_object].size()) {
	case 1: break;
	case 2:
	case 3:
	    switch(pred.sync) {
		case pred_invalid: return false;
		case pred_null: break;
		default: return false;
	    }
        default: return false;
    }
    return true;
}

bool costMetric::addProcess(process *p){

    for(unsigned i = 0; i < components.size(); i++){
       if(p == components[i])
	 return false;
    }
    components += p;
    sampleInfo *s = aggSample.newComponent();
    parts += s;
    lastProcessTime += timeStamp::ts1970();
    cumulative_values += pdSample(0);
    return true;
}

bool costMetric::removeProcess(process *p){
    unsigned size = components.size();
    for(unsigned i = 0; i < size; i++){
       if(p == components[i]){
	   //sampleInfo *olddata = parts[i];
	   aggSample.removeComponent(parts[i]);
	   for(unsigned j = i; j < size-1; j++){
	       components[j] = components[j+1];
	       parts[j] = parts[j+1];
	       lastProcessTime[j] = lastProcessTime[j+1];
	       cumulative_values[j] = cumulative_values[j+1];
	   }
	   components.resize(size -1);
	   parts.resize(size -1);
	   lastProcessTime.resize(size -1);
	   cumulative_values.resize(size -1);
	   // delete olddata;
	   assert(components.size() == parts.size());
	   assert(lastProcessTime.size() == parts.size());
	   assert(cumulative_values.size() == parts.size());
	   return true;
       }
    }
    return false;
}


bool costMetric::addProcessToAll(process *p){

    for(unsigned i=0; i < allCostMetrics.size(); i++){
        costMetric *next_met = allCostMetrics[i];
	next_met->addProcess(p);
    }
    return true;
}

bool costMetric::removeProcessFromAll(process *p){
    for(unsigned i=0; i < allCostMetrics.size(); i++){
        costMetric *next_met = allCostMetrics[i];
	next_met->removeProcess(p);
    }
    return true;
}


sampleInterval costMetricValueUpdate(costMetric *met, process *proc,
		    pdSample value, timeStamp endTime, timeStamp processTime) {
    sampleInterval ret(timeStamp::ts1970(), timeStamp::ts1970(),
		       pdSample::Zero());
    ret.valid = false;
    int proc_num = -1;
    for(unsigned i=0; i < met->components.size(); i++){
        if(proc == met->components[i]) proc_num = i;
    }
    if(proc_num == -1) return ret;
    
    met->lastProcessTime[proc_num] = processTime;

    // currently all cost metrics are EventCounters
    if(met->style_ == EventCounter){
        // only use delta from last sample
        if (value < met->cumulative_values[proc_num]) {
	  sampleVal_cerr << "value < met->cumulative_values[proc_num]: " 
	       << met->cumulative_values[proc_num] << "\n";
#ifdef ndef
            if ((value/met->cumulative_values[proc_num]) < 0.99999) {
		char buffer[200];
		sprintf(buffer,"value = %f cumulative_value = %f proc = %d met = %s\n",
			value,met->cumulative_values[proc_num],proc_num,
			met->getName());
                logLine(buffer);
	        assert((value + 0.0001)  >= met->cumulative_values[proc_num]);
	    } else {
	       // floating point rounding error ignore
	       met->cumulative_values[proc_num] = value;
	    }
#endif
            value = met->cumulative_values[proc_num];
	}
	value -= met->cumulative_values[proc_num];
	met->cumulative_values[proc_num] += value;
    }
    // update the sample value associated with the process proc
    if((met->parts[proc_num])->firstValueReceived()){
        // ret = (met->parts[proc_num])->newValue(endTime,value);
        (met->parts[proc_num])->newValue(endTime,value);
    }
    else {
        // this is the first sample. Since we don't have the start time for this
        // sample, we have to loose it, but we set the start time here.
        // ret = (met->parts[proc_num])->startTime(endTime);
        // (met->parts[proc_num])->value = value;

//        (met->parts[proc_num])->startTime(endTime);
        (met->parts[proc_num])->firstTimeAndValue(endTime, value);
    }
    // update value if we have a new value from each part
    // ret = met->sample.newValue(met->parts, endTime, value);
    // if(ret.valid) met->sample.value = ret.value;
    ret = met->aggSample.aggregateValues();
    sampleVal_cerr << "cmvu- ret.end: " << ret.end << "  ret.start: "
		   << ret.start << "  ret.value: " << ret.value
		   << "  ret.valid: " << ret.valid << "\n";
    if(ret.valid) met->cumulativeValue = ret.value;
    return ret;
}

void costMetric::updateValue(process *proc, pdSample value, timeStamp endTime, 
			     timeStamp processTime) {
    sampleInterval ret = costMetricValueUpdate(this, proc, value,
					       endTime, processTime); 
    if (node && ret.valid) {
	// kludge to fix negative time from CM5 
	if (ret.start < timeStamp::ts1970()) ret.start = timeStamp::ts1970();
	assert(ret.end >= timeStamp::ts1970());
	assert(ret.end >= ret.start);
	timeLength timeSpan = ret.end - ret.start;
	int64_t span_ns = timeSpan.getI(timeUnit::ns());
	int64_t rval = ret.value.getValue();
	sampleVal_cerr << "ret.end: " << ret.end << "  ret.start: " 
		       << ret.start << "  span_ns: " 
		       << span_ns << "  rval: " << rval << "\n";
	if (span_ns - rval > 0) {
	  double adjRatio = static_cast<double>(span_ns) / (span_ns - rval);
	  ret.value.assign(static_cast<int64_t>(span_ns * adjRatio));
	  node->forwardSimpleValue(ret.start, ret.end, ret.value, 1, true);
	}
    }
}

