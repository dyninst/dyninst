/*
 * Copyright (c) 1996-2003 Barton P. Miller
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

// $Id: DMmetricFocusReq.C,v

#include "DMmetricFocusReq.h"
#include "DMmetricFocusReqBundle.h"
#include "DMdaemon.h"
#include "DMmetric.h"


// ========   metricFocusReq_Val   ========

dictionary_hash<unsigned, metricFocusReq_Val*> metricFocusReq_Val::allMetricFocusReqVals(ui_hash_);


metricFocusReq_Val::metricFocusReq_Val(metricInstance *mi_, int num_daemons_) :
   num_daemons(num_daemons_), mi(mi_),
   cur_overall_state(inst_insert_unknown), dmn_state_buf(ui_hash_),
   error_msg(""), request_sent_to_daemon(false)
{
   have_reported_state[inst_insert_deferred] = false;
   have_reported_state[inst_insert_success]  = false;   
   have_reported_state[inst_insert_failure]  = false;   
   metricFocusReq_Val::allMetricFocusReqVals[mi->getHandle()] = this;
}

metricFocusReq_Val::~metricFocusReq_Val() {
   metricFocusReq_Val::allMetricFocusReqVals.undef(
                                                getMetricInst()->getHandle());
}

inst_insert_result_t metricFocusReq_Val::calculateNewOverallState() const {
   dictionary_hash<unsigned, inst_insert_result_t>::iterator state_iter =
      dmn_state_buf.begin();
   
   bool exists_a_deferred = false;
   bool exists_a_failure = false;
   bool all_are_success = true;
   int num_successes = 0;
   while(state_iter != dmn_state_buf.end()) {
      inst_insert_result_t cur_state = (*state_iter);
      if(cur_state == inst_insert_deferred) {
         exists_a_deferred = true;
         all_are_success = false;
      } else if(cur_state == inst_insert_failure) {
         exists_a_failure = true;
         all_are_success = false;
      } else if(cur_state == inst_insert_unknown)
         all_are_success = false;
      else {
         assert(cur_state == inst_insert_success);
         num_successes++;
      }

      state_iter++;
   }

   if(exists_a_failure) {
      return inst_insert_failure;
   } else if(exists_a_deferred) {
      return inst_insert_deferred;
   } else if(all_are_success && num_successes == num_daemons)
      return inst_insert_success;
   else
      return inst_insert_unknown;
}

bool
metricFocusReq_Val::isEachDaemonComplete( void ) const
{
    unsigned int nComplete = 0;
    for( dictionary_hash<unsigned, inst_insert_result_t>::iterator iter =
                dmn_state_buf.begin();
            iter != dmn_state_buf.end();
            iter++ )
    {
        inst_insert_result_t curState = (*iter);
        if( (curState == inst_insert_success) ||
            (curState == inst_insert_failure) )
        {
            nComplete++;
        }
    }
    return (nComplete == (unsigned int)num_daemons);
}


void metricFocusReq_Val::setNewDmnState(unsigned daemon_id, 
                                        inst_insert_result_t new_state,
                                        //errmsg only used in failure state
                                        string errmsg)  
{
   dmn_state_buf[daemon_id] = new_state;
   cur_overall_state = calculateNewOverallState();

   if(new_state == inst_insert_success) {
      paradynDaemon *pd = paradynDaemon::getDaemonById(daemon_id);
      component *comp = new component(pd, mi->getHandle(), mi);
      bool aflag;
      aflag=(mi->addComponent(comp));
      assert(aflag);
   } else if(new_state == inst_insert_failure ) {
      error_msg = errmsg;
   }
}

void metricFocusReq_Val::getBundleClients(
                         pdvector<metricFocusReqBundle *> *bundleClients) const
{
   // metricFocusReq_Vals probably won't have a large number of parent
   // bundles so this shouldn't take all that long
   // there shouldn't be a lange number of parent bundles because these
   // bundles disappear after the request is statisfied
   for(unsigned i=0; i<parent_list.size(); i++) {
      metricFocusReq *mf_req = parent_list[i];
      metricFocusReqBundle *bundle = mf_req->getParentBundle();
      (*bundleClients).push_back(bundle);
   }
}

void metricFocusReq_Val::addParent(metricFocusReq *mfReq) {
   parent_list.push_back(mfReq);
   
   inst_insert_result_t cur_state = getOverallState();

   metricFocusReqBundle *parent_bundle = mfReq->getParentBundle();
   if(cur_state == inst_insert_success) {
      parent_bundle->readyMetricInstanceForSampling(getMetricInst());
   }

   if(cur_state != inst_insert_unknown) {
      // update the new metricFocusReq with the current state
      metricInstInfo cur_mi_info;
      makeMetricInstInfo(cur_state, &cur_mi_info);
      parent_bundle->accumulatePerfStreamMsgs(cur_mi_info);
   }
}

void metricFocusReq_Val::removeParent(metricFocusReq *mfReq) {
   pdvector<metricFocusReq *>::iterator parIter = parent_list.end();
   
   while(parIter != parent_list.begin()) {
      --parIter;
      if((*parIter) == mfReq) {
         parent_list.erase(parIter);
         break;
      }
   }
}

bool metricFocusReq_Val::should_report_new_state(
                                       inst_insert_result_t new_overall_state)
{
   bool ret = false;
   switch(new_overall_state) {
     case inst_insert_failure:
        if(have_reported_state[inst_insert_failure] ||
           have_reported_state[inst_insert_success])
           ret = false;
        else
           ret = true;
        break;
     case inst_insert_success:
        if(have_reported_state[inst_insert_failure] ||
           have_reported_state[inst_insert_success])
           ret = false;
        else
           ret = true;
        break;
     case inst_insert_deferred:
        if(have_reported_state[inst_insert_failure] ||
           have_reported_state[inst_insert_success] ||
           have_reported_state[inst_insert_deferred])
        {
           ret = false;
        } else
           ret = true;
        break;
     case inst_insert_unknown:
        // don't ever want to report this state
        ret = false;
        break;
   }
   return ret;
}

void metricFocusReq_Val::readyMetricInstanceForSampling() {
   pdvector<metricFocusReqBundle *> bundleClients;
   getBundleClients(&bundleClients);

   for(unsigned i=0; i<bundleClients.size(); i++) {
      bundleClients[i]->readyMetricInstanceForSampling(getMetricInst());
   }
}

void metricFocusReq_Val::makeMetricInstInfo(inst_insert_result_t currentState,
                                            metricInstInfo *mi_response) {
   // should be impossible to make a performance stream message
   // that indicates an unknown state
   assert(currentState != inst_insert_unknown);

   metric *metricptr = metric::getMetric(mi->getMetricHandle());
   if(! metricptr) {
      cout << "mis enabled but no metric handle: " 
           << mi->getMetricHandle() << endl;
      assert(0);
   }

   mi_response->mi_id = mi->getHandle();
   mi_response->m_id = mi->getMetricHandle();
   mi_response->r_id = mi->getFocusHandle();
   mi_response->metric_name = mi->getMetricName();
   mi_response->focus_name = mi->getFocusName();
   mi_response->metric_units = metricptr->getUnits();
   mi_response->units_type = metricptr->getUnitsType();

   if(currentState == inst_insert_deferred) {
      mi_response->deferred = true;
      mi_response->successfully_enabled = false;
   } else if(currentState == inst_insert_failure) {
      mi_response->deferred = false;
      mi_response->successfully_enabled = false;
      mi_response->emsg = error_msg;
   } else if(currentState == inst_insert_success) {
      mi_response->successfully_enabled = true;
      mi_response->deferred = false;
   }
}

void metricFocusReq_Val::accumulatePerfStreamMsgs() {
   inst_insert_result_t currentState = getOverallState();
   have_reported_state[currentState] = true;

   metricInstInfo cur_mi_info;
   makeMetricInstInfo(currentState, &cur_mi_info);

   pdvector<metricFocusReqBundle *> bundleClients;
   getBundleClients(&bundleClients);

   for(unsigned i=0; i<bundleClients.size(); i++) {
                                        // cur_mi_info gets copied
      bundleClients[i]->accumulatePerfStreamMsgs(cur_mi_info);
   }
}

void metricFocusReq_Val::flushPerfStreamMsgs() {
   pdvector<metricFocusReqBundle *> bundleClients;      
   getBundleClients(&bundleClients);

   for(unsigned i=0; i<bundleClients.size(); i++) {
      bundleClients[i]->flushPerfStreamMsgs();
   }
}

void metricFocusReq_Val::propagateToDaemon(paradynDaemon *dmn) {
   inst_insert_result_t cur_state = getOverallState();
   if(cur_state == inst_insert_failure || cur_state == inst_insert_success) {
      cerr << "  PARADYN: Can't propagate metric-focus in failed or success\n"
           << "  state to new machine " << dmn->getMachineName() << endl;
      return;
   }

   num_daemons++;

   assert(cur_state == inst_insert_deferred ||
          cur_state == inst_insert_unknown);

   // yes, cur_mfReq->requestSentToDaemon() would evaluate to true here
   // because the requests were made on the daemons initially specified
   pdvector<unsigned> mi_ids;
   pdvector<T_dyninstRPC::focusStruct> foci;
   pdvector<string> metric_names;

   mi_ids.push_back(mi->getHandle());
   T_dyninstRPC::focusStruct focus;
   bool aflag = mi->convertToIDList(focus.focus);
   assert(aflag);
   foci.push_back(focus);
   metric_names.push_back(mi->getMetricName());
   
   assert(foci.size() == metric_names.size());
   assert(metric_names.size() == mi_ids.size());

   pdvector<metricFocusReqBundle *> bundleClients;
   getBundleClients(&bundleClients);

   // if it's in deferred or unknown state, there should be a connected bundle
   assert(bundleClients.size() > 0);
   // We just need a requested id for a bundle attached to this mfReqVal.
   // Doesn't matter which one.  All of the bundles will get notified
   // anyways.
   unsigned chosen_req_id = bundleClients[0]->getRequestID();

   //cerr << " request for metfocus in " << state_str(cur_state) 
   //     << " state, on machine: " << dmn->getMachineName() << endl;
   dmn->enableDataCollection(foci, metric_names, mi_ids, dmn->get_id(),
                             chosen_req_id);
}

void metricFocusReq_Val::cancelOutstandingMetricFocusesInCurrentPhase() {
   dictionary_hash<unsigned, metricFocusReq_Val*>::iterator mfIter =
      metricFocusReq_Val::allMetricFocusReqVals.begin();

   pdvector<metricFocusReq_Val *> deletedMFRVs;
   
   while(mfIter != metricFocusReq_Val::allMetricFocusReqVals.end())
   {
      metricFocusReq_Val *mfReqVal = (*mfIter);
      inst_insert_result_t cur_state = mfReqVal->getOverallState();

      if(cur_state == inst_insert_deferred ||
         cur_state == inst_insert_unknown)
      {
         // a more graceful way of reporting the failure to the visi
         // would probably be to send the cancel to the daemon
         // and wait to get the callback from the daemon about the change
         // in state in metricFocusReqBundle::updateWithEnableCallback()
         metricInstInfo cur_mi_info;
         mfReqVal->makeMetricInstInfo(inst_insert_failure, &cur_mi_info);
         // mark this metric-focus as failed
         cur_mi_info.emsg = "cancelled since starting new phase";
         
         pdvector<metricFocusReqBundle *> bundleClients;      
         mfReqVal->getBundleClients(&bundleClients);
         bool activeOnAnyGlobalPhase = false;
         for(unsigned i=0; i<bundleClients.size(); i++) {
            metricFocusReqBundle *bundle = bundleClients[i];

            if(bundle->getPhaseType() == CurrentPhase) {
               bundle->accumulatePerfStreamMsgs(cur_mi_info);
            } else
               activeOnAnyGlobalPhase = true;
         }
         mfReqVal->flushPerfStreamMsgs();

         if(! activeOnAnyGlobalPhase) {
            for(unsigned i=0; i<paradynDaemon::allDaemons.size(); i++) {
               paradynDaemon *pd = paradynDaemon::allDaemons[i];

               // This code clones that in the component destructor in DMmetric.h
               // 1) tell the daemon to disable data collection
               // 2) add the MID (mfReqVal->getMetricInst()->getHandle()) to the disabledMids list
               // 3) remove the MID from active -- note: this can be skipped, since it's not
               //    there yet (since the instru never went in)
               pd->disableDataCollection(
                                       mfReqVal->getMetricInst()->getHandle());
               pd->disabledMids += mfReqVal->getMetricInst()->getHandle();
            }
            deletedMFRVs.push_back(mfReqVal);
         }
      }
      mfIter++;
   }

   for (unsigned i = 0; i < deletedMFRVs.size(); i++)
       delete deletedMFRVs[i];
}

void metricFocusReq_Val::attachToOutstandingRequest(metricInstance *mi, 
                                                    paradynDaemon *dmn) {
   dictionary_hash<unsigned, metricFocusReq_Val*>::iterator mfIter =
      metricFocusReq_Val::allMetricFocusReqVals.begin();
   
   while(mfIter != metricFocusReq_Val::allMetricFocusReqVals.end())
   {
      metricFocusReq_Val *mfReqVal = (*mfIter);      
      if(mfReqVal->getMetricInst() == mi) {
         mfReqVal->propagateToDaemon(dmn);
      }
      mfIter++;
   }
}


// ========   metricFocusReq   ========

metricFocusReq *metricFocusReq::createMetricFocusReq(metricInstance *mi,
                                                     int num_daemons,
                                              metricFocusReqBundle *parent_) {
   metricFocusReq_Val *new_mfVal;
   new_mfVal = metricFocusReq_Val::lookup_mfReqVal(mi->getHandle());
   if(new_mfVal == NULL) {  // couldn't find an existing one
      new_mfVal = new metricFocusReq_Val(mi, num_daemons);
   }
   
   metricFocusReq *mfReq = new metricFocusReq(new_mfVal, parent_);
   new_mfVal->addParent(mfReq);
   return mfReq;
}

metricFocusReq::~metricFocusReq() {
   V.removeParent(this);
   // don't delete the metricFocusReq_Val here.  We leave it out
   // there so other metricFocusReqBundles can grab onto it and "reuse"
   // it.
}



const char *state_str(inst_insert_result_t st) {
   const char *rstr = NULL;
   switch(st) {
     case inst_insert_unknown:
        rstr = "inst_insert_unknown";
        break;
     case inst_insert_success:
        rstr = "inst_insert_success";
        break;
     case inst_insert_failure:
        rstr = "inst_insert_failure";
        break;
     case inst_insert_deferred:
        rstr = "inst_insert_deferred";
        break;
   }
   return rstr;
}


