#include "paradynd/src/costmetrics.h"
#include "paradynd/src/process.h"

vector<costMetric*> costMetric::allCostMetrics;

costMetric::costMetric(const string n,
		     metricStyle style,
		     int a,
		     const string units,
		     im_pred_struct& preds,
		     bool developerMode,
		     daemon_MetUnitsType unitstype,
		     int combiner_op){


   name_ = n; style_ = style; agg_ = a; units_ = units; pred = preds; 
   developermode_ = developerMode; unitstype_ = unitstype;
   node = 0; past_head = 0; smooth_sample = 0.0;
   if((combiner_op >= 0) && (combiner_op <= 3)) 
       combinerop_ = combiner_op;
   else combinerop_ = -1;

   for(unsigned i = 0; i < PAST_LIMIT; i++){
       past[i] = 0.0;
   }
   for(unsigned i2 = 0; i2 < processVec.size(); i2++){
       components += processVec[i2];
       sampleInfo *s = new sampleInfo;
       if(combiner_op != -1)
           s->aggOp = combiner_op;
       else
           s->aggOp = agg_;
       parts += s;
       lastProcessTime += 0.0; 
       cumulative_values += 0.0;
   }
   // how the deamons combine values from diff processes
   // this may be different from the agg_ param. which tells paradyn
   // how to combine the values from different daemons processes
   if(combiner_op != -1)
       sample.aggOp = combiner_op;
   else
       sample.aggOp = agg_;

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
    switch (focus[resource::machine].size()) {
        case 1: break;
        case 2:
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
    switch (focus[resource::process].size()) {
	case 1: break;
	case 2:
	    switch(pred.process) {
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
    sampleInfo *s = new sampleInfo;
    parts += s;
    lastProcessTime += 0.0;
    cumulative_values += 0.0;
    return true;
}

bool costMetric::removeProcess(process *p){
    unsigned size = components.size();
    for(unsigned i = 0; i < size; i++){
       if(p == components[i]){
	   sampleInfo *olddata = parts[i];
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
	   delete olddata;
	   assert(components.size() == parts.size());
	   assert(lastProcessTime.size() == parts.size());
	   assert(cumulative_values.size() == parts.size());
           char buffer[200];
           sprintf(buffer, "costMetric::removeProcess: components.size = %d\n",
           components.size());
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


sampleInterval costMetricValueUpdate(costMetric *met,
				     process *proc,
		     	   	     sampleValue value,
		  	   	     timeStamp endTime,
	       		   	     timeStamp processTime){

    sampleInterval ret;
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
        ret = (met->parts[proc_num])->newValue(endTime,value);
    }
    else {
        // this is the first sample. Since we don't have the start time for this
        // sample, we have to loose it, but we set the start time here.
        ret = (met->parts[proc_num])->startTime(endTime);
	(met->parts[proc_num])->value = value;
    }
    // update value if we have a new value from each part
    ret = met->sample.newValue(met->parts, endTime, value);
    if(ret.valid) met->sample.value = ret.value;
    return ret;
}


void costMetric::updateValue(process *proc,
			     sampleValue value,
			     timeStamp endTime,
			     timeStamp processTime){

    sampleInterval ret = costMetricValueUpdate(this,proc,value,
					       endTime,processTime); 
    if (node && ret.valid) {
	// kludge to fix negative time from CM5 
	if (ret.start < 0.0) ret.start = 0.0;
	assert(ret.end >= 0.0);
	assert(ret.end >= ret.start);
	node->forwardSimpleValue(ret.start,ret.end,ret.value,1,true);
    }
}



void costMetric::updateSmoothValue(process *proc,
			     sampleValue newValue,
			     timeStamp endTime,
			     timeStamp processTime){

    sampleInterval ret = costMetricValueUpdate(this,proc,newValue,
					       endTime,processTime); 

    if(ret.valid){ // there is a new value for this interval
	// compute smoothed value 
	unsigned count = 0;
	smooth_sample = 0.0;
	timeStamp length = ret.end - ret.start;
	if(length > 0.0){
	    sampleValue temp = ret.value/length;
	    for(unsigned i=0; i < PAST_LIMIT; i++){
	        if(past[i] != 0.0){
	            temp += past[i];	
		    count++;
	    } }
            smooth_sample = temp / (1.0*(count + 1));
	    // add the newest value to the past list
	    past[past_head++] = smooth_sample;  // time normalized value
	    if(past_head >= PAST_LIMIT) past_head = 0;
	    // compute value for the appropriate time interval
	    smooth_sample *= length;
	}
	if(node){  // actually send the value
	    // kludge to fix negative time from CM5 
	    if (ret.start < 0.0) ret.start = 0.0;
	    assert(ret.end >= 0.0);
	    assert(ret.end >= ret.start);
	    node->forwardSimpleValue(ret.start,ret.end,smooth_sample,1,true);
	}
    }
}
