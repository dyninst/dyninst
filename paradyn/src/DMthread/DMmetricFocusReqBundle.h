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

// $Id: DMmetricFocusReqBundle.h,v

#ifndef __METRIC_FOCUS_REQ_BUNDLE_H__
#define __METRIC_FOCUS_REQ_BUNDLE_H__

#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/String.h"
#include "DMinclude.h"
#include "dyninstRPC.xdr.CLNT.h"

class paradynDaemon;
class metricInstance;
class metricFocusReq;

inline unsigned ui_hash___(const unsigned &u) { return u; }


class metricFocusReqBundle {
   static pdvector<metricFocusReqBundle *> all_mfReq_bundles;
   static int current_request_id;

   // indexed by handle
   dictionary_hash<unsigned, metricFocusReq *> mfRequests;
   // for making the actual request
   pdvector<paradynDaemon *> *daemons_requested_on;

   perfStreamHandle ps_handle;  // client thread
   perfStreamHandle pt_handle;  // client thread for traces
   phaseType ph_type;           // phase type assoc. with enable request
   phaseHandle ph_handle;       // phase id, used if request is for curr phase
   unsigned request_id;            // DM assigned enable request identifier
   unsigned client_id;             // enable request id sent by calling thread

   unsigned num_daemons;              // number of daemons 
   unsigned persistent_data;
   unsigned persistent_collection;
   unsigned phase_persistent_data;

   pdvector<metricInstInfo> metricInstInfoBuf;  // gets sent to perfStreams

   metricFocusReqBundle(perfStreamHandle ps_handle_,
                        perfStreamHandle pt_handle_, phaseType type,
                        phaseHandle phaseID,
                        unsigned requestID, unsigned clientID,
                        const pdvector<metricInstance *> &miVec,
                        pdvector<paradynDaemon *> *matchingDaemons_,
                        unsigned persistence_data_,
                        unsigned persistent_collection_,
                        unsigned phase_persistent_data_);

   metricFocusReq *getMetricFocusReq(unsigned handle);
   void update_mfReq_states(const T_dyninstRPC::instResponse &resp,
                            pdvector<metricFocusReq *> *mfReqsThatChanged);
   bool isBundleComplete() const;
   pdvector<metricInstInfo *> makePerfStreamMsgPacket();

 public:   
   static metricFocusReqBundle *createMetricFocusReqBundle(
                            perfStreamHandle ps_handle_,
                            perfStreamHandle pt_handle_, phaseType type,
                            phaseHandle phaseID,
                            unsigned clientID,
                            const pdvector<metricInstance *> &miVec,
                            pdvector<paradynDaemon *> *matchingDaemons_,
                            unsigned persistence_data_,
                            unsigned persistent_collection_,
                            unsigned phase_persistent_data_);

   static metricFocusReqBundle *findActiveBundle(int request_id) {
      return all_mfReq_bundles[request_id];
   }

   ~metricFocusReqBundle() {
      all_mfReq_bundles[request_id] = NULL;
   }

   phaseType getPhaseType() { return ph_type; }

   void enableWithDaemons();
   void updateWithEnableCallback(const T_dyninstRPC::instResponse &resp);

   void readyMetricInstanceForSampling(metricInstance *mi);
   void accumulatePerfStreamMsgs(const metricInstInfo &new_miInfo);
   void flushPerfStreamMsgs();

};



#endif

