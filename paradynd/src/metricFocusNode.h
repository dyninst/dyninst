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

// $Id: metricFocusNode.h,v 1.102 2003/05/23 07:28:05 pcroth Exp $ 

#ifndef METRIC_H
#define METRIC_H

#include "common/h/String.h"
// trace data streams
#include "common/h/Vector.h"
#include "pdutil/h/sampleAggregator.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "rtinst/h/trace.h"
#include "dyninstAPI/src/inst.h" // for "enum callWhen"
#include "dyninstRPC.xdr.SRVR.h" // for flag_cons
#include "common/h/Time.h"
#include "pdutil/h/metricStyle.h"
#include "pdutil/h/ByteArray.h"
#include "common/h/Dictionary.h"

class threadMetFocusNode_Val;

class instInstance; // enough since we only use instInstance* in this file
class pd_process;
class pd_thread;


/* ************************************************************************ */

class processMetFocusNode;
class threadMetFocusNode;
class instrCodeNode;

class metricFocusNode {
friend timeLength guessCost(string& metric_name, pdvector<u_int>& focus) ;

#if defined(MT_THREAD)
friend bool checkMetricMIPrimitives(string metric_flat_name, 
				    instrCodeNode *& metric_prim,
				    string name, 
				    pdvector< pdvector<string> >& comp_focus, 
				    int processIdx);
#endif

public:
  // NON_MT_THREAD version:
  // for primitive (real non-aggregate, per constraint var or metric var) mdn's
  // flat name should include process id
  //
  // for component (per-process) (non-aggregate, now aggregate) mdn's
  // difference: it now has parts too (become aggregate)

  metricFocusNode();

  // NON_MT_THREAD version:
  // for aggregate (not component) mdn's

  virtual ~metricFocusNode() { };

  virtual void print() { };

  // propagate this aggregate mi to a newly started process p (not for
  // processes started via fork or exec, just for those started "normally")
  static void handleNewProcess(process *p);
  static void handleExitedProcess(pd_process *p);

  static void handleFork(const pd_process *parent, pd_process *child);
     // called once per fork.  "map" maps all instInstance's of the parent
     // process to the corresponding copy in the child process...we'll delete
     // some instrumentation in the child process if we find that some
     // instrumentation in the parent doesn't belong in the child.

  static void handleExec(pd_process *);
     // called once per exec, once the "new" process has been bootstrapped.
     // We decide which mi's that were present in the pre-exec process should
     // be carried over to the new process.  For those that should, the
     // instrumentation is actually inserted.  For others, the component mi
     // in question is removed from the system.

  static void handleNewThread(pd_process *proc, pd_thread *thr);
  static void handleExitedThread(pd_process *proc, pd_thread *thr);

protected:
  // Since we don't define these, make sure they're not used:
  metricFocusNode &operator=(const metricFocusNode &src);
};


class metFocInstResponse : public T_dyninstRPC::instResponse
{
 public:
   metFocInstResponse(int request_id_, int daemon_id_) :
      T_dyninstRPC::instResponse( request_id_, daemon_id_ )
   {}

   bool hasResponse( void ) const           { return (rinfo.size() > 0); }

   void addResponse( unsigned int mi_id,
                        inst_insert_result_t res,
                        string emsg = "" );

   void updateResponse( unsigned int mi_id,
                        inst_insert_result_t res,
                        string emsg = "" );

   void makeCallback( void );
};

#if defined(MT_THREAD)
extern dictionary_hash<string, metricFocusNode*> allMIinstalled;
#endif

// don't access this directly, consider private
extern timeLength currentPredictedCost;

// Access currentPredictedCost through these functions, should probably
// be included in some class or namespace in the future
inline timeLength &getCurrentPredictedCost() {
  return currentPredictedCost;
}
inline void setCurrentPredictedCost(const timeLength &tl) {
  currentPredictedCost = tl;
}
inline void addCurrentPredictedCost(const timeLength &tl) {
  currentPredictedCost += tl;
}
inline void subCurrentPredictedCost(const timeLength &tl) {
  currentPredictedCost -= tl;
}

extern void reportInternalMetrics(bool force);

/*
 * Routines to control data collection.
 *
 * syncMode       - whether satisfying a synchronous or asynchronous request
 * focus		- a list of resources
 * metricName		- what metric to collect data for
 * id                   - metric id
 * procsToContinue      - a list of processes that had to be stopped to insert
 *                        instrumentation. The caller must continue these processes.
 */
void startCollecting(string& metricName, pdvector<u_int>& focus,
                                        int mid, 
                                        metFocInstResponse* cbi );

/*
 * Return the expected cost of collecting performance data for a single
 *    metric at a given focus.  The value returned is the fraction of
 *    perturbation expected (i.e. 0.10 == 10% slow down expected).
 */
timeLength guessCost(string& metric_name, pdvector<u_int>& focus);

void flush_batch_buffer();
void batchSampleData(string metname, int mid, timeStamp startTimeStamp, 
		     timeStamp endTimeStamp, pdSample value);

/*
bool AstNode_condMatch(AstNode* a1, AstNode* a2,
		       pdvector<dataReqNode*> &data_tuple1, // initialization?
		       pdvector<dataReqNode*> &data_tuple2,
		       pdvector<dataReqNode*> datareqs1,
		       pdvector<dataReqNode*> datareqs2);
*/
#endif


// need to make dataReqNode return their type info


 
