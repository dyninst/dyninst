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

// $Id: DMmetricFocusReq.h,v

#ifndef __METRIC_FOCUS_REQ_H__
#define __METRIC_FOCUS_REQ_H__


#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "dyninstRPC.xdr.CLNT.h"
#include "DMinclude.h"



class metricFocusReq;
class metricFocusReqBundle;
class metricInstance;
class paradynDaemon;

inline unsigned ui_hash_(const unsigned &u) { return u; }

class metricFocusReq_Val {
   static dictionary_hash<unsigned, metricFocusReq_Val*> allMetricFocusReqVals;

   pdvector<metricFocusReq *> parent_list;

   int num_daemons;
   metricInstance *mi;

   inst_insert_result_t cur_overall_state;
   // indexed by daemon id

   // we don't initialize the dmn_state_buf, because it would take too
   // long if there were a large number of daemons
   // in the future, if it becomes important to store around the daemon ids
   // associated with this metricFocusReq_Val, then we could store them
   // in an array in this class
   // For now, the bundle will save around the daemons which it will use
   // to make the request on each daemon.  Here, in the metricFocusReq_Val,
   // we'll just store how many daemons there are.  This will allow us
   // to determine when we've received all the requests we're waiting for.
   mutable dictionary_hash<unsigned, inst_insert_result_t> dmn_state_buf;
   string error_msg;  // only valid if in failure state
   bool request_sent_to_daemon;

   inst_insert_result_t calculateNewOverallState() const;
   void getBundleClients(pdvector<metricFocusReqBundle *> *bundleClnts) const;
   void makeMetricInstInfo(metricInstInfo *mi_response);
   void propagateToDaemon(paradynDaemon *dmn);

 public:
   static metricFocusReq_Val *lookup_mfReqVal(metricHandle mh) {
      metricFocusReq_Val *new_mfVal;
      bool foundIt = allMetricFocusReqVals.find(mh, new_mfVal);
      if(! foundIt)
         new_mfVal = NULL;
      
      return new_mfVal;
   }
   static void cancelOutstandingMetricFocusesInCurrentPhase();
   static void attachToOutstandingRequest(metricInstance *mi, 
                                          paradynDaemon *dmn);

   metricFocusReq_Val(metricInstance *mi_, int num_daemons_);
   ~metricFocusReq_Val();

   int getNumDaemons() const { return num_daemons; }
   metricInstance *getMetricInst() const { return mi; }

   bool requestSentToDaemon()    { return request_sent_to_daemon; }
   void setRequestSentToDaemon() { request_sent_to_daemon = true; }

   void addParent(metricFocusReq *mfReq);
   void removeParent(metricFocusReq *mfReq);

   inst_insert_result_t getOverallState() const {
      return cur_overall_state;
   }

   inst_insert_result_t getCurrentDmnState(unsigned daemon_id) const {
      inst_insert_result_t state;
      bool foundIt;
      foundIt = dmn_state_buf.find(daemon_id, state);
      if(! foundIt) {
         dmn_state_buf[daemon_id] = inst_insert_unknown;
         state = inst_insert_unknown;
      }
      return state;
   }

   void setNewDmnState(unsigned daemon_id, inst_insert_result_t new_state,
                       string errmsg = ""); //errmsg only used in failure state

   void readyMetricInstanceForSampling();
   void accumulatePerfStreamMsgs();
   void flushPerfStreamMsgs();
};


class metricFocusReq {
   metricFocusReq_Val &V;
   metricFocusReqBundle &parent;
   
   metricFocusReq(metricFocusReq_Val *metFocVal, metricFocusReqBundle *parent_)
      : V(*metFocVal), parent(*parent_) { }

 public:
   static metricFocusReq *createMetricFocusReq(metricInstance *mi, 
                                               int num_daemons,
                                               metricFocusReqBundle *parent_);
   ~metricFocusReq();

   metricInstance *getMetricInst() const {
      return V.getMetricInst();
   }
   metricFocusReqBundle *getParentBundle() const { return &parent; }

   bool requestSentToDaemon()    { return V.requestSentToDaemon(); }
   void setRequestSentToDaemon() { V.setRequestSentToDaemon(); }

   inst_insert_result_t getOverallState() const {
      return V.getOverallState();
   }

   inst_insert_result_t getCurrentDmnState(unsigned daemon_id) const {
      return V.getCurrentDmnState(daemon_id);
   }

   void setNewDmnState(unsigned daemon_id, inst_insert_result_t new_state,
                       string errmsg = "") //errmsg only used in failure state
   {
      V.setNewDmnState(daemon_id, new_state, errmsg);
   }
   void readyMetricInstanceForSampling() {
      V.readyMetricInstanceForSampling();
   }
   void accumulatePerfStreamMsgs() {
      V.accumulatePerfStreamMsgs();
   }
   void flushPerfStreamMsgs() {
      V.flushPerfStreamMsgs();
   }
};


const char *state_str(inst_insert_result_t st);


#endif
