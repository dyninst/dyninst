/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: DMmetricFocusReqBundle.C,v

#include "DMmetricFocusReqBundle.h"
#include "DMmetric.h"
#include "DMperfstream.h"
#include "DMdaemon.h"
#include "DMmetricFocusReq.h"


pdvector<metricFocusReqBundle *> metricFocusReqBundle::all_mfReq_bundles;
int metricFocusReqBundle::current_request_id = 0;


metricFocusReqBundle::metricFocusReqBundle(perfStreamHandle ps_handle_,
                        perfStreamHandle pt_handle_, phaseType type,
                        phaseHandle phaseID,
                        unsigned requestID, unsigned clientID,
                        const pdvector<metricInstance *> &miVec,
                        unsigned persistence_data_,
                        unsigned persistent_collection_,
                        unsigned phase_persistent_data_) :
   mfRequests(ui_hash___), 
   ps_handle(ps_handle_), pt_handle(pt_handle_), ph_type(type),
   ph_handle(phaseID), request_id(requestID), client_id(clientID),
   persistent_data(persistence_data_),
   persistent_collection(persistent_collection_),
   phase_persistent_data(phase_persistent_data_)
{
   for(unsigned i=0; i<miVec.size(); i++) {         
      metricFocusReq *new_mfReq = 
         metricFocusReq::createMetricFocusReq(miVec[i], this);
      mfRequests[miVec[i]->getHandle()] = new_mfReq;
   }

   // If any of the metric-focuses already existed, then we're all ready to
   // go.  The createMetricFocusReq() will have already have accumulated the
   // perfStreamMsgs and setup the metricInstance for this new performance
   // stream if it's in the success state.  Now we just need to send the
   // perfStream msgs.  For example, let's say someone previously initiated
   // cpu_inclusive/cham and let's say this metric-focus is in success state.
   // Then if a user requests the same message, we can immediately set up the
   // new perfstream for this metric-focus.
   flushPerfStreamMsgs();
}

metricFocusReq *metricFocusReqBundle::getMetricFocusReq(unsigned handle) {
   metricFocusReq *mfReq;
   bool foundIt;
   foundIt = mfRequests.find(handle, mfReq);
   if(! foundIt)
      mfReq = NULL;
   return mfReq;
}


bool
metricFocusReqBundle::isBundleComplete( void ) const
{
    for( dictionary_hash<unsigned, metricFocusReq*>::iterator mfiter = 
            mfRequests.begin();
            mfiter != mfRequests.end();
            mfiter++ )
    {            
        if( !(*mfiter)->isEachDaemonComplete() )
        {
            // we have an item that is still unknown or deferred
            return false;
        }
    }
    // all responses were success or failure
    return true;
}



metricFocusReqBundle *metricFocusReqBundle::createMetricFocusReqBundle(
                            perfStreamHandle ps_handle_,
                            perfStreamHandle pt_handle_, phaseType type,
                            phaseHandle phaseID,
                            unsigned clientID,
                            const pdvector<metricInstance *> &miVec,
                            unsigned persistence_data_,
                            unsigned persistent_collection_,
                            unsigned phase_persistent_data_) {
   current_request_id++;
   metricFocusReqBundle *newBundle = 
      new metricFocusReqBundle(ps_handle_, pt_handle_, type, phaseID,
                               current_request_id, clientID, miVec,
                               persistence_data_,
                               persistent_collection_, phase_persistent_data_);
   
   // allocate up to the index
   while(all_mfReq_bundles.size() < ((unsigned)current_request_id) + 1)
      all_mfReq_bundles.push_back(NULL);

   all_mfReq_bundles[current_request_id] = newBundle;
   return newBundle;
}

void metricFocusReqBundle::readyMetricInstanceForSampling(metricInstance *mi) {
   extern void histDataCallBack(pdSample *buckets, relTimeStamp, int count, 
                                int first, void *callbackData);
   extern void histFoldCallBack(const timeLength *_width,
                                void *callbackData);
   extern void traceDataCallBack(const void*, int, void*);

   mi->setEnabled();
         
   if(ph_type == CurrentPhase){
      unsigned old_current = mi->currUsersCount();
      bool  current_data = mi->isCurrHistogram();
      mi->newCurrDataCollection(histDataCallBack,   histFoldCallBack);
      mi->newGlobalDataCollection(histDataCallBack, histFoldCallBack);
      mi->addCurrentUser(ps_handle);
         
      // trace data streams
      mi->newTraceDataCollection(traceDataCallBack);
      mi->addTraceUser(pt_handle);

      // set sample rate to match current phase hist. bucket width
      if(!metricInstance::numCurrHists()){
         timeLength rate = phaseInfo::GetLastBucketWidth();
         newSampleRate(rate);
      }
      // new active curr. histogram added if there are no previous
      // curr. subscribers and either persistent_collection is clear
      // or there was no curr. histogram prior to this
      if((!old_current) && (mi->currUsersCount() == 1) && 
         (!(mi->isCollectionPersistent()) || (!current_data))) {
         metricInstance::incrNumCurrHists();
      }
      // new global histogram if this metricInstance was just enabled
      //         if(!((*enable.enabled)[enableIndex])){
      metricInstance::incrNumGlobalHists();  // ????
      //         }
   }
   else {  // this is a global phase enable
      mi->newGlobalDataCollection(histDataCallBack, histFoldCallBack);
      mi->addGlobalUser(ps_handle);
         
      // trace data streams
      mi->newTraceDataCollection(traceDataCallBack);
      mi->addTraceUser(pt_handle);

      // if this is first global histogram enabled and there are no
      // curr hists, then set sample rate to global bucket width
      if(!metricInstance::numCurrHists()){
         if(!metricInstance::numGlobalHists()){
            timeLength rate = Histogram::getGlobalBucketWidth();
            newSampleRate(rate);
         }
      }
      // new global hist added: update count
      //if(!((*enable.enabled)[enableIndex])){
      metricInstance::incrNumGlobalHists();  //  ????
      //}
   }

   // update the persistence flags: the OR of new & previous values
   if(persistent_data) {
      mi->setPersistentData();
   }
   if(persistent_collection){
      mi->setPersistentCollection();
   }
   if(phase_persistent_data){
      mi->setPhasePersistentData();
   }

   // if all daemons have responded update state for request and send
   // result to caller
   // a successful enable has both the enabled flag set and an mi*
      
   // clear currentlyEnabling flag and decrement the count of 
   // waiting enables for all MI's
   mi->clearCurrentlyEnabling();
   if(ph_type == CurrentPhase)
      mi->decrCurrWaiting();
   else
      mi->decrGlobalWaiting();
}

void metricFocusReqBundle::accumulatePerfStreamMsgs(
                                             const metricInstInfo &new_miInfo)
{
    metricInstInfoBuf.push_back(new_miInfo);
}

void metricFocusReqBundle::flushPerfStreamMsgs() {
   if(metricInstInfoBuf.size() == 0)
      return;   // already flushed

   // Is deleted by the visi thread or pc thread when it's done with
   // the vector of metricInstInfo's.  See
   //    VISIthreadmain.C / VISIthreadEnableCallback()  and
   //    PCfilter.C / filteredDataServer::newDataEnabled()
   pdvector<metricInstInfo> *psPacket = new pdvector<metricInstInfo>;
   for(unsigned i=0; i<metricInstInfoBuf.size(); i++)
      (*psPacket).push_back(metricInstInfoBuf[i]);

   bool is_last_of_perfstream_msgs = isBundleComplete();

   // make response call
   performanceStream::psIter_t allS = 
      performanceStream::getAllStreamsIter();
   perfStreamHandle h;
   performanceStream *ps;

   bool sentOnStream = false;
   while(allS.next(h,ps)) {
      if(h == (perfStreamHandle)(ps_handle)) {
         ps->callDataEnableFunc(psPacket, client_id,
                                is_last_of_perfstream_msgs);
         sentOnStream = true;
      }
   }
      
   // trace data streams
   if(! sentOnStream) {
      // I don't know why the perfstream msgs can only be sent either
      // "traced" or "non-traced", but that's what the existing code does
      allS.reset();
      while(allS.next(h,ps)){
         if(h == (perfStreamHandle)(pt_handle)) {
            ps->callDataEnableFunc(psPacket, client_id,
                                   is_last_of_perfstream_msgs);
            sentOnStream = true;
         }
      }
   }

   if(sentOnStream) {
      metricInstInfoBuf.clear();
   }
   else {
      // emulate the deletion that VISIthreadEnableCallback would do
      delete psPacket;
   }
}

typedef enum {  unset, report_transition, dont_report_transition,
                unexpected_transition } state_response_t;

state_response_t state_transition_response(inst_insert_result_t old_state,
                                           inst_insert_result_t new_state)
{
   state_response_t resp = unset;

   switch(new_state) {
     case inst_insert_failure:
        switch(old_state) {
          case inst_insert_failure:
             resp = dont_report_transition;  break;
          case inst_insert_success:
             resp = unexpected_transition;   break;
          case inst_insert_deferred:
             resp = report_transition;       break;
          case inst_insert_unknown:
             resp = report_transition;       break;
        }
        break;
     case inst_insert_success:
        switch(old_state) {
          case inst_insert_failure:
             resp = unexpected_transition;   break;
          case inst_insert_success:
             resp = dont_report_transition;  break;
          case inst_insert_deferred:
             resp = report_transition;       break;
          case inst_insert_unknown:
             resp = report_transition;       break;
        }
        break;
     case inst_insert_deferred:
        switch(old_state) {
          case inst_insert_failure:
             resp = unexpected_transition;   break;
          case inst_insert_success:
             resp = unexpected_transition;   break;
          case inst_insert_deferred:
             resp = dont_report_transition;  break;
          case inst_insert_unknown:
             resp = report_transition;       break;
        }
        break;
     case inst_insert_unknown:
        switch(old_state) {
          case inst_insert_failure:
             resp = unexpected_transition;   break;
          case inst_insert_success:
             resp = unexpected_transition;   break;
          case inst_insert_deferred:
             resp = unexpected_transition;   break;
          case inst_insert_unknown:
             resp = dont_report_transition;  break;
        }
   }

   assert(resp != unset);
   return resp;
}

void warn_unexpected_individual_state_transition(
                                             metricFocusReq *cur_mfReq,
                                             inst_insert_result_t prior_state,
                                             inst_insert_result_t new_state,
                                             unsigned daemon_id)
{
   paradynDaemon *pd = paradynDaemon::getDaemonById(daemon_id);
   cerr << " PARADYN: unexpected state transition ("
        << state_str(prior_state) << " -> "
        << state_str(new_state) << ")\n"
        << "          for metric-focus "
        << cur_mfReq->getMetricInst()->getMetricName() << " / "
        << cur_mfReq->getMetricInst()->getFocusName() << endl;
   if(pd) 
      cerr << "          from response from machine: " 
           << pd->getMachineName() << endl;   
}

void metricFocusReqBundle::update_mfReq_states(
                         const T_dyninstRPC::instResponse &resp,
                         pdvector<metricFocusReq *> *mfReqsToReport)
{
   // update this request's responses with the new response
   for(unsigned i = 0; i < resp.rinfo.size(); i++)
   {
      const T_dyninstRPC::indivInstResponse& curMI_resp = resp.rinfo[i];
      metricInstanceHandle mh = curMI_resp.mi_id;
      metricFocusReq *cur_mfReq = getMetricFocusReq(mh);
      assert(cur_mfReq != NULL);
      //inst_insert_result_t prior_overall_state =
      //   cur_mfReq->getOverallState();

      inst_insert_result_t current_dmn_state =
         cur_mfReq->getCurrentDmnState(resp.daemon_id);
      inst_insert_result_t new_dmn_state =
         inst_insert_result_t(curMI_resp.status);

      unsigned daemon_id = resp.daemon_id;
      state_response_t trans_resp =
         state_transition_response(current_dmn_state, new_dmn_state);

      if(trans_resp == dont_report_transition)
         continue;  // this response must have been already handled
      else if(trans_resp == unexpected_transition) {
         warn_unexpected_individual_state_transition(cur_mfReq,
                               current_dmn_state, new_dmn_state, daemon_id);
         continue;
      }

      cur_mfReq->setNewDmnState(daemon_id, new_dmn_state,curMI_resp.emsg);
      inst_insert_result_t new_overall_state = cur_mfReq->getOverallState();

      if(cur_mfReq->should_report_new_state(new_overall_state))
         (*mfReqsToReport).push_back(cur_mfReq);
   }
}

void metricFocusReqBundle::updateWithEnableCallback(
                                     const T_dyninstRPC::instResponse &resp)
{
   pdvector<metricFocusReq *> mfReqsToReport;
   update_mfReq_states(resp, &mfReqsToReport);

   for(unsigned i=0; i<mfReqsToReport.size(); i++) {
      metricFocusReq *cur_mfReq = mfReqsToReport[i];
      if(cur_mfReq->getOverallState() == inst_insert_success)
         cur_mfReq->readyMetricInstanceForSampling();

      cur_mfReq->accumulatePerfStreamMsgs();
   }
   for(unsigned j=0; j<mfReqsToReport.size(); j++) {
      metricFocusReq *cur_mfReq = mfReqsToReport[j];
      cur_mfReq->flushPerfStreamMsgs();
   }

   if(isBundleComplete()) {
       delete this;
   }
}

static unsigned hashfunc(const unsigned &k) { return (k % 101); }

void metricFocusReqBundle::enableWithDaemons() {
  dictionary_hash<unsigned, metricFocusReq *>::iterator mfiter =
    mfRequests.begin();

  //These are stored per-daemon, by the daemon ID
  // So metric_ids[5] would be the metric ids for daemon 5
  dictionary_hash<unsigned, pdvector<unsigned> > metric_ids(hashfunc);
  dictionary_hash<unsigned, pdvector<T_dyninstRPC::focusStruct> > foci(hashfunc);
  dictionary_hash<unsigned, pdvector<pdstring> > metric_names(hashfunc);

  for (; mfiter != mfRequests.end(); mfiter++)
  {
    metricFocusReq *cur_mfReq = (*mfiter);
    if(cur_mfReq->requestSentToDaemon())
      continue;

    metricInstance *cur_mi = cur_mfReq->getMetricInst();
    pdvector<paradynDaemon *> &daemons = cur_mfReq->requestedDaemons();
    for (unsigned i=0; i<daemons.size(); i++)
    {
      unsigned daemon_id = daemons[i]->get_id();

      metric_ids[daemon_id].push_back(cur_mi->getHandle());

      T_dyninstRPC::focusStruct focus;
      bool result = cur_mi->convertToIDList(focus.focus);
      assert(result);
      foci[daemon_id].push_back(focus);
      
      metric_names[daemon_id].push_back(cur_mi->getMetricName());
      
      cur_mi->setCurrentlyEnabling();
      cur_mfReq->setRequestSentToDaemon();
    }
  }

  dictionary_hash<unsigned, pdvector<unsigned> >::iterator diter = 
    metric_ids.begin();
  for(; diter != metric_ids.end(); diter++)
  {
    unsigned pd_id = diter.currkey();
    paradynDaemon *pd = paradynDaemon::getDaemonById(pd_id);
    pd->enableDataCollection(foci[pd_id], metric_names[pd_id], metric_ids[pd_id],
			     pd_id, request_id);
  }
}
