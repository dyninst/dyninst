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

// $Id: mdl.C,v 1.94 2001/08/23 14:44:15 schendel Exp $

#include <iostream.h>
#include <stdio.h>
#include "dyninstRPC.xdr.SRVR.h"
#include "paradyn/src/met/mdl_data.h"
#include "paradynd/src/metric.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/ast.h"
#include "paradynd/src/main.h"
#include "dyninstAPI/src/symtab.h"
#include "common/h/Timer.h"
#include "paradynd/src/mdld.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/pdThread.h"
#include "common/h/debugOstream.h"
#include "pdutil/h/pdDebugOstream.h"
#include "dyninstAPI/src/instPoint.h" // new...for class instPoint

// The following vrbles were defined in process.C:
extern debug_ostream attach_cerr;
extern debug_ostream inferiorrpc_cerr;
extern debug_ostream shmsample_cerr;
extern debug_ostream forkexec_cerr;
extern pdDebug_ostream metric_cerr;


// Some global variables
static string currentMetric;  // name of the metric that is being processed.
static string daemon_flavor;
static process *global_proc = NULL;
static bool mdl_met=false, mdl_cons=false, mdl_stmt=false, mdl_libs=false;

inline unsigned ui_hash(const unsigned &u) { return u; }

// static members of mdl_env
vector<unsigned> mdl_env::frames;
vector<mdl_var> mdl_env::all_vars;

// static members of mdl_data
dictionary_hash<unsigned, vector<mdl_type_desc> > mdl_data::fields(ui_hash);
vector<mdl_focus_element> mdl_data::foci;
vector<T_dyninstRPC::mdl_stmt*> mdl_data::stmts;
vector<T_dyninstRPC::mdl_constraint*> mdl_data::all_constraints;
vector<string> mdl_data::lib_constraints;
vector<T_dyninstRPC::mdl_metric*> mdl_data::all_metrics;


#if defined(MT_THREAD)
int index_in_data(unsigned lev, unsigned ind, vector<dataReqNode*>& data) {
  int size = data.size();
  
  for (int i=0; i<size; i++) {
    if ((lev == data[i]->getAllocatedLevel())
	&& (ind == data[i]->getAllocatedIndex()))
      return i;
  }
  
  // not matched: return < 0
  return -1;
}
#else
int index_in_data(Address v, vector<dataReqNode*>& data) {
  int size = data.size();
  
  for (int i=0; i<size; i++) {
    if (v == data[i]->getInferiorPtr(global_proc))
      return i;
  }
  
  // not matched: return < 0
  return -1;
}
#endif


//
// walk_deref() is used to process the fields of the MDL_EXPR_DOT
// expression.  the MDL_EXPR_DOT expression is used to be called
// MDL_EXPR_DEREF, which is inaccurate at best. --chun
//
static bool walk_deref(mdl_var& ret, vector<unsigned>& types);

//
// these two do_operation()'s are for processing MDL_EXPR_BINOP, 
// MDL_EXPR_POSTUP, MDL_EXPR_PREUOP expressions. --chun
//
static bool do_operation(mdl_var& ret, mdl_var& left, mdl_var& right, unsigned bin_op);
static bool do_operation(mdl_var& ret, mdl_var& lval, unsigned u_op, bool is_preop);


class list_closure 
{
public:
  list_closure(string& i_name, mdl_var& list_v)
    : index_name(i_name), element_type(0), index(0), int_list(NULL), 
    float_list(NULL), string_list(NULL), bool_list(NULL), func_list(NULL), 
    funcName_list(NULL), mod_list(NULL), point_list(NULL), max_index(0)
  {
    assert (list_v.is_list());

    element_type = list_v.element_type();
    switch(element_type) 
    {
      case MDL_T_INT:
        assert (list_v.get(int_list));
        max_index = int_list->size();
        break;
      case MDL_T_FLOAT:
        assert (list_v.get(float_list));
        max_index = float_list->size();
        break;
      case MDL_T_STRING:
        assert (list_v.get(string_list));
        max_index = string_list->size();
        break;
      case MDL_T_PROCEDURE_NAME:
        assert (list_v.get(funcName_list));
        max_index = funcName_list->size();
        break;
      case MDL_T_PROCEDURE:
        assert (list_v.get(func_list));
        max_index = func_list->size();
        break;
      case MDL_T_MODULE:
        assert (list_v.get(mod_list));
        max_index = mod_list->size();
        break;
      case MDL_T_POINT:
        assert (list_v.get(point_list));
        max_index = point_list->size();
        break;
      default:
        assert(0);
    }
  }

  ~list_closure() { }

  bool next() 
  {
    string s;
    function_base *pdf; module *m;
    float f; int i;
    instPoint *ip;

    if (index >= max_index) return false;
    switch(element_type) 
    {
      case MDL_T_INT:
        i = (*int_list)[index++];
        return (mdl_env::set(i, index_name));
      case MDL_T_FLOAT:
        f = (*float_list)[index++];
        return (mdl_env::set(f, index_name));
      case MDL_T_STRING:
        s = (*string_list)[index++];
        return (mdl_env::set(s, index_name));
      case MDL_T_PROCEDURE_NAME:
        // lookup-up the functions defined in resource lists
        // the function may not exist in the image, in which case we get the
        // next one
        do 
        {
          functionName *fn = (*funcName_list)[index++];
          pdf = global_proc->findOneFunction(fn->get());
        }
        while (pdf == NULL && index < max_index);
        if (pdf == NULL)
          return false;
        return (mdl_env::set(pdf, index_name));
      case MDL_T_PROCEDURE:
        pdf = (*func_list)[index++];
        assert(pdf);
        return (mdl_env::set(pdf, index_name));
      case MDL_T_MODULE: 
        m = (*mod_list)[index++];
        assert (m);
        return (mdl_env::set(m, index_name));
      case MDL_T_POINT:
        ip = (*point_list)[index++];
        assert (ip);
        return (mdl_env::set(ip, index_name));
      default:
        assert(0);
    }
    return false;
  }

private:
  string index_name;
  unsigned element_type;
  unsigned index;
  vector<int> *int_list;
  vector<float> *float_list;
  vector<string> *string_list;
  vector<bool> *bool_list;
  vector<function_base*> *func_list;
  vector<functionName*> *funcName_list;
  vector<module*> *mod_list;
  vector<instPoint*> *point_list;
  unsigned max_index;
};

//
// Since the metric, constraints, etc. are manufactured in paradyn and
// then shipped to daemons over the RPC, the non-noarg constructors of 
// mdl_metric, mdl_constraint, mdl_instr_stmt, etc. and their destructors
// shoudln't be called here, rather, the paradyn's version should be
// called.  But I could be wrong.  Put assert(0)'s to verify this; didn't
// encounter any problem.  Should any problem occur in the future, the
// assert's need to be removed.  --chun
//
T_dyninstRPC::mdl_metric::mdl_metric(string id, string name, string units, 
				    u_int agg, u_int sty, u_int type,
				    vector<T_dyninstRPC::mdl_stmt*> *mv, 
				    vector<string> *flav,
				    vector<T_dyninstRPC::mdl_constraint*> *cons,
				    vector<string> *temp_counters,
				    bool developerMode,
				    int unitstype)
: id_(id), name_(name), units_(units), agg_op_(agg), style_(sty),
  type_(type), stmts_(mv), flavors_(flav), constraints_(cons),
  temp_ctr_(temp_counters), developerMode_(developerMode),
  unitstype_(unitstype) { assert(0); }

T_dyninstRPC::mdl_metric::mdl_metric()
: id_(""), name_(""), units_(""), agg_op_(0), style_(0), type_(0), 
  stmts_(NULL), flavors_(NULL), constraints_(NULL), temp_ctr_(NULL),
  developerMode_(false), unitstype_(0) { }

T_dyninstRPC::mdl_metric::~mdl_metric() {
  assert (0);
  if (stmts_) {
    unsigned size = stmts_->size();
    for (unsigned u=0; u<size; u++)
      delete (*stmts_)[u];
    delete stmts_;
  }
  delete flavors_;

  if (constraints_) {
    unsigned size = constraints_->size();
    for (unsigned u=0; u<size; u++)
      delete (*constraints_)[u];
    delete constraints_;
  }

}

bool mdl_data::new_metric(string id, string name, string units,
			  u_int agg, u_int sty, u_int type,
			  vector<T_dyninstRPC::mdl_stmt*> *mv,
			  vector<string> *flav,
			  vector<T_dyninstRPC::mdl_constraint*> *cons,
			  vector<string> *temp_counters,
			  bool developerMode,
			  int unitstype) 
{
  assert (0);
  T_dyninstRPC::mdl_metric *m = new T_dyninstRPC::mdl_metric(id, name, 
							     units, agg,
							     sty, type, mv,
							     flav, cons,
							     temp_counters,
							     developerMode,
							     unitstype);
  if (!m)
    return false;
  else {
    all_metrics += m;
    return true;
  }
}

static bool other_machine_specified(vector< vector<string> > &focus,
				    string& machine) {
  assert(focus[resource::machine][0] == "Machine");

  switch (focus[resource::machine].size()) {
  case 1: break;
  case 2: //Machine/grilled
  case 3: //Machine/grilled/process
  case 4: //Machine/grilled/process/thread
    if (machine != focus[resource::machine][1]) return true;
    break;
  default:
    assert(0);
  }
  return false;
}

static void add_processes(vector< vector<string> > &focus,
				 vector<process*> procs,
#if defined(MT_THREAD)
			  vector<process*> &ip,
			  vector< vector<pdThread *> >& threadsVec,
			  vector< vector<pdThread *> >& instThreadsVec) {
#else
				 vector<process*> &ip) {
#endif
  assert(focus[resource::machine][0] == "Machine");
  unsigned pi, ps;

  switch(focus[resource::machine].size()) {
  case 1:
  case 2:
    ip = procs;
#if defined(MT_THREAD)
    instThreadsVec = threadsVec;
#endif
    break;
  case 3:
    ps = procs.size();
    for (pi=0; pi<ps; pi++)
      if (procs[pi]->rid->part_name() == focus[resource::machine][2]) {
	ip += procs[pi];
	break;
      }
#if defined(MT_THREAD)
    for (pi=0; pi<ip.size(); pi++) {
      instThreadsVec += ip[pi]->threads;
    }
#endif
    break;
#if defined(MT_THREAD)
  case 4:
    ps = threadsVec.size();
    for (pi=0; pi<ps; pi++) {
      vector<pdThread *> tmpThrVec; 
      if (procs[pi]->rid->part_name() == focus[resource::machine][2]) {
	ip += procs[pi];
	for (unsigned ti=0; ti<threadsVec[pi].size(); ti++) {
	  pdThread *thr;
	  thr = (threadsVec[pi])[ti];
	  if (thr && thr->get_rid()) {
	    if (thr->get_rid()->part_name() == focus[resource::machine][3]) {
	      tmpThrVec += thr;
	      break;
	    }
	  }
	}
      }
      if (tmpThrVec.size() > 0) instThreadsVec += tmpThrVec;
    }
    break;
#endif
  default:
    assert(0);
  }
#if defined(TEST_DEL_DEBUG)
  sprintf(errorLine,"--> instThreadsVec.size()=%d\n",instThreadsVec.size());
  logLine(errorLine);
  for (unsigned i=0;i<instThreadsVec.size();i++) {
    sprintf(errorLine,"--> instThreadsVec[%d].size()=%d\n",i,(instThreadsVec[i]).size());
    logLine(errorLine);
    for (unsigned j=0;j<(instThreadsVec[i]).size();j++) {
      pdThread *thr;
      thr = (instThreadsVec[i])[j];
      sprintf(errorLine,"--> thread %d has been selected\n",thr->get_tid());
      logLine(errorLine);
    }
  }
#endif
}

static bool focus_matches(vector<string>& focus, vector<string> *match_path) {
  unsigned mp_size = match_path->size();
  unsigned f_size = focus.size();

  if ((mp_size < 1) || (f_size < 2) || (mp_size != (f_size-1))) {
    return false;
  }

  for (unsigned u = 0; u < mp_size; u++) {
    if(((*match_path)[u] != "*") && (focus[u] != (*match_path)[u])) {
      return false;
    }
  }

  return true;
}


// Global constraints are specified by giving their name within a metric def
// Find the real constraint by searching the dictionary using this name
static T_dyninstRPC::mdl_constraint *flag_matches(vector<string>& focus, 
						  T_dyninstRPC::mdl_constraint *match_me,
						  bool& is_default) 
{
  unsigned c_size = mdl_data::all_constraints.size();
  for (unsigned cs=0; cs<c_size; cs++) 
    if (mdl_data::all_constraints[cs]->id_ == match_me->id_) {
      match_me = mdl_data::all_constraints[cs];
      if (focus_matches(focus, mdl_data::all_constraints[cs]->match_path_)) {
	if (mdl_data::all_constraints[cs]->data_type_ == MDL_T_NONE)
	  is_default = true;
	else
	  is_default = false;
	return match_me;
      }
    }

  return NULL;
}

// Determine if the global and local constraints are applicable
// Global constraints that are applicable are put on flag_cons
// base_used returns the ONE replace constraint that matches

static bool check_constraints(vector<T_dyninstRPC::mdl_constraint*>& flag_cons,
			      vector<T_dyninstRPC::mdl_constraint*>& base_used,
			      vector< vector<string> >& focus,
			      vector<T_dyninstRPC::mdl_constraint*> *cons,
			      vector<unsigned>& flag_dex,
			      vector<unsigned>& base_dex) {
  unsigned size = cons->size();

  unsigned foc_size = focus.size();
  for (unsigned fi=0; fi<foc_size; fi++) {
    unsigned el_size = focus[fi].size();
    if ((focus[fi][0] == "Machine") || (focus[fi][0] == "Process")) {
      // do nothing, can't specify constraints for machine or process
      ;
    } else if (el_size) {
      bool matched = false;
      // Hack allow constraints such as "/SyncObject"
      // However, these don't have to match
      // Otherwise a failure to match means that the metric is invalid for the resource
      unsigned ci=0;
      while ((ci < size) && !matched) {
	if (!(*cons)[ci]->match_path_) {
	  // this is an external constraint, apply if applicable
	  T_dyninstRPC::mdl_constraint *mc;
	  bool is_default;
	  if ((mc = flag_matches(focus[fi], (*cons)[ci], is_default))) {
	    matched = true;
	    if (!is_default) {
	      flag_cons += mc; flag_dex += fi;
	    }
	  }
	} else {
	  // this could be the real constraint to use
	  // this guarantees that the first match is used
	  // TODO -- first matching replace constraint wins
	  // CHANGE: get all that match
	  if (focus_matches(focus[fi], (*cons)[ci]->match_path_)) {
	    matched = true;
	    base_used += (*cons)[ci]; base_dex += fi; 
	  }
	}
	ci++;
      }
      if (!matched && (el_size>1)) {
	return false;
      }
    } else {
      return false;
    }
  }
  return true;
}

// update the interpreter environment for this processor
// Variable updated: $procedures, $modules, $exit, $start
static bool update_environment(process *proc) {

  // for cases when libc is dynamically linked, the exit symbol is not
  // correct
  string vname = "$exit";
  function_base *pdf = proc->findOneFunction(string(EXIT_NAME));
   if (pdf) { 
      mdl_env::add(vname, false, MDL_T_PROCEDURE);
      mdl_env::set(pdf, vname);
  }

  vname = "$start";
  pdf = proc->getMainFunction();
  if (!pdf) return false;

  vname = "$start";
  mdl_env::add(vname, false, MDL_T_PROCEDURE);
  mdl_env::set(pdf, vname);

  vname = "$procedures";
  mdl_env::add(vname, false, MDL_T_LIST_PROCEDURE);
  // only get the functions that are not excluded by exclude_lib or 
  // exclude_func
  mdl_env::set(proc->getIncludedFunctions(), vname);

  vname = "$modules";
  mdl_env::add(vname, false, MDL_T_LIST_MODULE);
  // only get functions that are not excluded by exclude_lib or exclude_func
  mdl_env::set(proc->getIncludedModules(), vname);

  return true;
}

dataReqNode *create_data_object(unsigned mdl_data_type,
				metricDefinitionNode *mn,
#if defined(MT_THREAD)
				bool computingCost,
				pdThread *thr) {
#else
				bool computingCost) {
#endif
  switch (mdl_data_type) {
  case MDL_T_COUNTER:
#if defined(MT_THREAD)
    return (mn->addSampledIntCounter(thr, 0, computingCost));
#else
    return (mn->addSampledIntCounter(0, computingCost));
#endif

  case MDL_T_WALL_TIMER:
#if defined(MT_THREAD)
    return (mn->addWallTimer(computingCost, thr));
#else
    return (mn->addWallTimer(computingCost));
#endif

  case MDL_T_PROC_TIMER:
#if defined(MT_THREAD)
    return (mn->addProcessTimer(computingCost, thr));
#else
    return (mn->addProcessTimer(computingCost));
#endif

  case MDL_T_NONE:
    // just to keep mdl apply allocate a dummy un-sampled counter.
#if defined(SHM_SAMPLING)
    // "true" means that we are going to create a sampled int counter but
    // we are *not* going to sample it, because it is just a temporary
    // counter - naim 4/22/97
    // By default, the last parameter is false - naim 4/23/97
#if defined(MT_THREAD)
    return (mn->addSampledIntCounter(thr, 0, computingCost, true));
#else
    return (mn->addSampledIntCounter(0, computingCost, true));
#endif //MT_THREAD
#else
    return (mn->addUnSampledIntCounter(0, computingCost));
#endif //SHM_SAMPLING

  default:
    assert(0);
    return NULL;
  }
}

// used by both "non-threaded" version and "threaded" version
bool checkInMIPrimitives(string flat_name, metricDefinitionNode *&prim, bool replace_prim)
{
  // this is DCG optimization that will be applied only if OPT_VERSION is on
  // DCG for Dynamic Code Generation
  if (!OPT_VERSION)
    return false;
  
  metric_cerr << " -- in checkInMIPrimitives (by mdl apply_to_process) " << endl;
  
  metricDefinitionNode *temp = NULL;
  const bool alreadyThere = allMIPrimitives.find(flat_name, temp);
  
  if (alreadyThere) {
    if (replace_prim) {
      // fry old entry in allMIPrimitives
      metric_cerr << " checkInMIPrimitives: found primitive named " << flat_name 
		  << " but continue anyway since replace_prim flag is set" << endl;
      // note we don't call 'delete'
      allMIPrimitives.undef(flat_name);
      
      return false;
    }
    else {
      metric_cerr << " checkInMIPrimitives: found primitive for "
		  << flat_name << " ... reusing it. " << endl;
      metric_cerr << "   founded name = " << temp->getFullName() << endl;
      
      prim = temp;
      return true;
    }
  }
  else {
    metric_cerr << " MDL: going to create new primitive since flatname "
		<< flat_name << " doesn't exist" << endl;
    
    return false;
  }
}

// same as "checkInMIPrimitives" except that this is checked in allMIComponents
// also used by both "non-threaded" version and "threaded" version
bool checkInMIComponents(string flat_name, metricDefinitionNode *&comp, bool replace_comp)
{
  // this level of redundency check is applied no matter whether OPT_VERSION is on
  // if (!OPT_VERSION)
  // return false;
  
  metric_cerr << " -- in checkInMIComponents (by mdl apply_to_process) " << endl;
  
  metricDefinitionNode *temp = NULL;
  const bool alreadyThere = allMIComponents.find(flat_name, temp);
  
  if (alreadyThere) {
    if (replace_comp) {
      // fry old entry in allMIComponents
      metric_cerr << " checkInMIComponents: found component named " << flat_name 
		  << " but continue anyway since replace_comp flag is set" << endl;
      // note we don't call 'delete'
      allMIComponents.undef(flat_name);
      
      return false;
    }
    else {
      metric_cerr << " chekInMIComponents: found component for "
		  << flat_name << " ... reusing it. " << endl;
      metric_cerr << "   founded name = " << temp->getFullName() << endl;
      
      comp = temp;
      return true;
    }
  }
  else {
    metric_cerr << " MDL: going to create new component since flatname "
		<< flat_name << " doesn't exist" << endl;
    
    return false;
  }
}

#if !defined(MT_THREAD)
// this prim should always be a new primitive mdn just constructed
// that is, not used by any component mdn yet, and not added to
// allMIPrimitives yet.
bool check2MIPrimitives(string flat_name, metricDefinitionNode *&prim,
			bool computingCost)
{
  // this is DCG optimization that will be applied only if OPT_VERSION is on
  // DCG for Dynamic Code Generation, basicly, a list of (where, definition) of a variable
  // is checked, reuse an already instrumented variable if a match is found between the two
  if (!OPT_VERSION)
    return false;

  metric_cerr << " -- in check2MIPrimitives " << endl;
  assert(!allMIPrimitives.defines(flat_name));

  metricDefinitionNode *match_prim = prim->matchInMIPrimitives();

  if (match_prim != NULL) {  // matched!!
    metric_cerr << "  matched in miprimitives! " << flat_name << endl;

    if (!computingCost) {
      if (toDeletePrimitiveMDN(prim))                                                   // cleanup_drn
	delete prim;  // this is proper, should not be used anywhere else               // removeComponent
      else                                                                              // then delete
	metric_cerr << "  ERR: should be able to delete primitive! " << endl;
    }

    prim = match_prim;
  }
  else {
    // the ONLY place to add into allMIPrimitives
    allMIPrimitives[flat_name] = prim;
  }

  return (match_prim != NULL);
}

// create metric variable and temp varaibles for metric primitive
void initDataRequests(metricDefinitionNode *prim,
		      string& id,
		      unsigned& type,
		      vector<string> *temp_ctr,
		      bool computingCost)
{
  // Create the timer, counter
  dataReqNode *the_node = create_data_object(type, prim, computingCost);
  assert(the_node);

  // we've pushed it earlier
  mdl_env::set(the_node, id);

  // Create the temporary counters - are these useful
  if (temp_ctr) {
    unsigned tc_size = temp_ctr->size();
    for (unsigned tc=0; tc<tc_size; tc++) {
#if defined(SHM_SAMPLING) 
      // "true" means that we are going to create a sampled int counter but
      // we are *not* going to sample it, because it is just a temporary
      // counter - naim 4/22/97
      // By default, the last parameter is false - naim 4/23/97
      dataReqNode *temp_node=prim->addSampledIntCounter(0,computingCost,true);
#else
      dataReqNode *temp_node=prim->addUnSampledIntCounter(0,computingCost);
#endif
      // we've pushed them earlier too
      mdl_env::set(temp_node, (*temp_ctr)[tc]);
    }
  }
}

metricDefinitionNode *
apply_to_process(process *proc, 
		 string& id, string& name,
		 vector< vector<string> >& focus,
		 unsigned& agg_op, metricStyle /* metric_style */,
		 unsigned& type,
		 vector<T_dyninstRPC::mdl_constraint*>& flag_cons,
		 vector<T_dyninstRPC::mdl_constraint*>& base_use,
		 vector<T_dyninstRPC::mdl_stmt*> *stmts,
		 vector<unsigned>& flag_dex,
		 vector<unsigned>& base_dex,
		 vector<string> *temp_ctr,
		 bool replace_component,
		 bool computingCost) {

    metric_cerr << "apply_to_process()" << endl;

    if (!update_environment(proc)) return NULL;

    // compute the flat_name for this component: the machine and process
    // are always defined for the component, even if they are not defined
    // for the aggregate metric.
    vector< vector<string> > component_focus(focus); // they start off equal
    string component_flat_name(name);

    for (unsigned u1 = 0; u1 < focus.size(); u1++) {
      for (unsigned u2 = 0; u2 < focus[u1].size(); u2++) {
	component_flat_name += focus[u1][u2];
      }

      if (focus[u1][0] == "Machine") {
        switch ( (focus[u1].size()) ) {
	  case 1: {
            // there was no refinement to a specific machine...but the component focus
            // must have such a refinement.
	    string tmp_part_name = machineResource->part_name();
            component_flat_name += tmp_part_name;
            component_focus[u1] += tmp_part_name;
	    // no break
	  }
	  case 2: {
	    // there was no refinement to a specific process...but the component
	    // focus must have such a refinement.
	    string tmp_part_name = proc->rid->part_name();
            component_flat_name += tmp_part_name;
            component_focus[u1] += tmp_part_name;
	    break;
          }
	} 
      }
    }//for u1

    // now assert that focus2flatname(component_focus) equals component_flat_name
    extern string metricAndCanonFocus2FlatName(const string &met,
					       const vector< vector<string> > &focus);
    assert(component_flat_name == metricAndCanonFocus2FlatName(name, component_focus));

    metricDefinitionNode *mn = NULL;
    const bool matched = checkInMIComponents(component_flat_name, mn, replace_component);

    if (matched)  return mn;


    // TODO -- Using aggOp value for this metric -- what about folds
    mn = new metricDefinitionNode(proc, name, focus, component_focus, 
				  component_flat_name, aggregateOp(agg_op), 
				  COMP_MDN);
    assert(mn);

    // If the component exists, then we've either already returned, or fried it.
    assert(!allMIComponents.defines(component_flat_name));
    allMIComponents[component_flat_name] = mn;


    metricDefinitionNode *metric_prim = NULL;
    string metric_flat_name(component_flat_name);
    const bool alreadyThere = checkInMIPrimitives(metric_flat_name, metric_prim, replace_component);


    // CASE 1:  there are "replaced constraints" that match, generate code accordingly
    if (base_use.size() > 0) {

      unsigned base_size = base_use.size();
      metric_cerr << "  create base_use (size of " << base_size << ") primitive " << endl;

      if (!alreadyThere) {
	metric_cerr << "  base_use not already there " << endl;

	// primitive metricDefinitionNode constructor
	metric_prim = 
	  new metricDefinitionNode(proc, name, focus, component_focus,
				   metric_flat_name, aggregateOp(agg_op), 
				   PRIM_MDN);

	initDataRequests(metric_prim, id, type, temp_ctr, computingCost); // to create the metric data sample

	for (unsigned bs=0; bs<base_size; bs++) {
	  dataReqNode *flag = NULL;
	  // The following calls mdl_constraint::apply()
	  if (!base_use[bs]->apply(metric_prim,
				   flag, 
				   focus[base_dex[bs]], 
				   proc, 
				   (pdThread*) NULL, 
				   computingCost)) {
	    if (!computingCost) metric_prim->cleanup_drn();
	    delete metric_prim;
	    return NULL;
	  }
	}
      }
      else {
	metric_cerr << "  base_use already there, reuse it! " << endl;
	assert(metric_prim);
      }
    } else {
      // CASE 2: no "replace constraints" match, use normal constraints and metric def
      //    First ------ normal constraints part, generate constraints code

      if (!alreadyThere) {
	unsigned flag_size = flag_cons.size(); // could be zero
	vector<dataReqNode*> flags;
	metric_cerr << "There are " << flag_size << " flags (constraints)" << endl;

	for (unsigned fs=0; fs<flag_size; fs++) {
	  metricDefinitionNode *cons_prim = NULL;
	  string primitive_flat_name = metricAndCanonFocus2FlatName(flag_cons[fs]->id(), component_focus);

	  metric_cerr << "  create " << fs << "th constriant primitive" << endl
		      << "    " << primitive_flat_name << endl;

	  const bool alThere = checkInMIPrimitives(primitive_flat_name, cons_prim, replace_component);

	  if (!alThere) {
	    metric_cerr << "  flag not already there " << endl;

	    // primitive metricDefinitionNode constructor
	    // the primitive_focus is same as the component_focus
	    cons_prim = 
	      new metricDefinitionNode(proc, flag_cons[fs]->id(), focus, 
				       component_focus, primitive_flat_name,
				       aggregateOp(agg_op), PRIM_MDN);

	    dataReqNode *flag = NULL;
	    // The following calls mdl_constraint::apply()
	    if (!flag_cons[fs]->apply(cons_prim,
				      flag, 
				      focus[flag_dex[fs]], 
				      proc, 
				      (pdThread*) NULL, 
				      computingCost)) {
	      if (!computingCost) cons_prim->cleanup_drn();
	      // delete cons_prim;
	      return NULL;
	    }

	    check2MIPrimitives(primitive_flat_name, cons_prim, computingCost);
	  }
	  else { // alThere (constraints)
	    metric_cerr << "  flag already there " << endl;
	    assert(cons_prim);
	  }

	  dataReqNode *flag = cons_prim->getFlagDRN();
	  assert(flag);
	  flags += flag;

	  mn->addPartDummySample(cons_prim);

	  // metric_cerr << "Applied constraint for " << flag_cons[fs]->id_ << endl;
	}

	// Second ------ met def part, generate metric code
	metric_cerr << "  metric not already there, create:   " << endl;

	// primitive metricDefinitionNode constructor
	metric_prim = 
	  new metricDefinitionNode(proc, name, focus, component_focus, 
				   metric_flat_name, aggregateOp(agg_op),
				   PRIM_MDN);

	// do data requests for metric primitive
	initDataRequests(metric_prim, id, type, temp_ctr, computingCost);

	unsigned size = stmts->size();
	for (unsigned u=0; u<size; u++) {
	  if (!(*stmts)[u]->apply(metric_prim, flags)) { // virtual fn call depending on stmt type
	    if (!computingCost) metric_prim->cleanup_drn();
	    delete metric_prim;
	    return NULL;
	  }
	}
      }
      else {
	metric_cerr << "  metric already there, reuse it! " << endl;
	assert(metric_prim);
      }
    } // !(base_use)

    if (!metric_prim->nonNull()) {
      metric_cerr << "metric_prim->nonNull()" << endl;
      if (!computingCost) metric_prim->cleanup_drn();
      delete metric_prim;
      return NULL;
    }

    if (!alreadyThere)  check2MIPrimitives(metric_flat_name, metric_prim, computingCost);

    mn->addPart(metric_prim);

    return mn;
}

#else // following defined(MT_THREAD)

// this prim should always be a new primitive mdn just constructed
// that is, not used by any component mdn yet, and not added to
// allMIPrimitives yet.
bool checkFlagMIPrimitives(string flat_name, metricDefinitionNode *& prim,
			   bool computingCost)
{
  // this is DCG optimization that will be applied only if OPT_VERSION is on
  // DCG for Dynamic Code Generation, basicly, a list of (where, definition) of a variable
  // is checked, reuse an already instrumented variable if a match is found between the two
  if (!OPT_VERSION)
    return false;
  
  metric_cerr << " -- in checkFlagMIPrimitives " << endl;
  assert(!allMIPrimitives.defines(flat_name));
  
  metricDefinitionNode *match_prim = prim->matchInMIPrimitives();
  
  if (match_prim != NULL) {  // matched!!
    metric_cerr << "  flag matched in miprimitives! " << flat_name << endl;
    
    // if (toDeletePrimitiveMDN(prim))
    // delete prim;  // this is proper, should not be used anywhere else
    // else
    // metric_cerr << "  ERR: should be able to delete primitive! " << endl;
    if (!computingCost) {
      prim->cleanup_drn();

      for (unsigned u=0; u<(prim->getComponents()).size(); u++)
	prim->removeComponent((prim->getComponents())[u]);
      (prim->getComponents()).resize(0);
      delete prim;
    }
    
    prim = match_prim;
  }
  else {
    // the ONLY 1 of 2 places to add into allMIPrimitives --- should be allMIPrimitiveFLAGS though
    allMIPrimitives[flat_name] = prim;  // always has only one name
  }
  
  return (match_prim != NULL);
}

// same as "checkFlagMIPrimitives", except that this is for metric, --- not necessary
// need to reuse proc_mn if a match is found
bool checkMetricMIPrimitives(string metric_flat_name, metricDefinitionNode *& metric_prim,
			     bool computingCost)
{
  // this is DCG optimization that will be applied only if OPT_VERSION is on
  // DCG for Dynamic Code Generation, basicly, a list of (where, definition) of a variable
  // is checked, reuse an already instrumented variable if a match is found between the two
  if (!OPT_VERSION)
    return false;

  metric_cerr << " -- in checkMetricMIPrimitives " << endl;
  assert(!allMIPrimitives.defines(metric_flat_name));
  
  metricDefinitionNode *match_prim = metric_prim->matchInMIPrimitives();
  
  if (match_prim != NULL) {  // matched!!
    metric_cerr << "  metric matched in miprimitives! " << metric_flat_name << endl;
    
    // if (toDeletePrimitiveMDN(metric_prim))
    // delete metric_prim;  // this is proper, should not be used anywhere else
    // else
    // metric_cerr << "  ERR: should be able to delete primitive! " << endl;
    if (!computingCost) {
      metric_prim->cleanup_drn();

      for (unsigned u=0; u<(metric_prim->getComponents()).size(); u++)
	metric_prim->removeComponent((metric_prim->getComponents())[u]);
      (metric_prim->getComponents()).resize(0);
      delete metric_prim;
    }
    
    metric_prim = match_prim;
  }
  else {
    // the ONLY 1 of 2 places to add into allMIPrimitives --- this is allMIPrimitives
    allMIPrimitives[metric_flat_name] = metric_prim;  // always has only one name
  }

  return (match_prim != NULL);
}

extern dictionary_hash <unsigned, metricDefinitionNode*> midToMiMap;//metric.C
extern string metricAndCanonFocus2FlatName(const string &met, const vector< vector<string> > &focus);

// no "replace constraint" considered yet
metricDefinitionNode *allocateConstraintData(
                 metricDefinitionNode* mn,
                 pdThread* thr,
                 process */*proc*/,
                 string& /*id*/,
                 vector< vector<string> >& /*focus*/,
                 unsigned& /*type*/,
                 T_dyninstRPC::mdl_constraint *flag_con,  // change from a vector to only one flag_con
                 vector<T_dyninstRPC::mdl_constraint*>& /*base_use*/,
                 vector<T_dyninstRPC::mdl_stmt*> */*stmts*/,
                 unsigned& /*flag_dex*/,
                 vector<unsigned>& /*base_dex*/,
                 vector<string> */*temp_ctr*/,
                 bool computingCost) {

  assert(mn);

  // from constraint::apply()
  switch (flag_con->data_type_) {
  case MDL_T_COUNTER:
  case MDL_T_WALL_TIMER:
  case MDL_T_PROC_TIMER:
    break;
  default:
    assert(0);
  }
  
  dataReqNode *drn = mn->addSampledIntCounter(thr,0,computingCost,true);
  
  // this flag will construct a predicate for the metric -- have to return it
  // flag = drn;
  assert(drn);
  
  if (!mn->nonNull()) {
    if (!computingCost) mn->cleanup_drn();
    delete mn;
    return NULL;
  }
  return mn;
}

metricDefinitionNode *allocateMetricData(
                 metricDefinitionNode* mn,
                 pdThread* thr,
                 process */*proc*/,
                 string& /*id*/,
                 vector< vector<string> >& /*focus*/,
                 unsigned& type,
                 T_dyninstRPC::mdl_constraint */*flag_con*/,
                 vector<T_dyninstRPC::mdl_constraint*>& /*base_use*/,
                 vector<T_dyninstRPC::mdl_stmt*> */*stmts*/,
                 unsigned& /*flag_dex*/,
                 vector<unsigned>& /*base_dex*/,
                 vector<string> *temp_ctr,
                 bool computingCost) {

  // very similar to initDataRequests
  // but did Not set var in mdl_env
  // because we are not going to generate any code for this thread
  
  dataReqNode *the_node = create_data_object(type, mn, computingCost, thr);
  assert(the_node);
  
  // Create the temporary counters 
  if (temp_ctr) {
    unsigned tc_size = temp_ctr->size();
    for (unsigned tc=0; tc<tc_size; tc++) {
      dataReqNode *temp_node = mn->addSampledIntCounter(thr,0,computingCost,true);
      assert(temp_node);
    }
  }
  
  if (!mn->nonNull()) {
    if (!computingCost) mn->cleanup_drn();
    delete mn;
    return NULL;
  }
  return mn;
}

metricDefinitionNode *allocateConstraintData_and_generateCode(
                 metricDefinitionNode* mn,
                 pdThread* thr,
                 process *proc,
                 string& /*id*/,
                 vector< vector<string> >& focus,
                 unsigned& /*type*/,
                 T_dyninstRPC::mdl_constraint *flag_con,  // change from a vector to only one flag_con
                 vector<T_dyninstRPC::mdl_constraint*>& /*base_use*/,
                 vector<T_dyninstRPC::mdl_stmt*> */*stmts*/,
                 unsigned& flag_dex,
                 vector<unsigned>& /*base_dex*/,
                 vector<string> */*temp_ctr*/,
                 bool computingCost) {

  dataReqNode *flag = NULL;
  // The following calls mdl_constraint::apply():
  if (!flag_con->apply(mn, flag, focus[flag_dex], proc, thr, computingCost)) {
    // flag not needed, already set in mn
    if (!computingCost) mn->cleanup_drn();
    delete mn;
    return NULL;
  }
  assert(flag);
  
  if (!mn->nonNull()) {
    if (!computingCost) mn->cleanup_drn();
    delete mn;
    return NULL;
  }
  return mn;
}

metricDefinitionNode *allocateMetricData_and_generateCode(
		 vector<dataReqNode*> flags,

                 metricDefinitionNode* mn,
                 pdThread* thr,
                 process *proc,
                 string& id,
                 vector< vector<string> >& focus,
                 unsigned& type,
                 T_dyninstRPC::mdl_constraint */*flag_con*/,
                 vector<T_dyninstRPC::mdl_constraint*>& base_use,
                 vector<T_dyninstRPC::mdl_stmt*> *stmts,
                 unsigned& /*flag_dex*/,
                 vector<unsigned>& base_dex,
                 vector<string> *temp_ctr,
                 bool computingCost) { // "flags" are passed in as an argument
  
  dataReqNode *the_node = create_data_object(type, mn, computingCost, thr);
  assert(the_node);

  mdl_env::set(the_node, id);
  
  // Create the temporary counters 
  if (temp_ctr) {
    unsigned tc_size = temp_ctr->size();
    for (unsigned tc=0; tc<tc_size; tc++) {
      dataReqNode *temp_node=mn->addSampledIntCounter(thr,0,computingCost,true);
      assert(temp_node);
      mdl_env::set(temp_node, (*temp_ctr)[tc]);
    }
  }
  
  // so far, the same as in initDataRequests
  // "flags" are passed in
  
  if (base_use.size() > 0) {
    //mdl_constraint::apply()
    // metric_cerr << "base_use" <<endl ;
    for (unsigned bs=0; bs<base_use.size(); bs++) {
      dataReqNode *flag = NULL;
      if (!base_use[bs]->apply(mn, flag, focus[base_dex[bs]], proc, thr, computingCost)) {
	if (!computingCost) mn->cleanup_drn();
	delete mn;
	return NULL;
      }
    }
  } else {
    // metric_cerr << "!base_use" << endl ;
    unsigned size = stmts->size();
    for (unsigned u=0; u<size; u++) {
      if (!(*stmts)[u]->apply(mn, flags)) { // virtual fn call depending on stmt type
	if (!computingCost) mn->cleanup_drn();
	delete mn;
	return NULL;
      }
    }
  }

  if (!mn->nonNull()) {
    if (!computingCost) mn->cleanup_drn();
    delete mn;
    return NULL;
  }
  return mn;
}

// allocate data and generate code for all threads
bool allDataGenCode_for_threads(
		 string& name,
		 vector< vector<string> > component_focus,
		 int processIdx,
		 unsigned agg_op,
		 metricStyle metric_style,
		 metricDefinitionNode* mn,
                 process *proc,
                 string& id,
                 vector< vector<string> > focus,
                 unsigned& type,
                 T_dyninstRPC::mdl_constraint *flag_con,
                 vector<T_dyninstRPC::mdl_constraint*>& base_use,
                 vector<T_dyninstRPC::mdl_stmt*> *stmts,
		 unsigned& flag_dex,
                 vector<unsigned>& base_dex,
                 vector<string> *temp_ctr,
		 vector<dataReqNode*> flags,
                 bool computingCost) // flag: "for constraint", or "for metric"
{
    vector <metricDefinitionNode *> parts;

    // STEP 3: create or retrieve the mns for all the threads
    string thr_component_flat_name(name);
    metricDefinitionNode* thr_mn;
    vector <pdThread *>& allThr = proc->threads ;
    unsigned i;

    for (i=0;i<allThr.size();i++) {
      if (allThr[i] != NULL) {
        string thrName;
        thrName = string("thr_") + allThr[i]->get_tid()
	  + string("{") + string(allThr[i]->get_start_func()->prettyName().string_of())+string("}"); 
        if (i==0) {
          component_focus[processIdx] += thrName;
          focus[processIdx] += thrName;
        } else {
	  unsigned pSize ;
          pSize = component_focus[processIdx].size()-1;
          component_focus[processIdx][pSize] = thrName;
          pSize = focus[processIdx].size()-1;
          focus[processIdx][pSize] = thrName;
        }
	
        thr_component_flat_name = metricAndCanonFocus2FlatName(name,component_focus);
	
	if (base_use.size() == 0) {
	  if (!stmts) {
	    thr_component_flat_name += flag_con->id();      // append flag name at the end
	  }
	}

	// if (!allMIComponents.find(thr_component_flat_name,thr_mn)) 
	// thread level mn no longer added to allMIComponents
	thr_mn = 
	  new metricDefinitionNode(proc, name, focus, component_focus,
				   thr_component_flat_name, 
				   aggregateOp(agg_op), THR_LEV);
	assert(thr_mn);

	metric_cerr << "+++++++ construct thr_mn <" 
		    << thr_component_flat_name
		    << ">, " << (computingCost?"compute cost only!":"") << endl;
	
	// record thr_name and part(component) for (prim)mn
	mn->addThrName(thrName);  // make a method to thr_names in metricDefinitionNode
	parts += thr_mn;
      }
    }
    
    if (parts.size() == 0)
      return false;
    // Add thr_mns to proc_mn->components
    mn->addParts(parts);

    //STEP 4 : allocate data and generate code
    // generate code for one thread since all threads share code
    // but data need to be allocated for all threads
    assert(allThr.size()==parts.size());
    for (i=0; i<allThr.size(); i++) {
       pdThread *thr = allThr[i];
       thr_mn = parts[i];

       if (thr_mn->needData()||computingCost) {
         if (!computingCost)  
           thr_mn->needData() = false ;

	 if (base_use.size() == 0) {
	   if (i == 0) {
	     if (stmts == NULL) {  // non-base, fst thr, for constraint
	       if (! (allocateConstraintData_and_generateCode(thr_mn, thr, proc, id, focus, type, flag_con,
		      base_use /* size == 0 */, NULL, flag_dex, base_dex, temp_ctr, computingCost)) ) 
		 return false ;
	     }
	     else {  // non-base, fst thr, for metric
	       if (! (allocateMetricData_and_generateCode(flags, thr_mn, thr, proc, id, focus, type, NULL,
		      base_use /* size == 0 */, stmts, flag_dex, base_dex, temp_ctr, computingCost)) ) 
		 return false ;
	     }
	   }
	   else {
	     if (stmts == NULL) {  // non-base, non-fst thr, for constraint
	       if (! (allocateConstraintData(thr_mn, thr, proc, id, focus, type, flag_con,
		      base_use /* size == 0 */, NULL, flag_dex, base_dex, temp_ctr, computingCost)) ) 
		 return false ;
	     }
	     else {  // non-base, non-fst thr, for metric
	       if (! (allocateMetricData(thr_mn, thr, proc, id, focus, type, NULL,
		      base_use /* size == 0 */, stmts, flag_dex, base_dex, temp_ctr, computingCost)) ) 
		 return false ;
	     }
	   }
	 }
	 else {
	   if (i == 0) {
	     // base, fst thr, for metric;  no constraints needed
	     if (! (allocateMetricData_and_generateCode(flags, thr_mn, thr, proc, id, focus, type, NULL,
		    base_use, NULL, flag_dex, base_dex, temp_ctr, computingCost)) ) 
	       return false ;
	   }
	   else {
	     // base, non-fst thr, for metric;  no constraints needed
	     if (! (allocateMetricData(thr_mn, thr, proc, id, focus, type, NULL,
		    base_use, NULL, flag_dex, base_dex, temp_ctr, computingCost)) ) 
	       return false ;
	   }
	 }

	 // cleanup the instRequests in thr_mn, and copy it to proc_mn
	 // check in "duplicateInst" that code is generated only once
	 if (i == 0) { // instrumentation code is generated when i == 0
	   mn->duplicateInst(thr_mn);  // proc_mn
	   metric_cerr << "  --- " << mn->getSizeOfInstRequests() << " instRequests are generated " << endl;
	 }
       }
    }

    return true;
}

metricDefinitionNode *
apply_to_process(process *proc,
                 string& id, string& name,
                 vector< vector<string> >& focus,
                 unsigned& agg_op, metricStyle metric_style,
                 unsigned& type,
                 vector<T_dyninstRPC::mdl_constraint*>& flag_cons,
                 vector<T_dyninstRPC::mdl_constraint*>& base_use,
                 vector<T_dyninstRPC::mdl_stmt*> *stmts,
                 vector<unsigned>& flag_dex,
                 vector<unsigned>& base_dex,
                 vector<string> *temp_ctr,
                 bool replace_component,
                 bool computingCost) {

    metric_cerr << "apply_to_process()" << endl;
    
    if (!update_environment(proc)) return NULL;
    
    // compute the flat_name for this component: the machine and process
    // are always defined for the component, even if they are not defined
    // for the aggregate metric.
    vector< vector<string> > component_focus(focus); // they start off equal
    string component_flat_name(name);
    
    int thrSelected = -1;  string thrName("-1");
    int processIdx = -1;
    
    for (unsigned u1 = 0; u1 < focus.size(); u1++) {
      for (unsigned u2 = 0; u2 < focus[u1].size(); u2++) {
	component_flat_name += focus[u1][u2];
      }
      
      if (focus[u1][0] == "Machine") {
	processIdx = u1;
        switch ( (focus[u1].size()) ) {
	  case 1: {
            // there was no refinement to a specific machine...but the component focus
            // must have such a refinement.
	    string tmp_part_name = machineResource->part_name();
            component_flat_name += tmp_part_name;
            component_focus[u1] += tmp_part_name;
	    // no break
	  }
	  case 2: {
	    // there was no refinement to a specific process...but the component
	    // focus must have such a refinement.
	    string tmp_part_name = proc->rid->part_name();
            component_flat_name += tmp_part_name;
            component_focus[u1] += tmp_part_name;
	    break;
          }
	  case 4: {
	    thrSelected = u1;
	    thrName = string(component_focus[u1][3]);

	    break;
	  }
	}
      } // Machine 
    }//for u1
    
    // now assert that focus2flatname(component_focus) equals component_flat_name
    assert(component_flat_name == metricAndCanonFocus2FlatName(name, component_focus));
    
    
    // STEP 0: create or retrieve the mn for the selection: selected_mn;
    // STEP 1: create or retrieve the mn for the proc     : proc_mn;
    //         if selected_mn is not proc_mn, make proc_mn to be a component of selected_mn
    
    // STEP 2: create or retrieve the mns for each thread : thr_mn;
    // STEP 3: create or retrieve the mns for all the threads
    // STEP 4: allocate data and generate code
    
    
    // STEP 0 create or retrieve the proc_mn -- then create or retrieve seleted_mn out of it
    
    if (-1 != thrSelected) {
      // part of STEP 1:
      // restore original values for focus, component focus and flat name
      unsigned pSize ;
      pSize = component_focus[processIdx].size()-1;
      component_focus[processIdx].resize(pSize);
      pSize = focus[processIdx].size()-1;
      focus[processIdx].resize(pSize);
      // here component_flat_name is flat_name for thread, after removing thread
      // related focuses, proc_component_flat_name is flat_name for process
    }
    
    string proc_component_flat_name = metricAndCanonFocus2FlatName(name,component_focus);
    
    
    metricDefinitionNode * proc_mn = NULL;
    metricDefinitionNode * selected_mn = NULL;
    
    
    // component_flat_name could be flat_name for process, or for thread
    // allMIComponents record all proc_comp mdn's generated, try to get a match
    const bool matched = checkInMIComponents(proc_component_flat_name, proc_mn, replace_component);

    // What is done in STEP 0:
    // if process mdn is found
    //   if thread is selected
    //     if thread is first time selected
    //       record name
    //   if process is selected
    //     record name
    if (matched) {
      bool recordName = false;

      if (-1 != thrSelected) {
	metric_cerr << " a thread focus is selected. " << endl;
	
	metricDefinitionNode * metric_prim = proc_mn->getMetricPrim();
	selected_mn = metric_prim->getThrComp(thrName);  // thrSelected
	
	if (!selected_mn) {
	  metric_cerr << " the thread not found, might have exited. " << endl;
	  return NULL;
	}
	
	if (0 == selected_mn->getComponents().size()) {
	  metric_cerr << " add proc_mn to selected_mn's comp " << endl;
	  // for THR_LEV: aggregators[0] is a PROC_PRIM, components[0] is PROC_PROC
	  //              could have aggregators[1+], are AGG_LEV's
	  selected_mn->addPart(proc_mn);
	  recordName = true;
	}
      }
      else {
	metric_cerr << " a process focus is selected. " << endl;

	selected_mn = proc_mn;
	recordName = true;
      }
      
      if (recordName) {
	// @@ record the name of the next aggregator for this proc_mn:
	// next aggregator could be this thr_mn(selected_mn), or the agg_mn to-be-built
	// do this name record before return selected_mn
	proc_mn->addCompFlatName(proc_component_flat_name);
      }
      
      // either THR_LEV or PROC_COMP
      return selected_mn;
    }
    else
      metric_cerr // << "MDL: creating new component mi since flatname "
	          // << component_flat_name << " doesn't exist. "
	          << (computingCost? "compute cost only!" : "") << endl;

    
    // STEP 1: create the proc_mn
    
    // again, if a thread is selected, component_flat_name is flat_name for thread,
    // after removing thread related focuses, proc_component_flat_name is flat_name for process
    // otherwise if a process is selected, component_flat_name is flat_name for thread,
    // and proc_component_flat_name will generate the same name for process
    
    proc_mn = new metricDefinitionNode(proc, name, focus, component_focus,
				       proc_component_flat_name, 
				       aggregateOp(agg_op), PROC_COMP);
    assert(proc_mn);
    
    // memorizing stuff
    proc_mn->setMetricRelated(type, computingCost, temp_ctr, flag_cons, base_use);
    
    
    // BEGIN_OF_MT_THREAD
    // can still use "checkInMIPrimitives", "checkFlagMIPrimitives"
    // for each primitive, create mdn and allocate data for all threads, 
    // and create one instrumentation for this primitive  
    
    metricDefinitionNode *metric_prim = NULL;
    string metric_flat_name(proc_component_flat_name);  // the same as proc_component_flat_name
    bool alreadyThere = false;
    
    // CASE 1:  there are "replaced constraints" that match, generate code accordingly
    if (base_use.size() > 0) {
      metric_cerr << "  create base_use (size of " << base_use.size() << ") primitive " << endl;
      
      alreadyThere = checkInMIPrimitives(metric_flat_name, metric_prim, replace_component);
      
      if (!alreadyThere) {
	metric_cerr << "  base_use not already there " << endl;
	
	// primitive metricDefinitionNode constructor
	metric_prim =
	  new metricDefinitionNode(proc, name, // base_use[0]->id()
				   focus, component_focus, metric_flat_name,
				   aggregateOp(agg_op), PROC_PRIM);
	
	vector<dataReqNode*> empty_flags;
	unsigned int empty;

	if (!allDataGenCode_for_threads(name, component_focus, processIdx, agg_op, metric_style, 
					metric_prim, proc, id, focus, type, NULL/*flag_con*/, base_use,
					NULL/*stmts*/, empty, base_dex, temp_ctr, empty_flags, computingCost)) {
	  if (!computingCost) metric_prim->cleanup_drn();
	  //delete metric_prim;
	  return NULL;
	}
      }
      else {
	metric_cerr << "  base_use already there, reuse it! " << endl;

	assert(metric_prim);
      } // endof alreadyThere (base_use)
    } // else of base_use

    else {
      // CASE 2: no "replace constraints" match, use normal constraints and metric def
      //    First ------ normal constraints part, generate constraints and metric respectively 
      //                 as primitive and form component
      
      // metric_cerr << "  create metric primitive:  " << metric_flat_name << endl;
      alreadyThere = checkInMIPrimitives(metric_flat_name, metric_prim, replace_component);
      
      if (!alreadyThere) {
	unsigned flag_size = flag_cons.size(); // could be zero
	vector<dataReqNode*> flags;
	metric_cerr << " there are " << flag_size << " flags (constraints)" << endl;
	
	for (unsigned fs=0; fs<flag_size; fs++) {
	  metricDefinitionNode *cons_prim = NULL;
	  string primitive_flat_name = metricAndCanonFocus2FlatName(flag_cons[fs]->id(),component_focus);

	  metric_cerr << "  create " << fs << "th constraint primitive: " << endl
		      << "    " << primitive_flat_name << endl;

	  const bool alThere = checkInMIPrimitives(primitive_flat_name, cons_prim, replace_component);
	  
	  if (!alThere) {
	    metric_cerr << "  flag not already there " << endl;
	    
	    string cons_name(flag_cons[fs]->id());
	    // primitive metricDefinitionNode constructor
	    // the primitive_focus is same as the component_focus
	    cons_prim = 
	      new metricDefinitionNode(proc, cons_name, focus, component_focus,
				       primitive_flat_name, 
				       aggregateOp(agg_op), PROC_PRIM);
	    
	    // add allocate data for each thread (into thr prims), 
	    // generate instrument code (into proc prim)
	    // then check if proc prim's code already exist
	    // de-allocate data (level and index) for thread if already exist
	    // (just reuse proc prim with its thr prims)
	    
	    vector<dataReqNode*> empty_flags;
	    
	    if (!allDataGenCode_for_threads(cons_name, component_focus, processIdx, agg_op, metric_style, 
					    cons_prim, proc, id, focus, type, flag_cons[fs], base_use,
					    NULL/*stmt*/, flag_dex[fs], base_dex, NULL/*temp_ctr*/, empty_flags, 
					    computingCost)) {
	      if (!computingCost) cons_prim->cleanup_drn();
	      // delete cons_prim;
	      return NULL;
	    }
	    
	    // needed here and safe here, if flag is different, metric will be different,
	    // unless metric is null, but in that case, metric will be cleaned up
	    // allMIPrimitiveFLAGS should be used here!!
	    checkFlagMIPrimitives(primitive_flat_name, cons_prim, computingCost);
	  }
	  else { // alThere (constraints)
	    metric_cerr << "  flag already there " << endl;
	    
	    assert(cons_prim);
	  }
	  
	  // cache created flags
	  dataReqNode *flag = cons_prim->getFlagDRN();
	  assert(flag);
	  flags += flag;
	  
	  // TEMP: see create_data_object
	  proc_mn->addPartDummySample(cons_prim);
	  
	  // metric_cerr << "Applied constraint for " << flag_cons[fs]->id() << endl;
	}
	

	// Second ------ met def part, generate metric code
	metric_cerr << "  metric not already there " << endl;
	
	// primitive metricDefinitionNode constructor
	metric_prim = 
	  new metricDefinitionNode(proc, name, focus, component_focus, 
				   metric_flat_name,
				  // should be NO metric_style and NO agg_style
				   aggregateOp(agg_op), PROC_PRIM);
	
	// add allocate data for each thread (into thr prims), 
	// generate instrument code (into proc prim)
	// then check if proc prim's code already exist
	// de-allocate data (level and index) for thread if already exist
	// (just reuse proc prim with its thr prims)
	
	unsigned int empty;
	if (!allDataGenCode_for_threads(name, component_focus, processIdx, agg_op, metric_style, 
					metric_prim, proc, id, focus, type, NULL/*flag_con*/, base_use,
					stmts, empty, base_dex, temp_ctr, flags, computingCost)) {
	  if (!computingCost) metric_prim->cleanup_drn();
	  // delete metric_prim;
	  return NULL;
	}
      }
      else {
	metric_cerr << "  metric already there, reuse it! " << endl;
	
	assert(metric_prim);
      } // endof alreadyThere (metric stmt)
    } // end of !base_use
    // END_OF_MT_THREAD

    // now, metric_prim could be base_use or not
    
    // Difference: if found the same, reuse metric_prim AND PROC_MN!!
    // register MIComponents for threads in checkMetricMIPrimitives
    const bool alreadyExist = (alreadyThere? false : checkMetricMIPrimitives(metric_flat_name, metric_prim, computingCost));
    
    if (!metric_prim->nonNull()) {
      metric_cerr << "metric_prim->nonNull()" << endl;
      if (!computingCost) metric_prim->cleanup_drn();

      // Extra cleanup
      allMIPrimitives.undef(metric_flat_name);
      allMIComponents.undef(metric_flat_name);
      (metric_prim->getComponents()).resize(0);

      delete metric_prim;
      return NULL;
    }
    
    if (alreadyThere || alreadyExist) {
      // proc_mn: do not do cleanup_drn --- have already been cleaned in prims
      // delete proc_mn;  // metric_prim already deleted in checkMetricMIPrimitives

      for (unsigned u=0; u<(proc_mn->getComponents()).size(); u++)
	proc_mn->removeComponent((proc_mn->getComponents())[u]);  // --- TEST: might remove
      (proc_mn->getComponents()).resize(0);
      delete proc_mn;

      proc_mn = metric_prim->getProcComp();
    }
    else {
      proc_mn->addPart(metric_prim);
      
      if (!computingCost)
	proc->allMIComponentsWithThreads += proc_mn;
    }
    
    
    // move STEP 3 and STEP 4 into "allDataGenCode_for_threads"
    
    // start from recordName, same as before
    bool recordName = false;
    if (-1 != thrSelected) {
      metric_cerr << " a thread focus is selected. " << endl;

      selected_mn = metric_prim->getThrComp(thrName);  // thrSelected

      if (!selected_mn) {
	metric_cerr << " the thread not found, might have exited. " << endl;
	return NULL;
      }
      
      if (0 == selected_mn->getComponents().size()) {
	metric_cerr << " add proc_mn to selected_mn's comp " << endl;
	selected_mn->addPart(proc_mn);
	recordName = true;
      }
    }
    else {
      metric_cerr << " a process focus is selected. " << endl;

      selected_mn = proc_mn;
      recordName = true;
    }

    if (recordName) {
      // @@ record the name of the next aggregator for this proc_mn:
      // next aggregator could be this thr_mn(selected_mn), or the agg_mn to-be-built
      // do this name record before return selected_mn
      proc_mn->addCompFlatName(proc_component_flat_name);

      // MOVE HERE: after checkMetric is done for its replace prim or metric prim:
      // the ONLY place to add into allMIComponents in mdl.C, this add PROC_COMP
      allMIComponents[proc_component_flat_name] = proc_mn;  // only if name is recorded
    }

    //now we assert
    assert(-1 == thrSelected || selected_mn->getComponents().size()==1);

    return selected_mn;
}
#endif // !defined(MT_THREAD)

static bool apply_to_process_list(vector<process*>& instProcess,
#if defined(MT_THREAD)
				  vector< vector<pdThread *> >& threadsVec,
#endif
				  vector<metricDefinitionNode*>& parts,
				  string& id, string& name,
				  vector< vector<string> >& focus,
				  unsigned& agg_op, metricStyle metric_style,
				  unsigned& type,
				  vector<T_dyninstRPC::mdl_constraint*>& flag_cons,
				  vector<T_dyninstRPC::mdl_constraint*>& base_use,
				  vector<T_dyninstRPC::mdl_stmt*> *stmts,
				  vector<unsigned>& flag_dex,
				  vector<unsigned>& base_dex,
				  vector<string> *temp_ctr,
				  bool replace_components_if_present,
				  bool computingCost) {
  metric_cerr << "apply_to_process_list()" << endl;
#ifdef DEBUG_MDL
  timer loadTimer, totalTimer;
  static ofstream *of=NULL;
  if (!of) {
    char buffer[100];
    ostrstream osb(buffer, 100, ios::out);
    osb << "mdl_" << "__" << getpid() << ends;
    of = new ofstream(buffer, ios::app);
  }

  (*of) << "Instrumenting for " << name << " " << instProcess.size() << " processes\n";
#endif

  unsigned psize = instProcess.size();
#ifdef DEBUG_MDL
  totalTimer.clear(); totalTimer.start();
#endif
  for (unsigned p=0; p<psize; p++) {
#ifdef DEBUG_MDL
    loadTimer.clear(); loadTimer.start();  // TIMER
#endif
    process *proc = instProcess[p]; assert(proc);
    global_proc = proc;     // TODO -- global

#if defined(MT_THREAD)
    //check that this vector of threads corresponds to this process
    assert(p<threadsVec.size());
    vector<pdThread *> thrForThisProc = threadsVec[p];
    for (unsigned i=0;i<thrForThisProc.size();i++) 
      assert(thrForThisProc[i]->get_proc() == proc);
#endif
    // skip neonatal and exited processes.
    if (proc->status() == exited || proc->status() == neonatal) continue;

    metricDefinitionNode *comp = apply_to_process(proc, id, name, focus, 
						 agg_op, metric_style, type,
						 flag_cons, base_use, stmts, 
						 flag_dex,
						 base_dex, temp_ctr,
						 replace_components_if_present,
						 computingCost);
    if (comp) {
      // we have another component (i.e. process-specific) mi
      parts += comp;
    }
      

#ifdef DEBUG_MDL
    loadTimer.stop();
    char timeMsg[500];
    sprintf(timeMsg, "%f::%f  ", (float) loadTimer.usecs(), (float) loadTimer.wsecs());
    /*should be removed for output redirection
    tp->applicationIO(10, strlen(timeMsg), timeMsg);
    */
    (*of) << name << ":" << timeMsg << endl;
#endif

  }
#ifdef DEBUG_MDL
  totalTimer.stop();
  char timeMsg[500];
  sprintf(timeMsg, "\nTotal:  %f:user\t%f:wall\n", (float) totalTimer.usecs(),
	  (float) totalTimer.wsecs());
  /*should be removed for output redirection
  tp->applicationIO(10, strlen(timeMsg), timeMsg);
  */
  (*of) << name << ":" << timeMsg << endl;
#endif

  if (parts.size() == 0)
    // no components!
    return false;

  return true;
}

///////////////////////////
vector<string>global_excluded_funcs;


metricDefinitionNode *T_dyninstRPC::mdl_metric::apply(vector< vector<string> > &focus,
					              string& flat_name,
					              vector<process *> procs,
						      vector< vector<pdThread *> >& 
#if defined(MT_THREAD)
                                                      threadsVec
#endif
                                                      ,
					              bool replace_components_if_present,
					              bool computingCost) {
  // TODO -- check to see if this is active ?
  // TODO -- create counter or timer
  // TODO -- put it into the environment ?
  // TODO -- this can be passed directly -- faster - later
  // TODO -- internal metrics
  // TODO -- assume no constraints, all processes instrumented
  // TODO -- support two-level aggregation: one at the daemon, one at paradyn
  // TODO -- how is folding specified ?
  // TODO -- are lists updated here ?

  mdl_env::push();
  mdl_env::add(id_, false, MDL_T_DRN);
  assert(stmts_);

  const unsigned tc_size = temp_ctr_->size();
  for (unsigned tc=0; tc<tc_size; tc++) {
    mdl_env::add((*temp_ctr_)[tc], false, MDL_T_DRN);
  }

  static string machine;
  static bool machine_init= false;
  if (!machine_init) {
    machine_init = true;
    machine = getNetworkName();
  }

  if (other_machine_specified(focus, machine)) return NULL;
  vector<process*> instProcess;
#if defined(MT_THREAD)
  vector< vector<pdThread *> > instThreadsVec;
  add_processes(focus, procs, instProcess, threadsVec, instThreadsVec);

  if (!instProcess.size() || !instThreadsVec.size()) return NULL;
#else
  add_processes(focus, procs, instProcess);

  if (!instProcess.size()) return NULL;
#endif

  // build the list of constraints to use
  vector<T_dyninstRPC::mdl_constraint*> flag_cons;

  // CHANGE: to get all that match
  // the first replace constraint that matches, if any
  vector<T_dyninstRPC::mdl_constraint*> base_used;

  // build list of global constraints that match and choose local replace constraint
  vector<unsigned> base_dex; vector<unsigned> flag_dex;
  if (!check_constraints(flag_cons, base_used, focus, constraints_, flag_dex, base_dex))
    return NULL;

  //////////
  /* 
     Compute the list of excluded functions here. This is the list of functions
     that should be excluded from the calls sites.
     We set the global variable global_excluded_functions to the list of
     excluded functions.

     At this point, the $constraint variable is not in the environment yet,
     so anything that uses $constraint will not be added to the list,
     which is what we want.
   */
  if (base_used.size() > 0) {
    for (unsigned i=0; i < base_used.size(); i++)
      base_used[i]->mk_list(global_excluded_funcs);
  } else {
    for (unsigned u1 = 0; u1 < flag_cons.size(); u1++)
      flag_cons[u1]->mk_list(global_excluded_funcs);
    for (unsigned u2 = 0; u2 < stmts_->size(); u2++)
      (*stmts_)[u2]->mk_list(global_excluded_funcs);
  }
  /*
  metric_cerr << "Metric: " << name_ << endl;
  for (unsigned x1 = 0; x1 < global_excluded_funcs.size(); x1++)
    metric_cerr << "  " << global_excluded_funcs[x1] << endl;
  */
  //////////


  // build the instrumentation request
  vector<metricDefinitionNode*> parts; // one per process
  metricStyle styleV = static_cast<metricStyle>(style_);
  if (!apply_to_process_list(instProcess, 
#if defined(MT_THREAD)
                             instThreadsVec, 
#endif
                             parts, id_, name_, focus,
			     agg_op_, styleV, type_, flag_cons, base_used,
			     stmts_, flag_dex, base_dex, temp_ctr_,
			     replace_components_if_present,
			     computingCost))
    return NULL;

  // construct aggregate for the metric instance parts
  metricDefinitionNode *ret = NULL;

  if (parts.size()) {
    // create aggregate mi, containing the process components "parts"
    ret = new metricDefinitionNode(name_, focus, flat_name, parts,  
				   /* instProcess */ aggregateOp(agg_op_));
  }

  metric_cerr << " apply of " << name_ << " ok" << endl;
  mdl_env::pop();

  ////////////////////
  global_excluded_funcs.resize(0);

  return ret;
}

T_dyninstRPC::mdl_constraint::mdl_constraint()
: id_(""), match_path_(NULL), stmts_(NULL), replace_(false),
  data_type_(0), hierarchy_(0), type_(0) { }

T_dyninstRPC::mdl_constraint::mdl_constraint(string id, 
                                        vector<string> *match_path,
					vector<T_dyninstRPC::mdl_stmt*> *stmts,
					bool replace, u_int d_type, bool& err)
: id_(id), match_path_(match_path), stmts_(stmts), replace_(replace),
  data_type_(d_type), hierarchy_(0), type_(0) 
{ assert(0); err = false; }

T_dyninstRPC::mdl_constraint::~mdl_constraint() {
  assert (0);
  delete match_path_;
  if (stmts_) {
    for (unsigned u=0; u<stmts_->size(); u++)
      delete (*stmts_)[u];
    delete stmts_;
  }
}


static bool do_trailing_resources(vector<string>& resource_,
				  process *proc)
{
  vector<string>  resPath;

  for(unsigned pLen = 0; pLen < resource_.size(); pLen++) {
    string   caStr = string("$constraint") + 
                     string(resource_.size()-pLen-1);
    string   trailingRes = resource_[pLen];

    resPath += resource_[pLen];
    assert(resPath.size() == (pLen+1));

    resource *r = resource::findResource(resPath);
    if (!r) assert(0);

    switch (r->type()) {
    case MDL_T_INT: {
      const char* p = trailingRes.string_of();
      char*       q;
      int         val = (int) strtol(p, &q, 0);
      if (p == q) {
	string msg = string("unable to convert resource '") + trailingRes + 
                     string("' to integer.");
	showErrorCallback(92,msg.string_of());
	return(false);
      }
      mdl_env::add(caStr, false, MDL_T_INT);
      mdl_env::set(val, caStr);
      break;
    }
    case MDL_T_STRING:
      mdl_env::add(caStr, false, MDL_T_STRING);
      mdl_env::set(trailingRes, caStr);
      break;
    case MDL_T_PROCEDURE: {
      // find the resource corresponding to this function's module 
      vector<string> m_vec;
      for(u_int i=0; i < resPath.size()-1; i++){
	m_vec += resPath[i];
      }
      assert(m_vec.size());
      assert(m_vec.size() == (resPath.size()-1));
      resource *m_resource = resource::findResource(m_vec);
      if(!m_resource) return(false);
      
      function_base *pdf = proc->findOneFunction(r,m_resource);
      if (!pdf) return(false);
      mdl_env::add(caStr, false, MDL_T_PROCEDURE);
      mdl_env::set(pdf, caStr);
      break;
    }
    case MDL_T_MODULE: {
      module *mod = proc->findModule(trailingRes,true);
      if (!mod) return(false);
      mdl_env::add(caStr, false, MDL_T_MODULE);
      mdl_env::set(mod, caStr);
      break;
    }
    case MDL_T_MEMORY:
      break ;
    default:
      assert(0);
      break;
    }
  }
  return(true);
}


// Flag constraints need to return a handle to a data request node -- the flag
bool T_dyninstRPC::mdl_constraint::apply(metricDefinitionNode *mn,
					 dataReqNode *&flag,
					 vector<string>& resource,
					 process *proc, pdThread* 
#if defined(MT_THREAD)
                                         thr
#endif
                                         , bool computingCost) {
  assert(mn);
  switch (data_type_) {
  case MDL_T_COUNTER:
  case MDL_T_WALL_TIMER:
  case MDL_T_PROC_TIMER:
    break;
  default:
    assert(0);
  }
  mdl_env::push();

  if (!replace_) {
    // create the counter used as a flag
    mdl_env::add(id_, false, MDL_T_DRN);
#if defined(SHM_SAMPLING) 
    // "true" means that we are going to create a sampled int counter but
    // we are *not* going to sample it, because it is just a temporary
    // counter - naim 4/22/97
    // By default, the last parameter is false - naim 4/23/97
#if defined(MT_THREAD)
    dataReqNode *drn = mn->addSampledIntCounter(thr,0,computingCost,true);
#else
    dataReqNode *drn = mn->addSampledIntCounter(0,computingCost,true);
#endif
#else
    dataReqNode *drn = mn->addUnSampledIntCounter(0,computingCost);
#endif
    // this flag will construct a predicate for the metric -- have to return it
    flag = drn;
    assert(drn);
#if defined(MT_THREAD)
    if (mn->installed()) midToMiMap[drn->getSampleId()] = mn ; 
#endif
    mdl_env::set(drn, id_);
  }

  // put $constraint[X] in the environment
  if(!do_trailing_resources(resource, proc)) {
    mdl_env::pop();
    return(false);
  }

  // Now evaluate the constraint statements
  unsigned size = stmts_->size();
  vector<dataReqNode*> flags;
  bool wasRunning = global_proc->status()==running;
#ifdef DETACH_ON_THE_FLY
  global_proc->reattachAndPause();
#else
  global_proc->pause();
#endif
  for (unsigned u=0; u<size; u++) {
    if (!(*stmts_)[u]->apply(mn, flags)) { // virtual fn call; several possibilities
      metric_cerr << "apply of constraint " << id_ << " failed\n";
      if (wasRunning) {
#ifdef DETACH_ON_THE_FLY
	   global_proc->detachAndContinue();
#else
	   global_proc->continueProc();
#endif
      }
      return(false);
    }
  }
  mdl_env::pop();
  if (wasRunning) {
#ifdef DETACH_ON_THE_FLY
       global_proc->detachAndContinue();
#else
       global_proc->continueProc();
#endif
  }
  return(true);
}

bool T_dyninstRPC::mdl_constraint::replace() {
  return replace_ ;
}

string T_dyninstRPC::mdl_constraint::id() {
  return id_ ;
}

T_dyninstRPC::mdl_constraint *mdl_data::new_constraint(string id, 
                                        vector<string> *path,
					vector<T_dyninstRPC::mdl_stmt*> *Cstmts,
					bool replace, u_int d_type) {
  assert (0);
  bool error;
  return (new T_dyninstRPC::mdl_constraint(id, path, Cstmts, replace, d_type, error));
}

T_dyninstRPC::mdl_stmt::mdl_stmt() { }

T_dyninstRPC::mdl_for_stmt::mdl_for_stmt(string index_name, 
        T_dyninstRPC::mdl_expr *list_exp, T_dyninstRPC::mdl_stmt *body) 
: for_body_(body), index_name_(index_name), list_expr_(list_exp) 
{assert(0);}

T_dyninstRPC::mdl_for_stmt::mdl_for_stmt()
: for_body_(NULL), index_name_(""), list_expr_(NULL) { }

T_dyninstRPC::mdl_for_stmt::~mdl_for_stmt() 
{
  assert (0);
  delete for_body_;
  delete list_expr_;
}

bool T_dyninstRPC::mdl_for_stmt::apply(metricDefinitionNode *mn,
				       vector<dataReqNode*>& flags) {
  mdl_env::push();
  mdl_env::add(index_name_, false);
  mdl_var list_var(false);

  // TODO -- build iterator closure here -- list may be vector or dictionary
  if (!list_expr_->apply(list_var))
    return false;
  if (!list_var.is_list())
    return false;

  // TODO
  //  vector<function_base*> *vp;
  //  list_var.get(vp);
  list_closure closure(index_name_, list_var);

  // mdl_env::set_type(list_var.element_type(), index_name_);
  while (closure.next()) {
    if (!for_body_->apply(mn, flags)) return false;
  }

  mdl_env::pop();
  return true;
}

T_dyninstRPC::mdl_icode::mdl_icode()
  : if_expr_(NULL), expr_(NULL) { }

T_dyninstRPC::mdl_icode::mdl_icode(
  T_dyninstRPC::mdl_expr* expr1, T_dyninstRPC::mdl_expr* expr2)
  : if_expr_(expr1), expr_(expr2)
{ assert (0); }

T_dyninstRPC::mdl_icode::~mdl_icode() 
{ assert(0); delete if_expr_; delete expr_; }

bool T_dyninstRPC::mdl_icode::apply(AstNode *&mn, bool mn_initialized) 
{
  // a return value of true implies that "mn" has been written to

  if (!expr_)
     return false;

  AstNode* pred = NULL;
  AstNode* ast = NULL;

  if (if_expr_) 
  {
    if (!if_expr_->apply(pred))
       return false;
  }
  if (!expr_->apply(ast))
    return false;

  AstNode* code = NULL;
  if (pred) 
  {
    // Note: we don't use assignAst on purpose here
    code = createIf(pred, ast);
    removeAst(pred);
    removeAst(ast);
  }
  else
    code = ast;

  if (mn_initialized) 
  {
    // Note: we don't use assignAst on purpose here
    AstNode *tmp=mn;
    mn = new AstNode(tmp, code);
    removeAst(tmp);
  }
  else 
    mn = assignAst(code);

  removeAst(code);
  return true;
}

T_dyninstRPC::mdl_expr::mdl_expr() { }
T_dyninstRPC::mdl_expr::~mdl_expr() { }

T_dyninstRPC::mdl_v_expr::mdl_v_expr() 
: type_(MDL_T_NONE), int_literal_(0), bin_op_(0), u_op_(0), left_(NULL), 
  right_(NULL), args_(NULL), do_type_walk_(false), ok_(false)
{ }

T_dyninstRPC::mdl_v_expr::mdl_v_expr(int int_lit) 
: type_(MDL_EXPR_INT), int_literal_(int_lit), bin_op_(0), u_op_(0),
  left_(NULL), right_(NULL), args_(NULL), do_type_walk_(false), ok_(false)
{ assert(0); }

T_dyninstRPC::mdl_v_expr::mdl_v_expr(string a_str, bool is_literal) 
: int_literal_(0), bin_op_(0), u_op_(0), left_(NULL),
  right_(NULL), args_(NULL), do_type_walk_(false), ok_(false)
{
  assert (0);
  if (is_literal)
  {
    type_ = MDL_EXPR_STRING;
    str_literal_ = a_str;
  }
  else
  {
    type_ = MDL_EXPR_VAR;
    var_ = a_str;
  }
}

T_dyninstRPC::mdl_v_expr::mdl_v_expr(mdl_expr* expr, vector<string> fields) 
: type_(MDL_EXPR_DOT), int_literal_(0), bin_op_(0),
  u_op_(0), left_(expr), right_(NULL), args_(NULL),
  fields_(fields), do_type_walk_(false), ok_(false)
{ assert(0); }

T_dyninstRPC::mdl_v_expr::mdl_v_expr(string func_name,
				     vector<T_dyninstRPC::mdl_expr *> *a) 
: type_(MDL_EXPR_FUNC), int_literal_(0), var_(func_name), bin_op_(100000),
  u_op_(0), left_(NULL), right_(NULL), args_(a), do_type_walk_(false), 
  ok_(false)
{ assert(0); }

T_dyninstRPC::mdl_v_expr::mdl_v_expr(u_int bin_op, T_dyninstRPC::mdl_expr *left,
				 T_dyninstRPC::mdl_expr *right) 
: type_(MDL_EXPR_BINOP), int_literal_(0), bin_op_(bin_op), u_op_(0),
  left_(left), right_(right), args_(NULL), do_type_walk_(false), ok_(false)
{ assert(0); }

T_dyninstRPC::mdl_v_expr::mdl_v_expr(string var, u_int assign_op,
    T_dyninstRPC::mdl_expr *expr)
: type_(MDL_EXPR_ASSIGN), int_literal_(0), var_(var), bin_op_(assign_op),
  u_op_(0), left_(expr), right_(NULL), args_(NULL), do_type_walk_(false), 
  ok_(false)
{ assert(0); }

T_dyninstRPC::mdl_v_expr::mdl_v_expr(u_int u_op, T_dyninstRPC::mdl_expr *expr,
                 bool is_preop)
: int_literal_(0), bin_op_(0), u_op_(u_op), left_(expr), right_(NULL),
  args_(NULL), do_type_walk_(false), ok_(false)
{
  assert(0);
  if (is_preop)
    type_ = MDL_EXPR_PREUOP;
  else
    type_ = MDL_EXPR_POSTUOP;
}

T_dyninstRPC::mdl_v_expr::mdl_v_expr(string var, mdl_expr* index_expr) 
: type_(MDL_EXPR_INDEX), int_literal_(0), var_(var), bin_op_(0),
  u_op_(0), left_(index_expr), right_(NULL), args_(NULL),
  do_type_walk_(false), ok_(false)
{ assert(0); }

T_dyninstRPC::mdl_v_expr::~mdl_v_expr() 
{
  assert (0);
  if (args_) 
  {
    unsigned size = args_->size();
    for (unsigned u=0; u<size; u++)
      delete (*args_)[u];
    delete args_;
  }
  delete left_; delete right_;
}

bool T_dyninstRPC::mdl_v_expr::apply(AstNode*& ast)
{
  switch (type_) 
  {
    case MDL_EXPR_INT: 
    {
      ast = new AstNode(AstNode::Constant, (void*)int_literal_);
      return true;
    }
    case MDL_EXPR_STRING:
    {
      // create another string here and pass it to AstNode(), instead
      // of using str_literal_.string_of() directly, just to get rid
      // of the compiler warning of "cast discards const". --chun
      //
      char* tmp_str = new char[strlen(str_literal_.string_of())+1];
      strcpy (tmp_str, str_literal_.string_of());
      ast = new AstNode(AstNode::ConstantString, tmp_str);
      delete[] tmp_str;
      return true;
    }
    case MDL_EXPR_INDEX:
    {
      mdl_var index_var;
      if (!left_->apply (index_var))
        return false;
      int index_value;
      if (!index_var.get(index_value))
        return false;

      if (var_ == string ("$arg"))
        ast = new AstNode (AstNode::Param, (void*)index_value);
      else if (var_ == string ("$constraint"))
      {
        string tmp = string("$constraint") + string(index_value);
        mdl_var int_var;
        assert (mdl_env::get(int_var, tmp));
        int value;
        if (!int_var.get(value))
        {
          fprintf (stderr, "Unable to get value for %s\n", tmp.string_of());
          fflush(stderr);
          return false;
        }
        ast = new AstNode(AstNode::Constant, (void*)value);
      }
      else
      {
        mdl_var array_var;
        if (!mdl_env::get(array_var, var_)) return false;
        if (!array_var.is_list()) return false;  
        mdl_var element;
        assert (array_var.get_ith_element(element, index_value));
        switch (element.get_type())
        {
          case MDL_T_INT:
          {
            int value;
            assert (element.get(value));
            ast = new AstNode(AstNode::Constant, (void*)value);
            break;
          }
          case MDL_T_STRING:
          {
            string value;
            assert (element.get(value));
            //
            // create another string here and pass it to AstNode(), instead
            // of using value.string_of() directly, just to get rid of the 
            // compiler warning of "cast discards const".  --chun
            //
            char* tmp_str = new char[strlen(value.string_of())+1];
            strcpy (tmp_str, value.string_of());
            ast = new AstNode(AstNode::ConstantString, tmp_str);
            delete[] tmp_str;

            break;
          }
          default: return false;
        }
      }
      return true;
    }
    case MDL_EXPR_BINOP:
    {
      AstNode* lnode;
      AstNode* rnode;
      if (!left_->apply (lnode)) return false;
      if (!right_->apply (rnode)) return false;
      switch (bin_op_) 
      {
        case MDL_PLUS:
          ast = new AstNode(plusOp, lnode, rnode);
          break;
        case MDL_MINUS:
          ast = new AstNode(minusOp, lnode, rnode);
          break;
        case MDL_DIV:
          ast = new AstNode(divOp, lnode, rnode);
          break;
        case MDL_MULT:
          ast = new AstNode(timesOp, lnode, rnode);
          break;
        case MDL_LT:
          ast = new AstNode(lessOp, lnode, rnode);
          break;
        case MDL_GT:
          ast = new AstNode(greaterOp, lnode, rnode);
          break;
        case MDL_LE:
          ast = new AstNode(leOp, lnode, rnode);
          break;
        case MDL_GE:
          ast = new AstNode(geOp, lnode, rnode);
          break;
        case MDL_EQ:
          ast = new AstNode(eqOp, lnode, rnode);
          break;
        case MDL_NE:
          ast = new AstNode(neOp, lnode, rnode);
          break;
        case MDL_AND:
          ast = new AstNode(andOp, lnode, rnode);
          break;
        case MDL_OR:
          ast = new AstNode(orOp, lnode, rnode);
          break;
        default: return false;
      }
      return true;
    }
    case MDL_EXPR_FUNC:
    {
      if (var_ == "startWallTimer" || var_ == "stopWallTimer"
#if defined(MT_THREAD)
      || var_ == "startProcessTimer" || var_ == "stopProcessTimer"
      || var_ == "startProcessTimer_lwp" || var_ == "destroyProcessTimer")
#else
      || var_ == "startProcessTimer" || var_ == "stopProcessTimer")
#endif
      {
        if (!args_) return false;
        unsigned size = args_->size();
        if (size != 1) return false;

        mdl_var timer(false);
        dataReqNode* drn;
        if (!(*args_)[0]->apply(timer)) return false;
        if (!timer.get(drn)) return false;

        string timer_func;
        if (var_ == "startWallTimer")
          timer_func = START_WALL_TIMER;
        else if (var_ == "stopWallTimer")
          timer_func = STOP_WALL_TIMER;
        else if (var_ == "startProcessTimer")
          timer_func = START_PROC_TIMER;
        else if (var_ == "stopProcessTimer")
          timer_func = STOP_PROC_TIMER;
#if defined(MT_THREAD)
        else if (var_ == "startProcessTimer_lwp")
          timer_func = START_PROC_TIMER_LWP;
	else if (var_ == "destroyProcessTimer")
	  timer_func = DESTROY_PROC_TIMER;
#endif

        vector<AstNode *> ast_args;

#if defined(SHM_SAMPLING)
  #if defined(MT_THREAD)
        ast = createTimer(timer_func, (void*)(drn->getAllocatedLevel()),
			  (void *)(drn->getAllocatedIndex()),
			  ast_args);
  #else
        ast = createTimer(timer_func, 
          (void*)(drn->getInferiorPtr(global_proc)), ast_args);
  #endif
#else
        ast = createTimer(timer_func,(void*)(drn->getInferiorPtr()),ast_args);
#endif
      }
      else if (var_ == "readSymbol")
      {
        mdl_var symbol_var;
        if (!(*args_)[0]->apply(symbol_var))
          return false;
        string symbol_name;
        if (!symbol_var.get(symbol_name))
        {
          fprintf (stderr, "Unable to get symbol name for readSymbol()\n");
          fflush(stderr);
          return false;
        }

        // relying on global_proc to be set in mdl_metric::apply
        if (global_proc) 
        {
          Symbol info;
          Address baseAddr;
          if (global_proc->getSymbolInfo(symbol_name, info, baseAddr)) 
          {
            Address adr = info.addr();
            ast = new AstNode(AstNode::DataAddr, (void*)adr);
          } 
          else 
          {
            string msg = string("In metric '") + currentMetric + string("': ")
              + string("unable to find symbol '") + symbol_name + string("'");
            showErrorCallback(95, msg);
            return false;
          }
        }
      }
      else if (var_ == "readAddress")
      {
        mdl_var addr_var;
        if (!(*args_)[0]->apply (addr_var))
          return false;
        int addr;
        if (!addr_var.get(addr))
        {
          fprintf (stderr, "Unable to get address readAddress()\n");
          fflush(stderr);
          return false;
        }
        ast = new AstNode(AstNode::DataAddr, (void*)addr);
      }
      else
      {
        vector<AstNode *> astargs;
        for (unsigned u = 0; u < args_->size(); u++) 
        {
          AstNode *tmparg=NULL;
          if (!(*args_)[u]->apply(tmparg)) 
          {
            removeAst(tmparg);
            return false;
          }
          astargs += assignAst(tmparg);
          removeAst(tmparg);
        }
        function_base* pdf = global_proc->findOneFunction(var_);
        if (!pdf) 
        {
          string msg = string("In metric '") + currentMetric + string("': ")
            + string("unable to find procedure '") + var_ + string("'");
          showErrorCallback(95, msg);
          return false;
        }
        ast = new AstNode(var_, astargs); //Cannot use simple assignment here!
        for (unsigned i=0;i<astargs.size();i++)
          removeAst(astargs[i]);
        break;
      }
      return true;
    }
    case MDL_EXPR_DOT:
    {
      //??? only allow left hand of DOT to be "$constraint[i]"?

      mdl_var dot_var;
      if (!left_->apply(dot_var))
      {
        fprintf (stderr, "Invalid expression on the left of DOT.\n");
        fflush(stderr);
        return false;
      }
      return true;
    }
    case MDL_EXPR_ASSIGN:
    {
      mdl_var get_drn;
      dataReqNode* drn;
      if (!mdl_env::get(get_drn, var_)) return false;
      if (!get_drn.get(drn)) return false;
      AstNode* ast_arg;
      if (!left_->apply(ast_arg)) return false;

      string func_str;
      switch (bin_op_)
      {
        case MDL_ASSIGN: func_str = "setCounter"; break;
        case MDL_PLUSASSIGN: func_str = "addCounter"; break;
        case MDL_MINUSASSIGN: func_str = "subCounter"; break;
        default: return false;
      }
#if defined(SHM_SAMPLING)
  #if defined(MT_THREAD)
      ast = createCounter(func_str, (void*)(drn->getAllocatedLevel()),
			  (void *)(drn->getAllocatedIndex()), 
			  ast_arg);
  #else
      ast = createCounter(func_str, 
        (void*)(drn->getInferiorPtr(global_proc)), ast_arg);
  #endif
#else
      ast = createCounter(func_str, (void *)(drn->getInferiorPtr()), ast_arg);
#endif
      removeAst (ast_arg);
      return true;
    }
    case MDL_EXPR_VAR:
    {
      if (var_ == "$return") {
        ast = new AstNode(AstNode::ReturnVal, (void*)0);
        return true;
      }
      mdl_var get_drn;
      assert (mdl_env::get(get_drn, var_));
      switch (get_drn.type())
      {
        case MDL_T_INT:
        {
          int value;
          if (!get_drn.get(value)) 
          {
            fprintf(stderr, "Unable to get value for %s\n", var_.string_of());
            fflush(stderr);
            return false;
          }
          else 
            ast = new AstNode(AstNode::Constant, (void*)value);
          return true;
        }
        //case MDL_T_COUNTER:
        case MDL_T_DRN:
        {
          dataReqNode* drn;
          if (!get_drn.get(drn))
          {
            fprintf(stderr, "Unable to get value for %s\n", var_.string_of());
            fflush(stderr);
            return false;
          }
#if defined(SHM_SAMPLING)
  #if defined(MT_THREAD)
          AstNode *tmp_ast;
          tmp_ast = computeTheAddress((void*)(drn->getAllocatedLevel()),
				      (void *)(drn->getAllocatedIndex()),
				      0); // 0 is for intCounter
          // First we get the address, and now we get the value...
          ast = new AstNode(AstNode::DataIndir,tmp_ast);
          removeAst(tmp_ast);
  #else
	  ast = new AstNode(AstNode::DataAddr,  // was AstNode::DataValue
	  // ast = new AstNode(AstNode::DataValue,  // restore AstNode::DataValue
                    (void*)(drn->getInferiorPtr(global_proc)));
  #endif
#else
          // Note: getInferiorPtr could return a NULL pointer here if
          // we are just computing cost - naim 2/18/97
          ast = new AstNode(AstNode::DataAddr,  // was AstNode::DataValue
          // ast = new AstNode(AstNode::DataValue,  // restore AstNode::DataValue
                    (void*)(drn->getInferiorPtr()));
#endif
          return true;
        }
        default:
          fprintf(stderr, "type of variable %s is not known\n", 
            var_.string_of());
          fflush(stderr);
          return false;
      }
    }
    case MDL_EXPR_PREUOP:
    {
      switch (u_op_)
      {
        case MDL_ADDRESS:
        {
          mdl_var get_drn;
          if (!left_->apply(get_drn))
          {
            string msg = string("In metric '") + currentMetric 
              + string("' : ") + string("error in operand of address operator");
            showErrorCallback(92, msg);
            return false;
          }
          dataReqNode *drn;
          assert (get_drn.get(drn));
#if defined(SHM_SAMPLING)
  #if defined(MT_THREAD)
          ast = computeTheAddress((void *)(drn->getAllocatedLevel()),
				  (void *)(drn->getAllocatedIndex()),
				  0); // 0 is for intCounter
  #else
          // ast = new AstNode(AstNode::Constant,  // was AstNode::DataPtr
          ast = new AstNode(AstNode::DataPtr,  // restore AstNode::DataPtr
            (void*)(drn->getInferiorPtr(global_proc)));
  #endif
#else
          // ast = new AstNode(AstNode::Constant, (void*)(drn->getInferiorPtr()));  // was AstNode::DataPtr
          ast = new AstNode(AstNode::DataPtr, (void*)(drn->getInferiorPtr()));  // was AstNode::DataPtr
#endif
          break;
        }
        case MDL_MINUS:
        {
          mdl_var tmp;
          if (!left_->apply (tmp)) return false;
          int value;
          if (!tmp.get(value)) return false;
          ast = new AstNode(AstNode::Constant, (void*)(-value));
          break;
        }
        default:
          return false;
      }
      return true;
    }
    case MDL_EXPR_POSTUOP:
    {
      switch (u_op_)
      {
        case MDL_PLUSPLUS:
        {
          dataReqNode* drn;
          mdl_var drn_var;
          if (!left_->apply(drn_var)) return false;
          if (!drn_var.get(drn)) return false;

          int value = 1;
          AstNode* ast_arg = new AstNode(AstNode::Constant, (void*)value);

#if defined(SHM_SAMPLING)
  #if defined(MT_THREAD)
          ast = createCounter("addCounter", (void*)(drn->getAllocatedLevel()),
			      (void *)(drn->getAllocatedIndex()),
			      ast_arg);
  #else
          ast = createCounter("addCounter", 
            (void*)(drn->getInferiorPtr(global_proc)), ast_arg);
  #endif
#else
          ast = createCounter("addCounter", (void *)(drn->getInferiorPtr()), 
            ast_arg);
#endif
          removeAst(ast_arg);       
          break;
        }
        default: return false;
      }
      return true;
    }
    default:
      return false;
  }
  return true;
}

bool T_dyninstRPC::mdl_v_expr::apply(mdl_var& ret) 
{
  switch (type_) 
  {
    case MDL_EXPR_INT: 
      return (ret.set(int_literal_));
    case MDL_EXPR_STRING:
      return (ret.set(str_literal_));
    case MDL_EXPR_INDEX:
    {
      if (var_ == string ("$arg"))
      {
        // we only allow $arg to appear inside icode (is this right?),
        // and therefore, the other mdl_v_expr::apply() should be used for
        // $arg, and not this one. --chun
        assert (0);
      }
      if (var_ == string ("$constraint"))
      {
        mdl_var ndx(false);
        if (!left_->apply(ndx)) return false;
        int x;
        if (!ndx.get(x)) return false;
        return (mdl_env::get(ret, var_+string(x)));
      }
      mdl_var array(false);
      if (!mdl_env::get(array, var_)) return false;
      if (!array.is_list()) return false;  
      mdl_var index_var;
      if (!left_->apply(index_var)) return false;
      int index_value;
      if (!index_var.get(index_value)) return false;
      if (index_value >= (int)array.list_size()) return false;
      return (array.get_ith_element(ret, index_value));
    }
    case MDL_EXPR_BINOP:
    {
      mdl_var left_val(false), right_val(false);
      if (!left_ || !right_) return false;
      if (!left_->apply(left_val)) return false;
      if (!right_->apply(right_val)) return false;
      return (do_operation(ret, left_val, right_val, bin_op_));
    }
    case MDL_EXPR_FUNC:
    {
      if (var_ == "startWallTimer" || var_ == "stopWallTimer"
#if defined(MT_THREAD)
      || var_ == "startProcessTimer" || var_ == "stopProcessTimer"
      || var_ == "startProcessTimer_lwp")
#else
      || var_ == "startProcessTimer" || var_ == "stopProcessTimer")
#endif
      {
        // this mdl_v_expr::apply() is for expressions outside
        // instrumentation blocks.  these timer functions are not
        // supposed to be used here
        return false;
      }
      else
      {
        mdl_var arg0(false);
        if (!(*args_)[0]->apply(arg0)) return false;
        string func_name;
        if (!arg0.get(func_name)) return false;
        if (global_proc)
        {
          // TODO -- what if the function is not found ?
          function_base *pdf = global_proc->findOneFunction(func_name);
          if (!pdf) { assert(0); return false; }
          return (ret.set(pdf));
        }
        else { assert(0); return false; }
      }
    }
    case MDL_EXPR_DOT:
    {
      if (!left_->apply(ret)) return false;

      if (!do_type_walk_)
        return true;
      else
      { // do_type_walk and type_walk are set in paradyn's mdl.C
        return (walk_deref(ret, type_walk)); 
      }
    }
    case MDL_EXPR_ASSIGN:
    {
      mdl_var lval(false), rval(false);
      if (!mdl_env::get(lval, var_)) return false;
      if (!left_ || !left_->apply(rval))
        return false;
      if (rval.type() == MDL_T_NONE)
      {
        ok_ = true;
        return ok_;
      }
      switch (bin_op_)
      {
        case MDL_ASSIGN:
        {
          int x;
          if (!rval.get(x)) return false;
          ok_ = ret.set(x);
          return ok_;
        }
        case MDL_PLUSASSIGN:
        {
          //chun-- a hack for $arg[i]
          if (lval.type() != MDL_T_NONE && rval.type() != MDL_T_NONE)
            ok_ = do_operation(ret, lval, rval, MDL_MINUS);
          else
            ok_ = true;
          return ok_;
        }
        case MDL_MINUSASSIGN:
        {
          //chun-- a hack for $arg[i]
          if (lval.type() != MDL_T_NONE && rval.type() != MDL_T_NONE)
            ok_ = do_operation(ret, lval, rval, MDL_PLUS);
          else
            ok_ = true;
          return ok_;
        }
      }
    }
    case MDL_EXPR_VAR:
    {
      if (var_ == string("$cmin") || var_ == string("$cmax"))
        ok_ = true;
      else
        ok_ = mdl_env::get (ret, var_);
      return ok_;
    }
    case MDL_EXPR_PREUOP:
    {
      mdl_var lval(false);
      if (!left_ || !left_->apply (lval)) return false;
      ok_ = do_operation(ret, lval, u_op_, true);
      return ok_;
    }
    case MDL_EXPR_POSTUOP:
    {
      mdl_var lval(false);
      if (!left_ || !left_->apply (lval)) return false;
      ok_ = do_operation(ret, lval, u_op_, false);
      return ok_;
    }
    default:
      return false;
  }
  return true;
}

T_dyninstRPC::mdl_if_stmt::mdl_if_stmt(T_dyninstRPC::mdl_expr *expr, T_dyninstRPC::mdl_stmt *body)
  : expr_(expr), body_(body) {assert(0);}

T_dyninstRPC::mdl_if_stmt::mdl_if_stmt()
  : expr_(NULL), body_(NULL) { }

T_dyninstRPC::mdl_if_stmt::~mdl_if_stmt() {
  assert (0);
  delete expr_; delete body_;
}

bool T_dyninstRPC::mdl_if_stmt::apply(metricDefinitionNode *mn,
				      vector<dataReqNode*>& flags) {
  // An if stmt is comprised of (1) the 'if' expr and (2) the body to
  // execute if true.
  mdl_var res(false);
  if (!expr_->apply(res))
    return false;

  int iv;
  switch (res.type()) {
  case MDL_T_INT:
    if (!res.get(iv))
      return false;
    if (!iv)
      return true;
    break;
  default:
    return false;
  }

  return body_->apply(mn, flags);
}

T_dyninstRPC::mdl_seq_stmt::mdl_seq_stmt(vector<T_dyninstRPC::mdl_stmt*> *stmts)
 : stmts_(stmts) { assert (0); }

T_dyninstRPC::mdl_seq_stmt::mdl_seq_stmt() 
 : stmts_(NULL) { }

T_dyninstRPC::mdl_seq_stmt::~mdl_seq_stmt() {
  assert (0);
  if (stmts_) {
    unsigned size = stmts_->size();
    for (unsigned u=0; u<size; u++)
      delete (*stmts_)[u];
    delete stmts_;
  }
}

bool T_dyninstRPC::mdl_seq_stmt::apply(metricDefinitionNode *mn,
				       vector<dataReqNode*>& flags) {
  // a seq_stmt is simply a sequence of statements; apply them all.
  if (!stmts_)
    return true;

  unsigned size = stmts_->size();
  for (unsigned index=0; index<size; index++)
    if (!(*stmts_)[index]->apply(mn, flags)) // virtual fn call
      return false;

  return true;
}

T_dyninstRPC::mdl_list_stmt::mdl_list_stmt(u_int type, string ident,
					   vector<string> *elems,
					   bool is_lib, vector<string>* flavor) 
  : type_(type), id_(ident), elements_(elems), is_lib_(is_lib), flavor_(flavor) 
  { assert (0); }

T_dyninstRPC::mdl_list_stmt::mdl_list_stmt()
  : type_(MDL_T_NONE), id_(""), elements_(NULL), is_lib_(false), flavor_(NULL)
  { }

T_dyninstRPC::mdl_list_stmt::~mdl_list_stmt() 
{ assert(0); delete elements_; }

bool T_dyninstRPC::mdl_list_stmt::apply(metricDefinitionNode * /*mn*/,
					vector<dataReqNode*>& /*flags*/) {
  bool found = false;
  for (unsigned u0 = 0; u0 < flavor_->size(); u0++) {
    if ((*flavor_)[u0] == daemon_flavor) {
      found = true;
      break;
    }
  }
  if (!found) return false;
  if (!elements_) return false;
  mdl_var list_var(id_, false);
  if (!list_var.make_list(type_)) return false;

  unsigned size = elements_->size();

  if (type_ == MDL_T_PROCEDURE_NAME) {
    vector<functionName*> *list_fn;
    bool aflag;
    aflag=list_var.get(list_fn);
    assert(aflag);
    for (unsigned u=0; u<size; u++) {
      functionName *fn = new functionName((*elements_)[u]);
      *list_fn += fn;
    }
  } else if (type_ == MDL_T_PROCEDURE) { 
    assert(0);
  } else if (type_ == MDL_T_MODULE) {
    assert(0);
  } else {
    for (unsigned u=0; u<size; u++) {
      int i; float f;
      mdl_var expr_var(false);
      switch (type_) {
      case MDL_T_INT:
	if (sscanf((*elements_)[u].string_of(), "%d", &i) != 1) return false;
	if (!expr_var.set(i)) return false; 
        break;
      case MDL_T_FLOAT:
	if (sscanf((*elements_)[u].string_of(), "%f", &f) != 1) return false;
	if (!expr_var.set(f)) return false; 
        break;
      case MDL_T_STRING: 
	if (!expr_var.set((*elements_)[u])) return false; 
        break;
      default:
	return false;
      }
      if (!list_var.add_to_list(expr_var)) return false;
    }
  }
  return (mdl_env::add(list_var));
}

T_dyninstRPC::mdl_instr_stmt::mdl_instr_stmt(unsigned pos, 
                                      T_dyninstRPC::mdl_expr *expr,
				      vector<T_dyninstRPC::mdl_icode*> *reqs,
				      unsigned where, bool constrained) 
  : position_(pos), point_expr_(expr), icode_reqs_(reqs),
  where_instr_(where), constrained_(constrained) 
{ assert(0); }

T_dyninstRPC::mdl_instr_stmt::mdl_instr_stmt()
  : position_(0), point_expr_(NULL), icode_reqs_(NULL),
  where_instr_(0), constrained_(false) { }

T_dyninstRPC::mdl_instr_stmt::~mdl_instr_stmt() {
  assert (0);
  delete point_expr_;
  if (icode_reqs_) {
    unsigned size = icode_reqs_->size();
    for (unsigned u=0; u<size; u++)
      delete (*icode_reqs_)[u];
    delete icode_reqs_;
  }
}

bool T_dyninstRPC::mdl_instr_stmt::apply(metricDefinitionNode *mn,
					 vector<dataReqNode*>& inFlags) {
   // An instr statement is like:
   //    append preInsn $constraint[0].entry constrained
   //       (* setCounter(procedureConstraint, 1); *)
   // (note that there are other kinds of statements; i.e. there are other classes
   //  derived from the base class mdl_stmt; see dyninstRPC.I)

  if (icode_reqs_ == NULL)
    return false; // no instrumentation code to put in!

  mdl_var pointsVar(false);
  if (!point_expr_->apply(pointsVar)) // process the 'point(s)' e.g. "$start.entry"
    return false;

  vector<instPoint *> points;
  if (pointsVar.type() == MDL_T_LIST_POINT) {
    vector<instPoint *> *pts;
    if (!pointsVar.get(pts)) return false;
    points = *pts;
  } else if (pointsVar.type() == MDL_T_POINT) {
    instPoint *p;
    if (!pointsVar.get(p)) return false; // where the point is located...
    points += p;
  } else {
    return false;
  }

  // Let's generate the code now (we used to calculate it in the loop below,
  // which was a waste since the code is the same for all points).
  AstNode *code = NULL;
  unsigned size = icode_reqs_->size();
  for (unsigned u=0; u<size; u++) {
    if (!(*icode_reqs_)[u]->apply(code, u>0)) {
      // when u is 0, code is un-initialized
      removeAst(code);
      return false;
    }
  }

  // Instantiate all constraints (flags) here (if any)
  // (if !constrained_ then don't do the following)
  if (constrained_) {
     unsigned fsize = inFlags.size();
     for (int fi=fsize-1; fi>=0; fi--) { // any reason why we go backwards?
#if defined(SHM_SAMPLING)
  #if defined(MT_THREAD)
        AstNode *tmp_ast;
        tmp_ast = computeTheAddress((void *)((inFlags[fi])->getAllocatedLevel()),
				    (void *)((inFlags[fi])->getAllocatedIndex()), 
				    0); // 0 is for intCounter
        AstNode *temp1 = new AstNode(AstNode::DataIndir,tmp_ast);
        removeAst(tmp_ast);
  #else
        // Note: getInferiorPtr could return a NULL pointer here if we are
        // just computing cost - naim 2/18/97
        AstNode *temp1 = new AstNode(AstNode::DataAddr,  // was AstNode::DataValue
        // AstNode *temp1 = new AstNode(AstNode::DataValue,  // restore AstNode::DataValue
				     (void*)((inFlags[fi])->getInferiorPtr(global_proc)));
  #endif
#else
        // Note: getInferiorPtr could return a NULL pointer here if we are
        // just computing cost - naim 2/18/97
        AstNode *temp1 = new AstNode(AstNode::DataAddr,  // was AstNode::DataValue
        // AstNode *temp1 = new AstNode(AstNode::DataValue,  // restore AstNode::DataValue
				     (void*)((inFlags[fi])->getInferiorPtr()));
#endif
        // Note: we don't use assignAst on purpose here
        AstNode *temp2 = code;
        code = createIf(temp1, temp2);
        removeAst(temp1);
        removeAst(temp2);
     }
  }

  if (!code) {
    // we are probably defining an empty metric
    code = new AstNode();
  }

  callOrder corder;
  switch (position_) {
      case MDL_PREPEND: 
	  corder = orderFirstAtPoint; 
	  break;
      case MDL_APPEND: 
	  corder = orderLastAtPoint; 
	  break;
      default: assert(0);
  }

  callWhen cwhen;
  switch (where_instr_) {
      case MDL_PRE_INSN: 
	  cwhen = callPreInsn; 
	  break;
      case MDL_POST_INSN: 
	  cwhen = callPostInsn; 
	  break;
      default: assert(0);
  }

  // for all of the inst points, insert the predicates and the code itself.
  for (unsigned i = 0; i < points.size(); i++) {
      mn->addInst(points[i], code, cwhen, corder );
         // appends an instReqNode to mn's instRequests; actual 
         // instrumentation only
         // takes place when mn->insertInstrumentation() is later called.
  }
  removeAst(code); 
  return true;
}

bool mdl_can_do(string& met_name) {
  // NOTE: We can do better if there's a dictionary of <metric-name> to <anything>
  unsigned size = mdl_data::all_metrics.size();
  for (unsigned u=0; u<size; u++)
    if (mdl_data::all_metrics[u]->name_ == met_name)
      return true;

  /*
  metric_cerr << endl << " all metric names: " << endl;
  for (unsigned u=0; u<size; u++)
    metric_cerr << "  metric name [" << u << "] = [" 
		<< mdl_data::all_metrics[u]->name_ << "] " << endl;
  */

  return false;
}

metricDefinitionNode *mdl_do(vector< vector<string> >& canon_focus, 
                             string& met_name,
			     string& flat_name,
			     vector<process *> procs,
			     vector< vector<pdThread *> >& threadsVec,
			     bool replace_components_if_present,
			     bool computingCost) {
  currentMetric = met_name;
  unsigned size = mdl_data::all_metrics.size();
  // NOTE: We can do better if there's a dictionary of <metric-name> to <metric>!
  for (unsigned u=0; u<size; u++) {
    if (mdl_data::all_metrics[u]->name_ == met_name) {
      return (mdl_data::all_metrics[u]->apply(canon_focus, flat_name, procs,
					      threadsVec,
					      replace_components_if_present,
					      computingCost));
      // calls mdl_metric::apply()
    }
  }
  return NULL;
}

bool mdl_init(string& flavor) { 

  daemon_flavor = flavor;

#ifdef notdef
  vector<mdl_type_desc> kids;
  mdl_type_desc self, kid;
  mdl_focus_element fe;

  self.name = "SyncObject"; self.type = 0; self.end_allowed = false; 
  kid.name = "Message"; kid.type = MDL_T_INT; kid.end_allowed = true; kids += kid;
  kid.name = "Barrier"; kid.type = MDL_T_INT; kid.end_allowed = true; kids += kid;
  kid.name = "Semaphore"; kid.type = MDL_T_INT; kid.end_allowed = true; kids += kid;
  kid.name = "SpinLock"; kid.type = MDL_T_INT; kid.end_allowed = true; kids += kid;
  fe.self = self; fe.kids = kids;
  mdl_data::foci += fe;
  kids.resize(0);

//  self.name = "Procedure"; self.type = MDL_T_MODULE; self.end_allowed = true;
  self.name = "Code"; self.type = MDL_T_MODULE; self.end_allowed = true;
  kid.name = "Module"; kid.type = MDL_T_PROCEDURE; self.end_allowed = true; kids += kids;
  fe.self = self; fe.kids = kids;
  mdl_data::foci += fe;
  kids.resize(0);

  self.name = "Machine"; self.type = MDL_T_STRING; self.end_allowed = true;
  kid.name = "Process"; kid.type = MDL_T_PROCESS; kid.end_allowed = true; kids += kid;
  fe.self = self; fe.kids = kids;
  mdl_data::foci += fe;
  kids.resize(0);
#endif

  mdl_env::push();
  // These are pushed on per-process
  // mdl_env::add("$procedures", false, MDL_T_LIST_PROCEDURE);
  // mdl_env::add("$modules", false, MDL_T_LIST_MODULE);

  string vname = "$machine";
  mdl_env::add(vname, false, MDL_T_STRING);
  string nodename = getNetworkName();
  mdl_env::set(nodename, vname);

  /* Are these entered by hand at the new scope ? */
  /* $arg, $return */

  vector<mdl_type_desc> field_list;
  mdl_type_desc desc;
  desc.end_allowed = false;     // random initialization

  desc.name = "name"; desc.type = MDL_T_STRING; field_list += desc;
  desc.name = "calls"; desc.type = MDL_T_LIST_POINT; field_list += desc;
  desc.name = "entry"; desc.type = MDL_T_POINT; field_list += desc;
  desc.name = "return"; desc.type = MDL_T_POINT; field_list += desc;
  mdl_data::fields[MDL_T_PROCEDURE] = field_list;
  field_list.resize(0);

  desc.name = "name"; desc.type = MDL_T_STRING; field_list += desc;
  desc.name = "funcs"; desc.type = MDL_T_LIST_PROCEDURE; field_list += desc;
  mdl_data::fields[MDL_T_MODULE] = field_list;
  field_list.resize(0);

  return true;
}

void dynRPC::send_metrics(vector<T_dyninstRPC::mdl_metric*>* var_0) {
  mdl_met = true;

  if (var_0) {
    unsigned var_size = var_0->size();
    for (unsigned v=0; v<var_size; v++) {
      // fprintf(stderr, "Got metric %s\n", (*var_0)[v]->name_.string_of());
      // fflush(stderr);
      bool found = false;
      unsigned f_size = (*var_0)[v]->flavors_->size();

      bool flavor_found = false;
      for (unsigned f=0; f<f_size; f++) {
	if ((*(*var_0)[v]->flavors_)[f] == daemon_flavor) {
	  flavor_found = true; break;
	}
      }
      if (flavor_found) {
	unsigned size=mdl_data::all_metrics.size();
	for (unsigned u=0; u<size; u++) 
	  if (mdl_data::all_metrics[u]->id_ == (*var_0)[v]->id_) {
	    delete mdl_data::all_metrics[u];
	    mdl_data::all_metrics[u] = (*var_0)[v];
	    found = true;
	  }
	if (!found) {
	  T_dyninstRPC::mdl_metric *m = (*var_0)[v];
	  mdl_data::all_metrics += m;
	}
      }
    }
  } else {
     fprintf(stderr, "no metric defined\n");
     fflush(stderr);
  }
}

void dynRPC::send_constraints(vector<T_dyninstRPC::mdl_constraint*> *cv) {

  mdl_cons = true;
  if (cv) {
    unsigned var_size = cv->size();
    for (unsigned v=0; v<var_size; v++) {
      bool found = false;
      // cout << "Received " << (*cv)[v]->id_ << endl;
      for (unsigned u=0; u<mdl_data::all_constraints.size(); u++) 
	if (mdl_data::all_constraints[u]->id_ == (*cv)[v]->id_) {
	  delete mdl_data::all_constraints[u];
	  mdl_data::all_constraints[u] = (*cv)[v];
	  found = true;
	}
      if (!found) {
	mdl_data::all_constraints += (*cv)[v];
	// cout << *(*cv)[v] << endl;
      }
    }
  }
}


// TODO -- are these executed immediately ?
void dynRPC::send_stmts(vector<T_dyninstRPC::mdl_stmt*> *vs) {
  mdl_stmt = true;
  if (vs) {
    // ofstream of("other_out", (been_here ? ios::app : ios::out));
    // been_here = true;
    // of << "SEND_STMTS\n";
    // unsigned size = vs->size();
    // for (unsigned u=0; u<size; u++) 
    // (*vs)[u]->print(of);
    mdl_data::stmts += *vs;

    // TODO -- handle errors here
    // TODO -- apply these statements without a metric definition node ?
    unsigned s_size = vs->size();
    vector<dataReqNode*> flags;

    // Application may fail if the list flavor is different than the flavor
    // of this daemon

    for (unsigned s=0; s<s_size; s++) {
      if (!(*vs)[s]->apply(NULL, flags)) 
	;
      // (*vs)[s]->print(cout);
      // cout << endl;
    }
  }
}

// recieves the list of shared libraries to exclude 
void dynRPC::send_libs(vector<string> *libs) {

    mdl_libs = true;
    //metric_cerr << "void dynRPC::send_libs(vector<string> *libs) called" << endl;
    for(u_int i=0; i < libs->size(); i++){
	mdl_data::lib_constraints += (*libs)[i]; 
	//metric_cerr << " send_libs : adding " << (*libs)[i] << " to paradynd set of mdl_data::lib_constraints" << endl;
    }

}

// recieves notification that there are no excluded libraries 
void dynRPC::send_no_libs() {
    mdl_libs = true;
}

static bool do_operation(mdl_var& ret, mdl_var& lval, unsigned u_op, bool/*is_preop*/) 
{
  switch (u_op) 
  {
    case MDL_PLUSPLUS:
    {
      if (lval.type() == MDL_T_INT)
      {
        int x;
        if (!lval.get(x)) return false;
        return (ret.set(x++));
      }
      else
      {
        metric_cerr << "Invalid type for operator ++" << endl;
        return false;
      }
    }
    case MDL_MINUS:
    {
      if (lval.type() == MDL_T_INT)
      {
        int x;
        if (!lval.get(x)) return false;
        return ret.set(-x);
      }
      else
      {
        metric_cerr << "Invalid type for operator -" << endl;
        return false;
      }
    }
    case MDL_ADDRESS:
      return true;
    case MDL_NOT:
    default:
      return false;
  }
  return false;
}

static bool do_operation(mdl_var& ret, mdl_var& left_val,
			 mdl_var& right_val, unsigned bin_op) 
{
  switch (bin_op) 
  {
    case MDL_PLUS:
    case MDL_MINUS:
    case MDL_DIV:
    case MDL_MULT:
    {
      if ((left_val.type() == MDL_T_INT) && (right_val.type() == MDL_T_INT)) 
      {
        int v1, v2;
        if (!left_val.get(v1)) return false;
        if (!right_val.get(v2)) return false;
        switch (bin_op) 
        {
          case MDL_PLUS: return (ret.set(v1+v2));
          case MDL_MINUS: return (ret.set(v1-v2));
          case MDL_DIV: return (ret.set(v1/v2));
          case MDL_MULT: return (ret.set(v1*v2));
        }
      }
      else if (((left_val.type()==MDL_T_INT)||(left_val.type()==MDL_T_FLOAT)) 
        && ((right_val.type()==MDL_T_INT)||(right_val.type()==MDL_T_FLOAT)))
      {
        float v1, v2;
        if (left_val.type() == MDL_T_INT) 
        {
          int i1;
          if (!left_val.get(i1)) return false; v1 = (float)i1;
        }
        else 
        {
          if (!left_val.get(v1)) return false;
        }
        if (right_val.type() == MDL_T_INT) 
        {
          int i1;
          if (!right_val.get(i1)) return false; v2 = (float)i1;
        } 
        else 
        {
          if (!right_val.get(v2)) return false;
        }
        switch (bin_op) 
        {
          case MDL_PLUS: return (ret.set(v1+v2));
          case MDL_MINUS: return (ret.set(v1-v2));
          case MDL_DIV: return (ret.set(v1/v2));
          case MDL_MULT: return (ret.set(v1*v2));
        }
      }
      else
        return false;
    }
    case MDL_LT:
    case MDL_GT:
    case MDL_LE:
    case MDL_GE:
    case MDL_EQ:
    case MDL_NE:
    {
      if ((left_val.type()==MDL_T_STRING)&&(right_val.type()==MDL_T_STRING)) 
      {
        string v1, v2;
        if (!left_val.get(v1)) return false;
        if (!right_val.get(v2)) return false;
        switch (bin_op) 
        {
          case MDL_LT: return (ret.set(v1 < v2));
          case MDL_GT: return (ret.set(v1 > v2));
          case MDL_LE: return (ret.set(v1 <= v2));
          case MDL_GE: return (ret.set(v1 >= v2));
          case MDL_EQ: return (ret.set(v1 == v2));
          case MDL_NE: return (ret.set(v1 != v2));
        }  
      }
      if ((left_val.type() == MDL_T_INT) && (right_val.type() == MDL_T_INT)) 
      {
        int v1, v2;
        if (!left_val.get(v1)) return false;
        if (!right_val.get(v2)) return false;
        switch (bin_op) 
        {
          case MDL_LT: return (ret.set(v1 < v2));
          case MDL_GT: return (ret.set(v1 > v2));
          case MDL_LE: return (ret.set(v1 <= v2));
          case MDL_GE: return (ret.set(v1 >= v2));
          case MDL_EQ: return (ret.set(v1 == v2));
          case MDL_NE: return (ret.set(v1 != v2));
        }
      }
      else if (((left_val.type()==MDL_T_INT)||(left_val.type()==MDL_T_FLOAT))
        && ((right_val.type()==MDL_T_INT)||(right_val.type()==MDL_T_FLOAT))) 
      {
        float v1, v2;
        if (left_val.type() == MDL_T_INT) 
        {
          int i1;
          if (!left_val.get(i1)) return false; v1 = (float)i1;
        } 
        else 
        {
          if (!left_val.get(v1)) return false;
        }
        if (right_val.type() == MDL_T_INT) 
        {
          int i1;
          if (!right_val.get(i1)) return false; v2 = (float)i1;
        }
        else 
        {
          if (!right_val.get(v2)) return false;
        }
        switch (bin_op) 
        {
          case MDL_LT: return (ret.set(v1 < v2));
          case MDL_GT: return (ret.set(v1 > v2));
          case MDL_LE: return (ret.set(v1 <= v2));
          case MDL_GE: return (ret.set(v1 >= v2));
          case MDL_EQ: return (ret.set(v1 == v2));
          case MDL_NE: return (ret.set(v1 != v2));
        }
      }
      else
        return false;
    }
    case MDL_AND:
    case MDL_OR:
    {
      if ((left_val.type() == MDL_T_INT) && (right_val.type() == MDL_T_INT)) 
      {
        int v1, v2;
        if (!left_val.get(v1)) return false;
        if (!right_val.get(v2)) return false;
        switch (bin_op) 
        {
          case MDL_AND: return (ret.set(v1 && v2));
          case MDL_OR: return (ret.set(v1 || v2));
        }
      }
      else
        return false;
    }
    default:
      return false;
  }
  return false;
}

static bool walk_deref(mdl_var& ret, vector<unsigned>& types) 
{
  unsigned index=0;
  unsigned max = types.size();

  while (index < max) 
  {
    unsigned current_type = types[index++];
    unsigned next_field = types[index++];

    switch (current_type) 
    {
      case MDL_T_PROCEDURE_NAME:
      case MDL_T_PROCEDURE:
      {
        function_base *pdf = 0;
        if (!ret.get(pdf)) return false;
        switch (next_field) 
        {
          case 0: 
          {
            string prettyName = pdf->prettyName();
            if (!ret.set(prettyName)) return false;
            break;
            // TODO: should these be passed a process?  yes, they definitely should!
          }
          case 1:
          {
            //
            /*****
             here we should check the calls and exclude the calls to fns in the
             global_excluded_funcs list.
             *****/
            // ARI -- This is probably the spot!

            vector<instPoint*> calls = pdf->funcCalls(global_proc);
            // makes a copy of the return value (on purpose), since we 
            // may delete some items that shouldn't be a call site for 
            // this metric.
            bool anythingRemoved = false; // so far

	    // metric_cerr << "global_excluded_funcs size is: "
	    // << global_excluded_funcs.size() << endl;

	    // metric_cerr << "pdf->funcCalls() returned the following call sites:" 
	    // << endl;

            unsigned oldSize;
            for (unsigned u = 0; u < (oldSize = calls.size());
                                 (oldSize == calls.size()) ? u++ : u) 
            {  // calls.size() can change!
              // metric_cerr << u << ") ";

              instPoint *point = calls[u];
              function_base *callee = const_cast<function_base*>(point->iPgetCallee());

              const char *callee_name=NULL;

              if (callee == NULL) 
              {
                // call Tia's new process::findCallee() to fill in point->callee
                if (!global_proc->findCallee(*point, callee)) 
                {
                  // an unanalyzable function call; sorry.
                  callee_name = NULL;
                  // metric_cerr << "-unanalyzable-" << endl;
                }
                else 
                {
                  // success -- either (a) the call has been bound already, 
                  // in which case the instPoint is updated _and_ callee is 
                  // set, or (b) the call hasn't yet been bound, in which 
                  // case the instPoint isn't updated but callee *is* updated.
                  callee_name = callee->prettyName().string_of();
                  // metric_cerr << "(successful findCallee() was required) "
		  // << callee_name << endl;
                }
              }
              else 
              {
                callee_name = callee->prettyName().string_of();
                // metric_cerr << "(easy case) " << callee->prettyName() << endl;
              }

              // If this callee is in global_excluded_funcs for this metric 
              // (a global vrble...sorry for that), then it's not really a 
              // callee (for this metric, at least), and thus, it should be 
              // removed from whatever we eventually pass to "ret.set()" below.

              if (callee_name != NULL) // could be NULL (e.g. indirect fn call)
                for (unsigned lcv=0; lcv < global_excluded_funcs.size(); lcv++) 
                {
                  if (0==strcmp(global_excluded_funcs[lcv].string_of(),callee_name))
                  {
                    anythingRemoved = true;

                    // remove calls[u] from calls.  To do this, swap
                    // calls[u] with calls[maxndx], and resize-1.
                    const unsigned maxndx = calls.size()-1;
                    calls[u] = calls[maxndx];
                    calls.resize(maxndx);

                    // metric_cerr << "removed something! -- " << callee_name << endl;

                    break;
                  }
                }
            }

            if (!anythingRemoved) 
            {
              // metric_cerr << "nothing was removed -- doing set() now" << endl;
              const vector<instPoint*> *setMe = (const vector<instPoint*> *) 
                                (&pdf->funcCalls(global_proc));
              if (!ret.set(const_cast<vector<instPoint*>*>(setMe)))
                return false;
            }
            else 
            {
              // metric_cerr << "something was removed! -- doing set() now" << endl;
              vector<instPoint*> *setMe = new vector<instPoint*>(calls);
              assert(setMe);
	     
              if (!ret.set(setMe))
                return false;

              // WARNING: "setMe" will now be leaked memory!  The culprit is
              // "ret", which can only take in a _pointer_ to a vector of 
              // instPoint*'s;
              // it can't take in a vector of instPoints*'s, which we'd prefer.
            }
            break;
          }
          case 2: 
          {
            if (!ret.set(const_cast<instPoint *>(pdf->funcEntry(global_proc))))
              return false; 
            break;
          }
          case 3:
          {
            if (!ret.set(const_cast<vector<instPoint *>*>(&pdf->funcExits(global_proc))))
              return false;
            break;
          }
          default:
            assert(0);
            break;
        } //switch(next_field)
        break;
      }
      case MDL_T_MODULE:
      {
        module *mod;
        if (!ret.get(mod)) return false;
        switch (next_field) 
        {
          case 0: 
          {
            string fileName = mod->fileName();
            if (!ret.set(fileName)) return false; 
          } break;
          case 1: 
          {
            if (global_proc) 
            {
              // this is the correct thing to do...get only the included funcs
              // associated with this module, but since we seem to be testing
              // for global_proc elsewhere in this file I guess we will here too
              if (!ret.set(global_proc->getIncludedFunctions(mod))) return false; 
            }
            else 
            {
              // if there is not a global_proc, then just get all functions
              // associtated with this module....under what circumstances
              // would global_proc == 0 ???
              if (!ret.set(mod->getFunctions())) return false; 
            }
            break;
          }
          default: assert(0); break;	       
        } //switch (next_field)
        break;
      }
      default:
        assert(0); return false;
    } // big switch
  } // while
  return true;
}


bool mdl_get_initial(string flavor, pdRPC *connection) {
  mdl_init(flavor);
  while (!(mdl_met && mdl_cons && mdl_stmt && mdl_libs)) {
    switch (connection->waitLoop()) {
    case T_dyninstRPC::error:
      metric_cerr << "mdl_get_initial flavor = " << flavor
	  << " connection = " << connection
          << "  error in connection->waitLoop()" << endl;
      return false;
    default:
      break;
    }
    while (connection->buffered_requests()) {
      switch (connection->process_buffered()) {
      case T_dyninstRPC::error:
	metric_cerr << "mdl_get_initial flavor = " << flavor
	  << " connection = " << connection
          << "  error in connection->processBuffered()" << endl;
	return false;
      default:
	break;
      }
    }
  }
  return true;
}

bool mdl_get_lib_constraints(vector<string> &lc){
    for(u_int i=0; i < mdl_data::lib_constraints.size(); i++){
	lc += mdl_data::lib_constraints[i];
    }
    return (lc.size()>0);
}

void mdl_get_info(vector<T_dyninstRPC::metricInfo>& metInfo) {
  unsigned size = mdl_data::all_metrics.size();
  T_dyninstRPC::metricInfo element;
  for (unsigned u=0; u<size; u++) {
    element.name = mdl_data::all_metrics[u]->name_;
    element.style = mdl_data::all_metrics[u]->style_;
    element.aggregate = mdl_data::all_metrics[u]->agg_op_;
    element.units = mdl_data::all_metrics[u]->units_;
    element.developerMode = mdl_data::all_metrics[u]->developerMode_;
    element.unitstype = mdl_data::all_metrics[u]->unitstype_;
    element.handle = 0; // ignored by paradynd for now
    metInfo += element;
  }
}

bool mdl_metric_data(const string& met_name, mdl_inst_data& md) {
  unsigned size = mdl_data::all_metrics.size();
  for (unsigned u=0; u<size; u++)
    if (mdl_data::all_metrics[u]->name_ == met_name) {
      md.aggregate = mdl_data::all_metrics[u]->agg_op_;
      md.style = (metricStyle) mdl_data::all_metrics[u]->style_;
      return true;
    }
  return false;
}

//
// Compute the list of excluded functions recursively. 
// The mk_list functions are similar to apply, but they make a list
// of the excluded functions
// 

// These prototypes seem to confuse egcs, so let's comment them out
//bool T_dyninstRPC::mdl_list_stmt::mk_list(vector<string> &funcs);
//bool T_dyninstRPC::mdl_for_stmt::mk_list(vector<string> &funcs);
//bool T_dyninstRPC::mdl_if_stmt::mk_list(vector<string> &funcs);
//bool T_dyninstRPC::mdl_seq_stmt::mk_list(vector<string> &funcs);
//bool T_dyninstRPC::mdl_instr_stmt::mk_list(vector<string> &funcs);
//bool T_dyninstRPC::mdl_v_expr::mk_list(vector<string> &funcs);

bool T_dyninstRPC::mdl_v_expr::mk_list(vector<string> &funcs) 
{
  switch (type_) 
  {
    case MDL_EXPR_INT: 
    case MDL_EXPR_STRING:
      return true;
    case MDL_EXPR_INDEX:
    {
//??? why is the func here excluded?
      mdl_var array(false);
      mdl_var index_var(false);
      mdl_var elem(false);
      if (!mdl_env::get(array, var_)) return false;
      if (!left_->apply (index_var)) return false;
      int index_value;
      if (!index_var.get(index_value)) return false;
      if (!array.get_ith_element(elem, index_value)) return false;
      if (elem.get_type() == MDL_T_PROCEDURE_NAME) 
      {
        functionName *fn;
        elem.get(fn);
        funcs += fn->get();
      }
      return true;
    }
    case MDL_EXPR_BINOP: 
    {
      if (!left_ || !right_) return false;
      if (!left_->mk_list(funcs)) return false;
      if (!right_->mk_list(funcs)) return false;
      return true;
    }
    case MDL_EXPR_FUNC:
      return true;
      // TODO: should we add anything to the list here?
    case MDL_EXPR_DOT:
      return true;
    default:
      return false;
  }
  return true;
}


bool T_dyninstRPC::mdl_list_stmt::mk_list(vector<string> &funcs) {
  if (type_ == MDL_T_PROCEDURE_NAME) {
    unsigned size = elements_->size();
    for (unsigned u = 0; u < size; u++)
      funcs += (*elements_)[u];
  }
  return true;
}

bool T_dyninstRPC::mdl_for_stmt::mk_list(vector<string> &funcs) {
  mdl_env::push();
  mdl_var list_var(false);

  if (!list_expr_->apply(list_var)) {
    mdl_env::pop();
   return false;
  }
  if (!list_var.is_list()) {
    mdl_env::pop();
    return false;
  }

  if (list_var.element_type() == MDL_T_PROCEDURE_NAME) {
    vector<functionName *> *funcNames;
    if (!list_var.get(funcNames)) {
      mdl_env::pop();
      return false;
    }    
    for (unsigned u = 0; u < funcNames->size(); u++)
       funcs += (*funcNames)[u]->get();
  }

  if (!for_body_->mk_list(funcs)) {
    mdl_env::pop();
    return false;
  }
  mdl_env::pop();
  return true;
}

bool T_dyninstRPC::mdl_if_stmt::mk_list(vector<string> &funcs) {
  return expr_->mk_list(funcs) && body_->mk_list(funcs);
}

bool T_dyninstRPC::mdl_seq_stmt::mk_list(vector<string> &funcs) {
  for (unsigned u = 0; u < stmts_->size(); u++) {
    if (!(*stmts_)[u]->mk_list(funcs))
      return false;
  }
  return true;
}

bool T_dyninstRPC::mdl_instr_stmt::mk_list(vector<string> &funcs) {
  return point_expr_->mk_list(funcs);
}

bool T_dyninstRPC::mdl_constraint::mk_list(vector<string> &funcs) {
  for (unsigned u = 0; u < stmts_->size(); u++)
    (*stmts_)[u]->mk_list(funcs);
  return true;
}
