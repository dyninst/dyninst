/*
 * Copyright (c) 1993, 1994 Barton P. Miller, Jeff Hollingsworth,
 *     Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, Karen
 *     Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.
 *
 * This software is furnished under the condition that it may not be
 * provided or otherwise made available to, or used by, any other
 * person, except as provided for by the terms of applicable license
 * agreements.  No title to or ownership of the software is hereby
 * transferred.  The name of the principals may not be used in any
 * advertising or publicity related to this software without specific,
 * written prior authorization.  Any use of this software must include
 * the above copyright notice.
 *
 */
#include "DMphase.h"
#include "DMperfstream.h"
#include "DMmetric.h"
#include "util/h/hist.h"

phaseInfo::phaseInfo(timeStamp s, timeStamp e, timeStamp b, const string &n){

    startTime = s;
    endTime = e;
    bucketWidth = b;
    handle = dm_phases.size();
    if(!n.string_of()){
        char temp[20];
        sprintf(temp,"%s%d","phase_",handle);
        name = string(temp);
    }
    else
        name = n;
    phaseInfo *p = this;
    dm_phases += p;
}

void phaseInfo::setLastEndTime(timeStamp stop_time){
   unsigned size = dm_phases.size();
   if(size > 0){
       (dm_phases[size-1])->SetEndTime(stop_time);
   }
}

void phaseInfo::startPhase(timeStamp start_Time, const string &name){

    // TODO: change sampling rate (???)  if this is changed on fold
    // events (it is not currently) it should be changed here as well 

    phaseHandle lastId =  phaseInfo::CurrentPhaseHandle();
    // create a new phaseInfo object 
    timeStamp bin_width = (Histogram::getMinBucketWidth());
    timeStamp start_time = Histogram::currentTime();

    // set the end time for the curr. phase
    phaseInfo::setLastEndTime(start_time);
    phaseInfo *p = new phaseInfo(start_time, (timeStamp)-1.0, bin_width, name);
    assert(p);
    
    // update MI's current phase info 
    metricInstance::setPhaseId(p->GetPhaseHandle());
    metricInstance::SetCurrWidth(bin_width);

    // disable all currPhase data collection if persistent_collection flag
    // clear, and remove all currPhase data if persistent_data flag clear
    metricInstance::stopAllCurrentDataCollection(lastId);

    // invoke newPhaseCallback for all perf. streams
    dictionary_hash_iter<perfStreamHandle,performanceStream*>
			allS(performanceStream::allStreams);
     perfStreamHandle h;
     performanceStream *ps;
     while(allS.next(h,ps)){
         ps->callPhaseFunc(*p);
     }
     p = 0;
}	

// caller is responsible for freeing space assoc. with return list
vector<T_visi::phase_info> *phaseInfo::GetAllPhaseInfo(){

    vector<T_visi::phase_info> *phase_list = new vector<T_visi::phase_info>;
    T_visi::phase_info newValue;

    for(unsigned i=0; i < dm_phases.size(); i++){
	newValue.start = dm_phases[i]->GetStartTime();
	newValue.end = dm_phases[i]->GetEndTime();
	newValue.bucketWidth = dm_phases[i]->GetBucketWidth();
	newValue.handle = dm_phases[i]->GetPhaseHandle();
	newValue.name = dm_phases[i]->PhaseName();
	*phase_list += newValue;
    }
    assert(phase_list->size() == dm_phases.size());
    return(phase_list);
}

// returns the start time of the last defined phase
timeStamp phaseInfo::GetLastPhaseStart(){

  unsigned size = dm_phases.size(); 
  if (size == 0) return -1;
  return(dm_phases[size-1]->GetStartTime());

}

// returns handle of the last defined phase
phaseHandle phaseInfo::CurrentPhaseHandle(){

  unsigned size = dm_phases.size(); 
  if (size == 0) return 0;
  return(dm_phases[size-1]->GetPhaseHandle());
}

timeStamp phaseInfo::GetLastBucketWidth(){

  unsigned size = dm_phases.size(); 
  if (size == 0) return -1;
  return(dm_phases[size-1]->GetBucketWidth());

}

void phaseInfo::setCurrentBucketWidth(timeStamp new_width){

  unsigned size = dm_phases.size(); 
  if (size == 0) return;
  dm_phases[size-1]->ChangeBucketWidth(new_width);

}
