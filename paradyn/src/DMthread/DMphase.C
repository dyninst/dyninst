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

#include "DMphase.h"
#include "DMperfstream.h"
#include "DMmetric.h"
#include "pdutil/h/hist.h"

const relTimeStamp phaseInfo::histCurTime = relTimeStamp(-timeLength::sec());

phaseInfo::phaseInfo(relTimeStamp s, relTimeStamp e, timeLength b, 
		     const string &n) : 
  startTime(s), endTime(e), bucketWidth(b) 
{
    handle = dm_phases.size();
    if(!n.string_of())
      name = string("phase_") + string(handle);
    else
      name = n;
    phaseInfo *p = this;
    dm_phases += p;
    p = 0;
}

phaseInfo::~phaseInfo(){
}


void phaseInfo::setLastEndTime(relTimeStamp stop_time){
   unsigned size = dm_phases.size();
   if(size > 0){
       (dm_phases[size-1])->SetEndTime(stop_time);
   }
}

void phaseInfo::startPhase(const string &name, bool with_new_pc, 
			   bool with_visis, const relTimeStamp startTime){
    phaseHandle lastId =  phaseInfo::CurrentPhaseHandle();
    // create a new phaseInfo object 
    timeLength bin_width = (Histogram::getMinBucketWidth());
    relTimeStamp start_time = relTimeStamp::Zero();
    // histCurTime is a sentinal value passed as the default arg for startTime
    if(startTime == histCurTime) {  
      start_time = Histogram::currentTime();
    } else {
      // As of May 2001, this method was not used in Paradyn and untested
      start_time = startTime;
    }

    // set the end time for the curr. phase
    phaseInfo::setLastEndTime(start_time);
    phaseInfo *p = new phaseInfo(start_time, relTimeStamp(-1, timeUnit::sec()),
				 bin_width, name);
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
    
    // for any outstanding enables for the old current phase
    // remove from outstandingenables list and respond to all clients of
    // outstanding enable events with a failed enable response 
    if(paradynDaemon::outstanding_enables.size()){
        u_int i=0;
        bool done=false;
        while(!done){
	    DM_enableType *next_entry = paradynDaemon::outstanding_enables[i]; 
	    // this is an old curr phase enable request
	    if((next_entry->ph_type == CurrentPhase) 
	       && (next_entry->ph_handle == lastId)){
		// send enable failed response to calling thread
                vector<bool> successful(next_entry->request->size());
		for(u_int j=0; j< successful.size(); j++){
		    successful[j] = false;
		}
		DMenableResponse(*next_entry,successful);

		// make disable requests for any MI that is partly enabled
                for(u_int k=0; k < next_entry->request->size(); k++){
		    // if this entry may be partially enabled make disable
		    // call to daemons 
		    metricInstance *mi = (*next_entry->request)[k];
		    if((!(*next_entry->enabled)[k]) && mi){
		      for(u_int i2=0;i2<paradynDaemon::allDaemons.size();i2++){
			  paradynDaemon *pd = paradynDaemon::allDaemons[i2];
			  pd->disableDataCollection(mi->getHandle());
		      }
		    }
		}
		// remove entry from outstanding_enables
		u_int newsize= paradynDaemon::outstanding_enables.size()-1;
		paradynDaemon::outstanding_enables[i] = 
		     paradynDaemon::outstanding_enables[newsize];
                paradynDaemon::outstanding_enables.resize(newsize);
                delete next_entry;
		next_entry = 0;
            }
	    else{
	        i++;
	    }
	    if(i >= paradynDaemon::outstanding_enables.size()){
	        done = true;
	    }
        }
    }

    perfStreamHandle h;
    performanceStream *ps;
    while(allS.next(h,ps)){
         ps->callPhaseFunc(*p,with_new_pc,with_visis);
    }
    p = 0;
}	

// caller is responsible for freeing space assoc. with return list
vector<T_visi::phase_info> *phaseInfo::GetAllPhaseInfo(){

    vector<T_visi::phase_info> *phase_list = new vector<T_visi::phase_info>;
    T_visi::phase_info newValue;

    for(unsigned i=0; i < dm_phases.size(); i++){
	phaseInfo *next_phase = dm_phases[i];
	newValue.start = next_phase->GetStartTime().getD(timeUnit::sec());
	newValue.end = next_phase->GetEndTime().getD(timeUnit::sec());
	newValue.bucketWidth = 
	  next_phase->GetBucketWidth().getD(timeUnit::sec()); 
						 ;
	newValue.handle = next_phase->GetPhaseHandle();
	newValue.name = next_phase->PhaseName();
	*phase_list += newValue;
    }
    assert(phase_list->size() == dm_phases.size());
    return(phase_list);
}

// returns the start time of the last defined phase
relTimeStamp phaseInfo::GetLastPhaseStart(){

  unsigned size = dm_phases.size(); 
  if (size == 0) return relTimeStamp(-1, timeUnit::sec());
  return(dm_phases[size-1]->GetStartTime());

}

// returns handle of the last defined phase
phaseHandle phaseInfo::CurrentPhaseHandle(){

  unsigned size = dm_phases.size(); 
  if (size == 0) return 0;
  return(dm_phases[size-1]->GetPhaseHandle());
}

timeLength phaseInfo::GetLastBucketWidth(){

  unsigned size = dm_phases.size(); 
  if (size == 0) return timeLength(-1, timeUnit::sec());
  return(dm_phases[size-1]->GetBucketWidth());

}

void phaseInfo::setCurrentBucketWidth(timeLength new_width){

  unsigned size = dm_phases.size(); 
  if (size == 0) return;
  dm_phases[size-1]->ChangeBucketWidth(new_width);

}
