/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) /p/paradyn/CVSROOT/core/paradynd/src/metric.C,v 1.52 1995/05/18 10:38:42 markc Exp";
#endif

/*
 * metric.C - define and create metrics.
 *
 * $Log: metricFocusNode.C,v $
 * Revision 1.66  1996/01/29 20:16:32  newhall
 * added enum type "daemon_MetUnitsType" for internal metric definition
 * changed bucketWidth internal metric to EventCounter
 *
 * Revision 1.65  1995/12/28  23:42:29  zhichen
 * Added buffering to the paradynd --> paradyn interface
 * calls to tp->sampleDataCallbackFunc replaced with calls to the
 * local routine batchSampleData() which in turn occasionally calls
 * the new igen routine tp->batchSampleDataCallbackFunc().
 * Related buffer variables (theBatchBuffer, batch_buffer_next,
 * BURST_HAS_COMPLETED) are new.
 *
 * Revision 1.64  1995/12/20 23:48:43  mjrg
 * Stopped delivery of values from internal metrics when the application is paused.
 *
 * Revision 1.63  1995/12/18 23:27:04  newhall
 * changed metric's units type to have one of three values (normalized,
 * unnormalized, or sampled)
 *
 * Revision 1.62  1995/12/15 14:40:54  naim
 * Changing "hybrid_cost" by "smooth_obs_cost" - naim
 *
 * Revision 1.61  1995/11/30  16:55:04  naim
 * Fixing bug related to SampledFunction for internal metrics - naim
 *
 * Revision 1.60  1995/11/29  00:27:42  tamches
 * made getCurrentTime less susceptible to roundoff errors
 * reduced warnings with g++ 2.7.1
 *
 * Revision 1.59  1995/11/18 18:19:33  krisna
 * const this cannot be put into container of non-consts
 *
 * Revision 1.58  1995/11/17 17:24:31  newhall
 * support for MDL "unitsType" option, added normalized member to metric class
 *
 * Revision 1.57  1995/11/13  14:54:12  naim
 * Adding "mode" option to the Metric Description Language to allow specificacion
 * of developer mode for metrics (default mode is "normal") - naim
 *
 * Revision 1.56  1995/10/16  13:56:34  naim
 * Eliminating error message 65. It seems to be not necessary - naim
 *
 * Revision 1.55  1995/10/04  18:52:47  krisna
 * for-loop-scope change
 *
 * Revision 1.54  1995/09/26 20:17:50  naim
 * Adding error messages using showErrorCallback function for paradynd
 *
 * Revision 1.53  1995/08/24  15:04:18  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.52  1995/05/18  10:38:42  markc
 * Removed class metric
 *
 * Revision 1.51  1995/03/10  19:33:54  hollings
 * Fixed several aspects realted to the cost model:
 *     track the cost of the base tramp not just mini-tramps
 *     correctly handle inst cost greater than an imm format on sparc
 *     print starts at end of pvm apps.
 *     added option to read a file with more accurate data for predicted cost.
 *
 * Revision 1.50  1995/02/26  22:46:45  markc
 * Commented code that needs to be reexamined.
 *
 * Revision 1.49  1995/02/16  08:53:42  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.48  1995/02/16  08:33:45  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.47  1995/01/30  17:32:10  jcargill
 * changes for gcc-2.6.3; intCounter was both a typedef and an enum constant
 *
 * Revision 1.46  1994/11/11  10:17:47  jcargill
 * "Fixed" pause_time definition for CM5
 *
 * Revision 1.45  1994/11/11  05:11:06  markc
 * Turned off print message when internal metrics are enbled.
 *
 * Revision 1.44  1994/11/10  21:03:42  markc
 * metricValue gets intialized to 0.
 *
 * Revision 1.43  1994/11/10  18:58:06  jcargill
 * The "Don't Blame Me Either" commit
 *
 * Revision 1.42  1994/11/09  18:40:14  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.41  1994/11/02  11:10:59  markc
 * Attempted to clean up metric instrumentation requests with classes.
 * Removed string handles.
 *
 * Revision 1.40  1994/09/30  19:47:09  rbi
 * Basic instrumentation for CMFortran
 *
 * Revision 1.39  1994/09/22  02:13:35  markc
 * cast args to memset
 * cast stringHandles for string functions
 * change *allocs to news
 *
 * Revision 1.38  1994/09/20  18:18:28  hollings
 * added code to use actual clock speed for cost model numbers.
 *
 * Revision 1.37  1994/09/05  20:33:34  jcargill
 * Bug fix:  enabling certain metrics could cause no instrumentation to be
 * inserted, but still return a mid; this hosed the PC
 *
 * Revision 1.36  1994/08/17  16:43:32  markc
 * Removed early return from metricDefinitionNode::insertInstrumentation which
 * prevented instrumentation from being inserted if the application was
 * running when the request was made.
 *
 * Revision 1.35  1994/08/08  20:13:43  hollings
 * Added suppress instrumentation command.
 *
 * Revision 1.34  1994/08/02  18:22:55  hollings
 * Changed comparison for samples going backwards to use a ratio rather than
 * an absolute fudge factor.  This is required due to floating point rounding
 * errors on large numbers.
 *
 * Revision 1.33  1994/07/28  22:40:42  krisna
 * changed definitions/declarations of xalloc functions to conform to alloc.
 *
 * Revision 1.32  1994/07/26  19:58:35  hollings
 * added CMMDhostless variable.
 *
 * Revision 1.31  1994/07/22  19:20:12  hollings
 * moved computePauseTimeMetric to machine specific area.
 *
 * Revision 1.30  1994/07/21  01:34:19  hollings
 * Fixed to skip over null point and ast nodes for addInst calls.
 *
 * Revision 1.29  1994/07/20  18:21:55  rbi
 * Removed annoying printf
 *
 * Revision 1.28  1994/07/16  03:38:48  hollings
 * fixed stats to not devidi by 1meg, fixed negative time problem.
 *
 * Revision 1.27  1994/07/15  20:22:05  hollings
 * fixed 64 bit record to be 32 bits.
 *
 * Revision 1.26  1994/07/14  23:30:29  hollings
 * Hybrid cost model added.
 *
 * Revision 1.25  1994/07/14  14:39:17  jcargill
 * Removed call to flushPtrace
 *
 * Revision 1.24  1994/07/12  20:13:58  jcargill
 * Fixed logLine for printing out samples w/64 bit time
 *
 * Revision 1.23  1994/07/05  03:26:09  hollings
 * observed cost model
 *
 * Revision 1.22  1994/07/02  01:46:41  markc
 * Use aggregation operator defines from util/h/aggregation.h
 * Changed average aggregations to summations.
 *
 * Revision 1.21  1994/06/29  02:52:36  hollings
 * Added metricDefs-common.{C,h}
 * Added module level performance data
 * cleanedup types of inferrior addresses instrumentation defintions
 * added firewalls for large branch displacements due to text+data over 2meg.
 * assorted bug fixes.
 *
 * Revision 1.20  1994/06/27  18:56:56  hollings
 * removed printfs.  Now use logLine so it works in the remote case.
 * added internalMetric class.
 * added extra paramter to metric info for aggregation.
 *
 * Revision 1.19  1994/06/22  01:43:16  markc
 * Removed warnings.  Changed bcopy in inst-sparc.C to memcpy.  Changed 
 * process.C reference to proc->status to use proc->heap->status.
 *
 * Revision 1.18  1994/06/02  23:27:57  markc
 * Replaced references to igen generated class to a new class derived from
 * this class to implement error handling for igen code.
 *
 * Revision 1.17  1994/05/31  19:53:50  markc
 * Fixed pause time bug which was causing negative values to be reported.  The
 * fix involved adding an extra test in computePauseTimeMetric that did not
 * begin reporting pause times until firstSampleReceived is true.
 *
 * Revision 1.16  1994/05/31  19:16:17  markc
 * Commented out assert test for elapsed.
 *
 * Revision 1.15  1994/05/31  18:14:18  markc
 * Modified check for covered less than rather than not equal.  This is a short
 * term fix.
 *
 * Revision 1.14  1994/05/03  05:07:24  markc
 * Removed comment on pauseMetric for paradyndPVM.
 *
 * Revision 1.13  1994/04/15  15:37:34  jcargill
 * Removed duplicate definition of pauseTimeNode; used initialized version
 *
 * Revision 1.12  1994/04/13  16:48:10  hollings
 * fixed pause_time to work with multiple processes/node.
 *
 * Revision 1.11  1994/04/13  03:09:00  markc
 * Turned off pause_metric reporting for paradyndPVM because the metricDefNode is
 * not setup properly.  Updated inst-pvm.C and metricDefs-pvm.C to reflect changes
 * in cm5 versions.
 *
 * Revision 1.10  1994/04/12  15:29:20  hollings
 * Added samplingRate as a global set by an RPC call to control sampling
 * rates.
 *
 * Revision 1.9  1994/04/11  23:25:22  hollings
 * Added pause_time metric.
 *
 * Revision 1.8  1994/04/07  00:37:53  markc
 * Checked for NULL metric instance returned from createMetricInstance.
 *
 * Revision 1.7  1994/04/01  20:06:42  hollings
 * Added ability to start remote paradynd's
 *
 * Revision 1.6  1994/03/26  19:31:36  hollings
 * Changed sample time to be consistant.
 *
 * Revision 1.5  1994/03/24  16:41:59  hollings
 * Moved sample aggregation to lib/util (so paradyn could use it).
 *
 * Revision 1.4  1994/03/01  21:23:58  hollings
 * removed unused now variable.
 *
 * Revision 1.3  1994/02/24  04:32:34  markc
 * Changed header files to reflect igen changes.  main.C does not look at the number of command line arguments now.
 *
 * Revision 1.2  1994/02/01  18:46:52  hollings
 * Changes for adding perfConsult thread.
 *
 * Revision 1.1  1994/01/27  20:31:28  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.17  1994/01/20  17:47:16  hollings
 * moved getMetricValue to this file.
 *
 * Revision 1.16  1993/12/15  21:02:42  hollings
 * added PVM support.
 *
 * Revision 1.15  1993/12/13  19:55:16  hollings
 * counter operations.
 *
 * Revision 1.14  1993/10/19  15:27:54  hollings
 * AST based mini-tramp code generator.
 *
 * Revision 1.13  1993/10/07  19:42:43  jcargill
 * Added true combines for global instrumentation
 *
 * Revision 1.12  1993/10/01  21:29:41  hollings
 * Added resource discovery and filters.
 *
 * Revision 1.11  1993/09/03  18:36:16  hollings
 * removed extra printfs.
 *
 * Revision 1.10  1993/09/03  16:01:56  hollings
 * removed extra printf.
 *
 * Revision 1.9  1993/09/03  15:45:51  hollings
 * eat the first sample from an aggregate to get a good start interval time.
 *
 * Revision 1.8  1993/08/20  22:00:51  hollings
 * added getMetricValue for controller.
 * moved insertInstrumentation for predicates to before pred users.
 *
 * Revision 1.7  1993/08/16  16:24:50  hollings
 * commented out elapsedPauseTime because of CM-5 problems with node processes.
 * removed many debugging printfs.
 *
 * Revision 1.6  1993/08/11  01:52:12  hollings
 * chages for new build before use mode.
 *
 * Revision 1.5  1993/07/13  18:28:30  hollings
 * new include file syntax.
 *
 * Revision 1.4  1993/06/24  16:18:06  hollings
 * global fixes.
 *
 * Revision 1.3  1993/06/22  19:00:01  hollings
 * global inst state.
 *
 * Revision 1.2  1993/06/08  20:14:34  hollings
 * state prior to bc net ptrace replacement.
 *
 * Revision 1.1  1993/03/19  22:45:45  hollings
 * Initial revision
 *
 *
 */

#include "util/h/headers.h"
#include <assert.h>

#include "metric.h"
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "util/h/aggregateSample.h"
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "dyninstP.h"
#include "ast.h"
#include "util.h"
#include "comm.h"
#include "internalMetrics.h"
#include <strstream.h>
#include "util/h/list.h"
#include "init.h"
#include "perfStream.h"
#include "main.h"
#include "stats.h"
#include "dynrpc.h"
#include "paradynd/src/mdld.h"
#include "util/h/Timer.h"
#include "paradynd/src/mdld.h"
#include "showerror.h"

double currentPredictedCost = 0.0;
double currentSmoothObsValue= 0.0;

dictionary_hash <unsigned, metricDefinitionNode*> midToMiMap(uiHash);

unsigned mdnHash(const metricDefinitionNode *&mdn) {
  return ((unsigned) mdn);
}

dictionary_hash<unsigned, metricDefinitionNode*> allMIs(uiHash);
vector<internalMetric*> internalMetric::allInternalMetrics;

// used to indicate the mi is no longer used.
#define DELETED_MI 1
#define MILLION 1000000.0

bool mdl_internal_metric_data(string& metric_name, mdl_inst_data& result) {
  unsigned size = internalMetric::allInternalMetrics.size();
  for (unsigned u=0; u<size; u++) 
    if (internalMetric::allInternalMetrics[u]->name() == metric_name) {
      result.aggregate = internalMetric::allInternalMetrics[u]->aggregate();
      result.style = internalMetric::allInternalMetrics[u]->style();
      return true;
    }
  return (mdl_metric_data(metric_name, result));
}

metricDefinitionNode::metricDefinitionNode(process *p, string& met_name, 
                                           vector< vector<string> >& foc,
                                           string& cat_name, int agg_style)
: aggregate_(false), inserted_(false), met_(met_name), focus_(foc), flat_name_(cat_name),
  sample(agg_style), id_(-1), originalCost_(0.0), inform_(false), proc_(p)
{
  mdl_inst_data md;
  assert(mdl_internal_metric_data(met_name, md));
  style_ = md.style;
}

float metricDefinitionNode::getMetricValue()
{
    float total;

    if (aggregate_) {
        total = 0.0;
        unsigned c_size = components.size();
        for (unsigned u=0; u<c_size; u++)
          total += components[u]->getMetricValue();
        return(0.0);
    }
    return (data[0]->getMetricValue());
}

metricDefinitionNode::metricDefinitionNode(string& metric_name,
                                           vector< vector<string> >& foc,
                                           string& cat_name, 
                                           vector<metricDefinitionNode*>& parts)
: aggregate_(true), inserted_(false), met_(metric_name), focus_(foc),
  flat_name_(cat_name), components(parts),
  id_(-1), originalCost_(0.0), inform_(false), proc_(NULL)
{
  // TODO -- does this work 
  mdl_inst_data md;
  assert(mdl_metric_data(metric_name, md));
  style_ = md.style;
  sample = md.aggregate;
  unsigned p_size = parts.size();
  for (unsigned u=0; u<p_size; u++) {
    metricDefinitionNode *mi = parts[u];
    metricDefinitionNode *t = (metricDefinitionNode *) this;
    mi->aggregators += t;
    // parts[u]->aggregators += this;
    valueList.add(&parts[u]->sample);
  }
}

float getProcessCount() {  return ((float) processVec.size()); }

// check for "special" metrics that are computed directly by paradynd 
// if a cost of an internal metric is asked for, enable=false
metricDefinitionNode *doInternalMetric(vector< vector<string> >& canon_focus,
                                       string& metric_name, string& flat_name,
                                       bool enable, bool& matched)
{
  matched = false;
  unsigned im_size = internalMetric::allInternalMetrics.size();
  for (unsigned im_index=0; im_index<im_size; im_index++)
    if (internalMetric::allInternalMetrics[im_index]->name() == metric_name) {
      matched = true;
      if (!enable) return NULL;
      internalMetric *im = internalMetric::allInternalMetrics[im_index];
      if (!im->legalToInst(canon_focus)) return NULL;
      metricDefinitionNode *mn = new metricDefinitionNode(NULL, metric_name,
                                                          canon_focus, 
                                                          flat_name, im->aggregate());
      im->enable(mn);
      return(mn);
    }
  return NULL;
}

/*
 * See if we can find the requested metric instance.
 *   Currently this is only used to cache structs built for cost requests 
 *   which are then instantiated.  This could be used as a general system
 *   to request find sub-metrics that are already.defines and use them to
 *   reduce cost.  This would require adding the componenets of an aggregate
 *   into the allMIs list since searching tends to be top down, not bottom
 *   up.  This would also require adding a ref count to many of the structures
 *   so they only get deleted when we are really done with them.
 *
 */

metricDefinitionNode *createMetricInstance(string& metric_name, vector<u_int>& focus,
                                           bool enable, bool& internal)
{
    metricDefinitionNode *mi= NULL;

    // first see if it is already defined.
    dictionary_hash_iter<unsigned, metricDefinitionNode*> mdi(allMIs);
    unsigned u;

    vector< vector<string> > string_foc;
    if (!resource::foc_to_strings(string_foc, focus)) return NULL;
    vector< vector<string> > canon_focus;
    resource::make_canonical(string_foc, canon_focus);
    string flat_name(metric_name);

    unsigned cf_size = canon_focus.size();
    for (u=0; u<cf_size; u++) {
      unsigned v_size = canon_focus[u].size();
      for (unsigned v=0; v<v_size; v++) 
        flat_name += canon_focus[u][v];
    }

    // TODO -- a dictionary search here will be much faster
    while (mdi.next(u, mi))
      if (mi->getFullName() == flat_name)
        return mi;

    // KLUDGE ALERT
    // Catch complex metrics that are currently beyond the mdl's power
    if (metric_name == "observed_cost") {
      mi = mdl_observed_cost(canon_focus, metric_name, flat_name);
    } else if (mdl_can_do(metric_name)) {
      mi = mdl_do(canon_focus, metric_name, flat_name);
    } else {
      bool matched;
      mi = doInternalMetric(canon_focus, metric_name, flat_name, enable, matched);
      internal = true;
    }
    return(mi);
}

int startCollecting(string& metric_name, vector<u_int>& focus, int id) 
{
    // TODO -- why is this here?
    if (CMMDhostless == true) return(-1);

    static int MICount=0;
    bool internal = false;

    // Make the unique ID for this metric/focus visible in MDL.
    string vname = "$globalId";
    mdl_env::add(vname, false, MDL_T_INT);
    mdl_env::set(id, vname);

    metricDefinitionNode *mi = createMetricInstance(metric_name, focus,
                                                    true, internal);
    
    if (!mi) return(-1);
    mi->id_ = ++MICount;
    allMIs[mi->id_] = mi;

    float cost = mi->cost();
    mi->originalCost_ = cost;

    currentPredictedCost += cost;
    if (!internal) mi->insertInstrumentation();
    metResPairsEnabled++;
    return(mi->id_);
}

float guessCost(string& metric_name, vector<u_int>& focus)
{
    float cost;
    metricDefinitionNode *mi;
    bool internal;
    mi = createMetricInstance(metric_name, focus, false, internal);
    if (!mi) return(0.0);

    cost = mi->cost();
    return(cost);
}

bool metricDefinitionNode::insertInstrumentation()
{
    bool needToCont = false;

    if (inserted_) return(true);

    /* check all proceses are in an ok state */

    if (!isApplicationPaused()) {
        pauseAllProcesses();
        needToCont = true;
    }

    inserted_ = true;
    if (aggregate_) {
        unsigned c_size = components.size();
        for (unsigned u=0; u<c_size; u++)
          components[u]->insertInstrumentation();
    } else {
      unsigned size = data.size();
      for (unsigned u=0; u<size; u++)
        data[u]->insertInstrumentation(this);
      size = requests.size();
      for (unsigned u1=0; u1<size; u1++)
        requests[u1]->insertInstrumentation();
    }
    if (needToCont) continueAllProcesses();
    return(true);
}

float metricDefinitionNode::cost()
{
    float ret;
    float nc;

    ret = 0.0;
    if (aggregate_) {
        unsigned c_size = components.size();
        for (unsigned u=0; u<c_size; u++) {
          nc = components[u]->cost();
          if (nc > ret) ret = nc;
        }
    } else {
      unsigned size = requests.size();
      for (unsigned u=0; u<size; u++)
        ret += requests[u]->cost();
    }
    return(ret);
}

void metricDefinitionNode::disable()
{
    // check for internal metrics

    unsigned ai_size = internalMetric::allInternalMetrics.size();
    for (unsigned u=0; u<ai_size; u++)
      if (internalMetric::allInternalMetrics[u]->node == this) {
        internalMetric::allInternalMetrics[u]->disable();
        logLine("disabled internal metric\n");
        return;
      }

    if (!inserted_) return;

    inserted_ = false;
    if (aggregate_) {
        /* disable components of aggregate metrics */
        unsigned c_size = components.size();
        for (unsigned u=0; u<c_size; u++)
          components[u]->disable();
    } else {
      unsigned size = data.size();
      for (unsigned u=0; u<size; u++)
        data[u]->disable();
      size = requests.size();
      for (unsigned u1=0; u1<size; u1++)
        requests[u1]->disable();
    }

}

metricDefinitionNode::~metricDefinitionNode()
{
    if (aggregate_) {
        /* delete components of aggregate metrics */
        unsigned c_size = components.size();
        for (unsigned u=0; u<c_size; u++)
          delete components[u];
        components.resize(0);
    } else {
      unsigned size = data.size();
      for (unsigned u=0; u<size; u++)
        delete data[u];
      size = requests.size();
      for (unsigned u1=0; u1<size; u1++)
        delete requests[u1];
    }
}

//////////////////////////////////////////////////////////////////////////////
// Buffer the samples before we actually send it                            //
//      Send it when the buffers are full                                   //
//      or, send it when the last sample in the interval has arrived.       //
//////////////////////////////////////////////////////////////////////////////

const unsigned SAMPLE_BUFFER_SIZE = 1024;
bool BURST_HAS_COMPLETED = false;
   // set to true after a burst (after a processTraceStream(), or sampleNodes for
   // the CM5), which will force the buffer to be flushed before it fills up
   // (if not, we'd have bad response time)

static vector<T_dyninstRPC::batch_buffer_entry> theBatchBuffer (SAMPLE_BUFFER_SIZE);
static unsigned int batch_buffer_next=0;

void flush_batch_buffer(int program) {
   if (batch_buffer_next == 0)
      return;

   // alloc buffer of the exact size to make communication
   // more efficient.  Why don't we send theBatchBuffer with a count?
   // This would work but would always (in the igen call) copy the entire
   // vector.  This solution has the downside of calling new but is not too bad
   // and is clean.
   vector<T_dyninstRPC::batch_buffer_entry> copyBatchBuffer(batch_buffer_next);
   for (int i=0; i< batch_buffer_next; i++)
      copyBatchBuffer[i] = theBatchBuffer[i];

  //char myLogBuffer[120] ;
  //sprintf(myLogBuffer, "in metric.C batch size about to send = %d\n", batch_buffer_next) ;
  //logLine(myLogBuffer) ;

   // Now let's do the actual igen call!
   tp->batchSampleDataCallbackFunc(program, copyBatchBuffer);

   if (BURST_HAS_COMPLETED) BURST_HAS_COMPLETED = false ;

   batch_buffer_next = 0 ;             // reset the index
}

void batchSampleData(int program, int mid, double startTimeStamp,
                     double endTimeStamp, double value) 
{
   // This routine is called where we used to call tp->sampleDataCallbackFunc.
   // We buffer things up and eventually call tp->batchSampleDataCallbackFunc

   char myLogBuffer[120] ;
#ifdef notdef
   sprintf(myLogBuffer, "mid %d, value %g\n", mid, value) ;
   logLine(myLogBuffer) ;
#endif

   // Flush the buffer if (1) it is full, or (2) for good response time, after
   // a burst of data:
   if (batch_buffer_next >= SAMPLE_BUFFER_SIZE || BURST_HAS_COMPLETED) {
      if (batch_buffer_next > 0)
         // don't need to flush if the batch had no data (this does happen; see
         // perfStream.C)
         flush_batch_buffer(program);
   }

   // Now let's batch this entry.
   T_dyninstRPC::batch_buffer_entry &theEntry = theBatchBuffer[batch_buffer_next];
   theEntry.mid = mid;
   theEntry.startTimeStamp = startTimeStamp;
   theEntry.endTimeStamp = endTimeStamp;
   theEntry.value = value;
   batch_buffer_next++;
}

void metricDefinitionNode::forwardSimpleValue(timeStamp start, timeStamp end,
                                       sampleValue value)
{
  // TODO mdc
    assert(start >= (firstRecordTime/MILLION));
    assert(end >= (firstRecordTime/MILLION));
    assert(end > start);

    batchSampleData(0, id_, start, end, value);
}

void metricDefinitionNode::updateValue(time64 wallTime, 
                                       sampleValue value)
{
    sampleInterval ret;
    // extern timeStamp elapsedPauseTime;

    // sampleTime = wallTime/ 1000000.0 - elapsedPauseTime;
    // commented out elapsedPauseTime because we don't currently stop CM-5
    // node processes. (brought it back jkh 11/9/93).
    timeStamp sampleTime = wallTime / 1000000.0; 
    assert(value >= -0.01);

// TODO mdc
//    if (!sample.firstSampleReceived) {
//      sprintf(errorLine, "First for %s:%d at %f\n", met->info.name.string_of(), id,
//            sampleTime-(firstRecordTime/MILLION));
//      logLine(errorLine);
//    }

    // TODO -- is this ok?
    // TODO -- do sampledFuncs work ?
    if (style_ == EventCounter) { 

      // only use delta from last sample.
      if (value < sample.value) {
        if ((value/sample.value) < 0.99999) {
          assert(value + 0.0001 >= sample.value);
        } else {
          // floating point rounding error ignore
          sample.value = value;
        }
      }

      //        if (value + 0.0001 < sample.value)
      //           printf ("WARNING:  sample went backwards!!!!!\n");
      value -= sample.value;
      sample.value += value;
    } 
    //
    // If style==EventCounter then value is changed. Otherwise, it keeps the
    // the current "value" (e.g. SampledFunction case). That's why it is not
    // necessary to have an special case for SampledFunction.
    //

    ret = sample.newValue(valueList, sampleTime, value);
//    if (!ret.valid && inform_) {
//       sprintf(errorLine, "Invalid for %s:%d at %f, val=%f, inform=%d\n",
//            met->info.name.string_of(), id_, sampleTime-(firstRecordTime/MILLION),
//            value, inform);
//      logLine(errorLine);
//    }

    unsigned a_size = aggregators.size();
    for (unsigned a=0; a<a_size; a++) 
      aggregators[a]->updateAggregateComponent(this, sampleTime, value);

    /* 
     * must do this after all updates are done, because it may turn off this
     *  metric instance.
     */
    if (inform_ && ret.valid) {
        /* invoke call backs */
        // assert(ret.start >= 0.0);
        // I have no idea where negative time comes from but leave it to
        // the CM-5 to create it on the first sample -- jkh 7/15/94
        // This has been solved; no sample times will be bad -- zxu
        if (ret.start < 0.0) ret.start = 0.0;
        assert(ret.end >= 0.0);
        assert(ret.end >= ret.start);

        // TODO mdc
//      assert(ret.start >= (firstRecordTime/ MILLION));
//      assert(ret.end >= (firstRecordTime/MILLION));

//        double time1 = getCurrentTime(false);

        batchSampleData(0, id_, ret.start, ret.end, ret.value);

//        double diffTime = getCurrentTime(false) - time1;
//        if (diffTime > 0.2) {
//           char buffer[200];
//           sprintf(buffer, "metricDefinitionNode::updateValue: igen call took unusually long: %g seconds\n", 
//                   diffTime);
//           logLine(buffer);
//      }
    }
}

void metricDefinitionNode::updateAggregateComponent(metricDefinitionNode *curr,
                                                    timeStamp sampleTime, 
                                                    sampleValue value)
{
    sampleInterval ret;

    ret = sample.newValue(valueList, sampleTime, value);
    if (ret.valid) {
        assert(ret.end > ret.start);
        assert(ret.start >= (firstRecordTime/MILLION));
        assert(ret.end >= (firstRecordTime/MILLION));

        batchSampleData(0, id_, ret.start, ret.end, ret.value);
    }
}

void processCost(process *proc, traceHeader *h, costUpdate *s)
{
    int i;
    double unInstTime;

    if (proc->theCost.wallTimeLastTrampSample) {
        proc->pauseTime = s->pauseTime;
/*          ((h->wall - proc->theCost.wallTimeLastTrampSample)/1000000.0); */
    }
    proc->theCost.wallTimeLastTrampSample = h->wall;

    // should really compute this on a per process basis.
    proc->theCost.currentPredictedCost = currentPredictedCost;

    // update totalPredicted cost.
    // Need to multiply by the share of the un-inst time to get pred cost
    //   for the interval.
    //   timWOinst = TwInst/(1 + predCost);
    //
    unInstTime = ((h->process - proc->theCost.timeLastTrampSample)/1000000.0)/
        (1 + proc->theCost.currentPredictedCost);
    proc->theCost.totalPredictedCost += proc->theCost.currentPredictedCost *
            unInstTime;
    proc->theCost.timeLastTrampSample = h->process;

    //
    // build circular buffer of recent values.
    //
    proc->theCost.past[proc->theCost.currentHist] = 
        (s->obsCostIdeal - proc->theCost.lastObservedCost);
    if (++proc->theCost.currentHist == HIST_LIMIT) proc->theCost.currentHist = 0;

    // now compute current value of "smooth cost";
    for (i=0, proc->theCost.smoothObsCost = 0.0; i < HIST_LIMIT; i++) {
        proc->theCost.smoothObsCost += proc->theCost.past[i];
    }
    proc->theCost.smoothObsCost /= HIST_LIMIT;

    proc->theCost.lastObservedCost = s->obsCostIdeal;

    currentSmoothObsValue = 0.0;

    unsigned size = processVec.size();
    for (unsigned u=0; u<size; u++) {
      if (processVec[u]->theCost.smoothObsCost > currentSmoothObsValue) {
        currentSmoothObsValue = processVec[u]->theCost.smoothObsCost;
      }
    }
    smooth_obs_cost->value = currentSmoothObsValue;

    //
    // update total predicted cost.
    //
    totalPredictedCost->value = 0.0;
    for (unsigned u1=0; u1<size; u1++) {
      if (processVec[u1]->theCost.totalPredictedCost > totalPredictedCost->value) {
        totalPredictedCost->value = processVec[u1]->theCost.totalPredictedCost;
      }
    }

    return;
}

void processSample(traceHeader *h, traceSample *s)
{
    unsigned mid = s->id.id;
    if (!midToMiMap.defines(mid)) {
      sprintf(errorLine, "Sample %d not for a valid metric instance\n", 
              s->id.id);
      logLine(errorLine);
      return;
    }
    metricDefinitionNode *mi = midToMiMap[mid];

    //    sprintf(errorLine, "sample id %d at time %8.6f = %f\n", s->id.id, 
    //  ((double) *(int*) &h->wall) + (*(((int*) &h->wall)+1))/1000000.0, s->value);
    //    logLine(errorLine);
    mi->updateValue(h->wall, s->value);
    samplesDelivered++;
}

/*
 * functions to operate on inst request graph.
 *
 */
instReqNode::instReqNode(process *iProc,
                         instPoint *iPoint,
                         AstNode *iAst,
                         callWhen  iWhen,
                         callOrder o) 
{
    proc = iProc;
    point = iPoint;
    ast = iAst;
    when = iWhen;
    order = o;
    instance = NULL;

    assert(proc && point);
}

bool instReqNode::insertInstrumentation()
{
    instance = addInstFunc(proc, point, ast, when, order);
    if (instance) return(true);

    return(false);
}

void instReqNode::disable()
{
    deleteInst(instance);
    instance = NULL;
}

instReqNode::~instReqNode()
{
    instance = NULL;
}

float instReqNode::cost()
{
    float value;
    float unitCost;
    float frequency;
    int unitCostInCycles;

    unitCostInCycles = ast->cost() + getPointCost(proc, point) +
        getInsnCost(trampPreamble) + getInsnCost(trampTrailer);
    // printf("unit cost = %d cycles\n", unitCostInCycles);
    unitCost = unitCostInCycles/ cyclesPerSecond;
    frequency = getPointFrequency(point);
    value = unitCost * frequency;

    return(value);
}

dataReqNode::dataReqNode(dataObjectType iType,
                      process *iProc,
                      int iInitialValue,
                      bool iReport,
                      timerType iTType) 
{
    type = iType;
    proc = iProc;
    initialValue = iInitialValue;
    report = iReport;
    tType = iTType;
    instance = NULL;
};

float dataReqNode::getMetricValue()
{
    float ret;

    if (type == INTCOUNTER) {
        ret = getIntCounterValue((intCounterHandle*) instance);
    } else if (type == TIMER) {
        ret = getTimerValue((timerHandle*) instance);
    } else {
        // unknown type.
        abort();
        return(0.0);
    }
    return(ret);
}

unsigned dataReqNode::getInferiorPtr() 
{
    unsigned param=0;
    timerHandle *timerInst;
    intCounterHandle *counterInst;

    if (type == INTCOUNTER) {
        counterInst = (intCounterHandle *) instance;
        if (counterInst) {
            param = (unsigned) counterInst->counterPtr;
        } else {
            param = 0;
        }
    } else if (type == TIMER) {
        timerInst = (timerHandle *) instance;
        if (timerInst) {
            param = (unsigned) timerInst->timerPtr;
        } else {
            param = 0;
        }
    } else {
        abort();
    }
    return(param);
}

timerHandle *dataReqNode::createTimerInstance()
{
    timerHandle *ret;

    ret = createTimer(proc, tType, report);
    return(ret);
}

intCounterHandle *dataReqNode::createCounterInstance()
{
    intCounterHandle *ret;

    ret = createIntCounter(proc, initialValue, report);
    return(ret);
}

// allow a global "variable" to be inserted
// this will not report any values
// it is used internally by generated code -- see metricDefs-pvm.C
void dataReqNode::insertGlobal() {
  if (type == INTCOUNTER) {
    intCounterHandle *ret;
    ret = createCounterInstance();
    instance = (void *) ret;
    id = ret->data.id;
  } else 
    abort();
}

void dataReqNode::insertInstrumentation(metricDefinitionNode *mi) 
{
    if (type == INTCOUNTER) {
        intCounterHandle *ret;
        ret = createCounterInstance();
        instance = (void *) ret;
        id = ret->data.id;
        unsigned mid = id.id;
        midToMiMap[mid] = mi;
    } else {
        timerHandle *ret;
        ret = createTimerInstance();
        instance = (void *) ret;
        id = ret->data.id;
        unsigned mid = id.id;
        midToMiMap[mid] = mi;
    }
}

void dataReqNode::disable()
{
    if (!instance) return;

    unsigned mid = id.id;
    if (!midToMiMap.defines(mid))
      abort();
    midToMiMap.undef(mid);

    if (type == TIMER) {
        freeTimer((timerHandle *) instance);
    } else if (type == INTCOUNTER) {
        freeIntCounter((intCounterHandle *) instance);
    } else {
        abort();
    }
    instance = NULL;
}

dataReqNode::~dataReqNode()
{
    instance = NULL;
}

bool internalMetric::legalToInst(vector< vector<string> >& focus) {
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

internalMetric *internalMetric::newInternalMetric(const string n,
                                                  metricStyle style,
                                                  int a, 
                                                  const string units, 
                                                  sampleValueFunc f,
                                                  im_pred_struct& im_pred,
                                                  bool developerMode,
                                                  daemon_MetUnitsType unitstype) {
  internalMetric *im = new internalMetric(n, style, a, units, f, im_pred,
                                          developerMode, unitstype);
  assert(im);
  unsigned size = allInternalMetrics.size();
  for (unsigned u=0; u<size; u++)
    if (allInternalMetrics[u]->name() == n) {
      allInternalMetrics[u] = im;
      return im;
    }
  allInternalMetrics += im;
  return im;
}

timeStamp xgetCurrentTime(bool firstRecordRelative)
{
    time64 now;
    timeStamp ret;
    struct timeval tv;
    
    // TODO -- declaration for gettimeofday ?
    gettimeofday(&tv, NULL);
    now = (time64) (tv.tv_sec * MILLION) + tv.tv_usec;
    if (firstRecordRelative) now -= firstRecordTime;
    
    ret = now/MILLION;

    return(ret);
}

timeStamp getCurrentTime(bool firstRecordRelative)
{
    struct timeval tv;
    assert(gettimeofday(&tv, NULL) == 0); // 0 --> success; -1 --> error

    double seconds_dbl = tv.tv_sec * 1.0;
    assert(tv.tv_usec < 1000000);
    double useconds_dbl = tv.tv_usec * 1.0;
  
    seconds_dbl += useconds_dbl / 1000000.0;

    if (firstRecordRelative)
       seconds_dbl -= firstRecordTime;

    return seconds_dbl;    
}

void reportInternalMetrics()
{
    timeStamp now;
    timeStamp start;
    sampleValue value=0;

    static timeStamp end=0.0;

    // see if we have a sample to establish time base.
    if (!firstRecordTime) return;
    if (end==0.0)
        end = firstRecordTime/MILLION;

    now = getCurrentTime(false);

    //  check if it is time for a sample
    // We don't deliver a sample if the application is paused
    if (isApplicationPaused() || now < end + samplingRate) 
	return;

    start = end;
    end = now;

    // TODO -- clean me up, please
    unsigned ai_size = internalMetric::allInternalMetrics.size();
    for (unsigned u=0; u<ai_size; u++)
      if (internalMetric::allInternalMetrics[u]->enabled()) {
        internalMetric *imp = internalMetric::allInternalMetrics[u];

        if (imp->name() == "active_processes") {
          value = (end - start) * processVec.size();
        } else if (imp->name() == "bucket_width") {
	  value = (end - start)*(imp->value);
        } else if (imp->style() == EventCounter) {
          value = imp->getValue();
          assert(value + 0.0001 >= imp->cumulativeValue);
          value -= imp->cumulativeValue;
          imp->cumulativeValue += value;
        } else if (imp->style() == SampledFunction) {
          value = imp->getValue();
        }
        imp->node->forwardSimpleValue(start, end, value);
      }
}
