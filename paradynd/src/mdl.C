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

#include <iostream.h>
#include <stdio.h>
#include "dyninstRPC.xdr.SRVR.h"
#include "paradyn/src/met/globals.h"
#include "paradynd/src/metric.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/ast.h"
#include "paradynd/src/main.h"
#include "dyninstAPI/src/symtab.h"
#include "util/h/Timer.h"
#include "paradynd/src/mdld.h"
#include "showerror.h"
#include "dyninstAPI/src/process.h"
#include "util/h/debugOstream.h"
#include "paradynd/src/blizzard_memory.h"
#include "dyninstAPI/src/instPoint.h" // new...for class instPoint

// The following vrbles were defined in process.C:
extern debug_ostream attach_cerr;
extern debug_ostream inferiorrpc_cerr;
extern debug_ostream shmsample_cerr;
extern debug_ostream forkexec_cerr;
extern debug_ostream metric_cerr;


// Some global variables used to print error messages:
string currentMetric;  // name of the metric that is being processed.
string currentFocus;   // the focus


static string daemon_flavor;
static process *global_proc = NULL;
static bool mdl_met=false, mdl_cons=false, mdl_stmt=false, mdl_libs=false;

inline unsigned ui_hash(const unsigned &u) { return u; }

vector<unsigned> mdl_env::frames;
vector<mdl_var> mdl_env::all_vars;

vector<T_dyninstRPC::mdl_stmt*> mdl_data::stmts;
vector<T_dyninstRPC::mdl_metric*> mdl_data::all_metrics;
dictionary_hash<unsigned, vector<mdl_type_desc> > mdl_data::fields(ui_hash);
vector<mdl_focus_element> mdl_data::foci;
vector<T_dyninstRPC::mdl_constraint*> mdl_data::all_constraints;
vector<string> mdl_data::lib_constraints;

static bool walk_deref(mdl_var& ret, vector<unsigned>& types, string& var_name);
static bool do_operation(mdl_var& ret, mdl_var& left, mdl_var& right, unsigned bin_op);

class list_closure {
public:
  list_closure(string& i_name, mdl_var& list_v)
    : index_name(i_name), element_type(0), index(0), int_iter(NULL), float_iter(NULL),
  string_iter(NULL), bool_iter(NULL), func_iter(NULL), mod_iter(NULL), max_index(0)
  {
    bool aflag;
    aflag=list_v.is_list();
    assert(aflag);

    element_type = list_v.element_type();
    switch(element_type) {
    case MDL_T_INT:
      aflag=list_v.get(int_iter); assert(aflag);
      max_index = int_iter->size(); break;
    case MDL_T_FLOAT:
      aflag=list_v.get(float_iter); assert(aflag);
      max_index = float_iter->size(); break;
    case MDL_T_STRING:
      aflag=list_v.get(string_iter); assert(aflag);
      max_index = string_iter->size(); break;
    case MDL_T_PROCEDURE_NAME:
      aflag=list_v.get(funcName_iter); assert(aflag);
      max_index = funcName_iter->size();
      break;
    case MDL_T_PROCEDURE:
      aflag=list_v.get(func_iter); assert(aflag);
      max_index = func_iter->size();
      break;
    case MDL_T_MODULE:
      aflag=list_v.get(mod_iter); assert(aflag);
      max_index = mod_iter->size();
      break;
    case MDL_T_POINT:
      aflag=list_v.get(point_iter); assert(aflag);
      max_index = point_iter->size();
      break;
    default:
      assert(0);
    }
  }
  ~list_closure() { }
  bool next() {
    string s;
    function_base *pdf; module *m;
    float f; int i;
    instPoint *ip;

    if (index >= max_index) return false;
    switch(element_type) {
    case MDL_T_INT:
      i = (*int_iter)[index++];      return (mdl_env::set(i, index_name));
    case MDL_T_FLOAT:
      f = (*float_iter)[index++];    return (mdl_env::set(f, index_name));
    case MDL_T_STRING:
      s = (*string_iter)[index++];   return (mdl_env::set(s, index_name));
    case MDL_T_PROCEDURE_NAME:
      // lookup-up the functions defined in resource lists
      // the function may not exist in the image, in which case we get the
      // next one
      do {
	functionName *fn = (*funcName_iter)[index++];
	pdf = global_proc->findOneFunction(fn->get());
      } while (pdf == NULL && index < max_index);
      if (pdf == NULL)
	return false;
      return (mdl_env::set(pdf, index_name));
    case MDL_T_PROCEDURE:
      pdf = (*func_iter)[index++];
      assert(pdf);
      return (mdl_env::set(pdf, index_name));
    case MDL_T_MODULE: 
      m = (*mod_iter)[index++];      return (mdl_env::set(m, index_name));
    case MDL_T_POINT:
      ip = (*point_iter)[index++];   return (mdl_env::set(ip, index_name));
    default:
      assert(0);
    }
    return false;
  }

private:
  string index_name;
  unsigned element_type;
  unsigned index;
  vector<int> *int_iter;
  vector<float> *float_iter;
  vector<string> *string_iter;
  vector<bool> *bool_iter;
  vector<function_base*> *func_iter;
  vector<functionName*> *funcName_iter;
  vector<module*> *mod_iter;
  vector<instPoint*> *point_iter;
  unsigned max_index;
};

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
  unitstype_(unitstype) { }

T_dyninstRPC::mdl_metric::mdl_metric() { }

T_dyninstRPC::mdl_metric::~mdl_metric() {
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
			  int unitstype) {
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
  case 2:
    if (machine != focus[resource::machine][1]) return true;
    break;
  default:
    assert(0);
  }
  return false;
}

static void add_processes(vector< vector<string> > &focus,
				 vector<process*> procs,
				 vector<process*> &ip) {
  assert(focus[resource::process][0] == "Process");
  unsigned pi, ps;

  switch(focus[resource::process].size()) {
  case 1:
    ip = procs;
    break;
  case 2:
#if defined(MT_THREAD)
  case 3:
#endif
    ps = procs.size();
    for (pi=0; pi<ps; pi++)
      if (procs[pi]->rid->part_name() == focus[resource::process][1]) {
	ip += procs[pi];
	break;
      }
    break;
  default:
    assert(0);
  }
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
						  bool& is_default) {
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
			      T_dyninstRPC::mdl_constraint *&base_used,
			      vector< vector<string> >& focus,
			      vector<T_dyninstRPC::mdl_constraint*> *cons,
			      vector<unsigned>& flag_dex, unsigned& base_dex) {
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
	  if (focus_matches(focus[fi], (*cons)[ci]->match_path_)) {
	    matched = true;
	    base_used = (*cons)[ci]; base_dex = fi; 
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

  //Blizzard

    vname = "$gmin";
    mdl_env::add(vname, false, MDL_T_INT);
    mdl_env::set(theMemory->getGlobalMin(), vname);

    vname = "$gmax";
    mdl_env::add(vname, false, MDL_T_INT);
    mdl_env::set(theMemory->getGlobalMax(), vname);

    vname = "$cmin";
    mdl_env::add(vname, false, MDL_T_INT);
    mdl_env::set(theMemory->getCurrentMin(), vname);

    vname = "$cmax";
    mdl_env::add(vname, false, MDL_T_INT);
    mdl_env::set(theMemory->getCurrentMax(), vname);
  //

  return true;
}

dataReqNode *create_data_object(unsigned mdl_data_type,
				metricDefinitionNode *mn,
				bool computingCost) {
  switch (mdl_data_type) {
  case MDL_T_COUNTER:
    return (mn->addSampledIntCounter(0, computingCost));

  case MDL_T_WALL_TIMER:
    return (mn->addWallTimer(computingCost));

  case MDL_T_PROC_TIMER:
    return (mn->addProcessTimer(computingCost));

  case MDL_T_NONE:
    // just to keep mdl apply allocate a dummy un-sampled counter.
#if defined(SHM_SAMPLING) && defined(MT_THREAD)
    // "true" means that we are going to create a sampled int counter but
    // we are *not* going to sample it, because it is just a temporary
    // counter - naim 4/22/97
    // By default, the last parameter is false - naim 4/23/97
    return (mn->addSampledIntCounter(0, computingCost, true));
#else
    return (mn->addUnSampledIntCounter(0, computingCost));
#endif

  default:
    assert(0);
    return NULL;
  }
}


metricDefinitionNode *
apply_to_process(process *proc, 
		 string& id, string& name,
		 vector< vector<string> >& focus,
		 unsigned& agg_op,
		 unsigned& type,
		 vector<T_dyninstRPC::mdl_constraint*>& flag_cons,
		 T_dyninstRPC::mdl_constraint *base_use,
		 vector<T_dyninstRPC::mdl_stmt*> *stmts,
		 vector<unsigned>& flag_dex,
		 unsigned& base_dex,
		 vector<string> *temp_ctr,
		 bool replace_component,
		 bool computingCost) {

    if (!update_environment(proc)) return NULL;

    // compute the flat_name for this component: the machine and process
    // are always defined for the component, even if they are not defined
    // for the aggregate metric.
    vector< vector<string> > component_focus(focus); // they start off equal

    string component_flat_name(name);
    for (unsigned u1 = 0; u1 < focus.size(); u1++) {
      if (focus[u1][0] == "Process") {
	component_flat_name += focus[u1][0] + proc->rid->part_name();
	if (focus[u1].size() == 1) {
	   // there was no refinement to a specific process...but the component
	   // focus must have such a refinement.
	   component_focus[u1] += proc->rid->part_name();
	}
      }
      else if (focus[u1][0] == "Machine") {
	component_flat_name += focus[u1][0] + machineResource->part_name();
	if (focus[u1].size() == 1) {
	   // there was no refinement to a specific machine...but the component focus
	   // must have such a refinement.
	   component_focus[u1] += machineResource->part_name();
	}
      }
      else
	for (unsigned u2 = 0; u2 < focus[u1].size(); u2++)
	  component_flat_name += focus[u1][u2];
    }

    // now assert that focus2flatname(component_focus) equals component_flat_name
    extern string metricAndCanonFocus2FlatName(const string &met,
					       const vector< vector<string> > &focus);
    assert(component_flat_name == metricAndCanonFocus2FlatName(name, component_focus));

    metricDefinitionNode *existingMI;
    const bool alreadyThere = allMIComponents.find(component_flat_name, existingMI);
    if (alreadyThere) {
       if (replace_component) {
	  // fry old entry...
	  metric_cerr << "apply_to_process: found " << component_flat_name
	              << " but continuing anyway since replace_component flag set"
		      << endl;
	  // note that we don't call 'delete'.
	  allMIComponents.undef(component_flat_name);
       }
       else {
	 metric_cerr << "mdl apply_to_process: found component for "
	             << component_flat_name << "...reusing it" << endl;

	 return existingMI;
       }
    }
    else
       metric_cerr << "MDL: creating new component mi since flatname "
	           << component_flat_name << " doesn't exist" << endl;

    // If the component exists, then we've either already returned, or fried it.
    assert(!allMIComponents.defines(component_flat_name));

    // TODO -- Using aggOp value for this metric -- what about folds
    metricDefinitionNode *mn = new metricDefinitionNode(proc, name, focus,
							component_focus,
							component_flat_name, agg_op);
    assert(mn);

    assert(!allMIComponents.defines(component_flat_name));
    allMIComponents[component_flat_name] = mn;

    // Create the timer, counter
    dataReqNode *the_node = create_data_object(type, mn, computingCost);
    assert(the_node);
    mdl_env::set(the_node, id);

    // Create the temporary counters - are these useful
    if (temp_ctr) {
      unsigned tc_size = temp_ctr->size();
      for (unsigned tc=0; tc<tc_size; tc++) {
#if defined(SHM_SAMPLING) && defined(MT_THREAD)
        // "true" means that we are going to create a sampled int counter but
        // we are *not* going to sample it, because it is just a temporary
        // counter - naim 4/22/97
        // By default, the last parameter is false - naim 4/23/97
        dataReqNode *temp_node=mn->addSampledIntCounter(0,computingCost,true);
#else
	dataReqNode *temp_node=mn->addUnSampledIntCounter(0,computingCost);
#endif
	mdl_env::set(temp_node, (*temp_ctr)[tc]);
      }
    }

    unsigned flag_size = flag_cons.size(); // could be zero
    vector<dataReqNode*> flags;
    metric_cerr << "There are " << flag_size << " flags (constraints)" << endl;

    for (unsigned fs=0; fs<flag_size; fs++) {
	// TODO -- cache these created flags
	dataReqNode *flag = NULL;
	// The following calls mdl_constraint::apply():
	if (!flag_cons[fs]->apply(mn, flag, focus[flag_dex[fs]], proc, computingCost)) {
          mn->cleanup_drn();
          delete mn;
	  return NULL;
	}
	assert(flag);
	flags += flag;

	metric_cerr << "Applied constraint for " << flag_cons[fs]->id_ << endl;
    }

    if (base_use) {
      dataReqNode *flag = NULL;
      if (!base_use->apply(mn, flag, focus[base_dex], proc, computingCost)) {
	mn->cleanup_drn();  
        delete mn;
	return NULL;
      }
    } else {
      unsigned size = stmts->size();
      for (unsigned u=0; u<size; u++) {
	if (!(*stmts)[u]->apply(mn, flags)) { // virtual fn call depending on stmt type
	  mn->cleanup_drn();  
	  delete mn;
	  return NULL;
	}
      }
    }

    if (!mn->nonNull()) {
      mn->cleanup_drn();
      delete mn;
      return NULL;
    }

    return mn;
}

static bool apply_to_process_list(vector<process*>& instProcess,
				  vector<metricDefinitionNode*>& parts,
				  string& id, string& name,
				  vector< vector<string> >& focus,
				  unsigned& agg_op,
				  unsigned& type,
				  vector<T_dyninstRPC::mdl_constraint*>& flag_cons,
				  T_dyninstRPC::mdl_constraint *base_use,
				  vector<T_dyninstRPC::mdl_stmt*> *stmts,
				  vector<unsigned>& flag_dex,
				  unsigned& base_dex,
				  vector<string> *temp_ctr,
				  bool replace_components_if_present,
				  bool computingCost) {
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

    // skip neonatal and exited processes.
    if (proc->status() == exited || proc->status() == neonatal) continue;

    metricDefinitionNode *comp = apply_to_process(proc, id, name, focus, 
						 agg_op, type,
						 flag_cons, base_use, stmts, 
						 flag_dex,
						 base_dex, temp_ctr,
						 replace_components_if_present,
						 computingCost);
    if (comp)
      // we have another component (i.e. process-specific) mi
      parts += comp;

#ifdef DEBUG_MDL
    loadTimer.stop();
    char timeMsg[500];
    sprintf(timeMsg, "%f::%f  ", (float) loadTimer.usecs(), (float) loadTimer.wsecs());
    // tp->applicationIO(10, strlen(timeMsg), timeMsg);
    (*of) << name << ":" << timeMsg << endl;
#endif

  }
#ifdef DEBUG_MDL
  totalTimer.stop();
  char timeMsg[500];
  sprintf(timeMsg, "\nTotal:  %f:user\t%f:wall\n", (float) totalTimer.usecs(),
	  (float) totalTimer.wsecs());
  // tp->applicationIO(10, strlen(timeMsg), timeMsg);
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
    machine = getHostName();
  }

  // TODO -- I am assuming that a canonical resource list is
  // machine, procedure, process, syncobject

  if (other_machine_specified(focus, machine)) return NULL;
  vector<process*> instProcess;
  add_processes(focus, procs, instProcess);

  if (!instProcess.size()) return NULL;

  // build the list of constraints to use
  vector<T_dyninstRPC::mdl_constraint*> flag_cons;

  // the first replace constraint that matches, if any
  T_dyninstRPC::mdl_constraint *base_used=NULL;

  // build list of global constraints that match and choose local replace constraint
  unsigned base_dex; vector<unsigned> flag_dex;
  if (!check_constraints(flag_cons, base_used, focus, constraints_, flag_dex, base_dex))
    return NULL;

  //////////
  /* 
     Compute the list of excluded functions here. This is the list of functions
     that should be excluded from the calls sites.
     We set the global variable global_excluded_functions to the list of excluded
     functions.

     At this point, the $constraint variable is not in the environment yet,
     so anything that uses $constraint will not be added to the list,
     which is what we want.
   */
  if (base_used) {
    base_used->mk_list(global_excluded_funcs);
  } else {
    for (unsigned u1 = 0; u1 < flag_cons.size(); u1++)
      flag_cons[u1]->mk_list(global_excluded_funcs);
    for (unsigned u2 = 0; u2 < stmts_->size(); u2++)
      (*stmts_)[u2]->mk_list(global_excluded_funcs);
  }
  metric_cerr << "Metric: " << name_ << endl;
  for (unsigned x1 = 0; x1 < global_excluded_funcs.size(); x1++)
    metric_cerr << "  " << global_excluded_funcs[x1] << endl;
  //////////


  // build the instrumentation request
  vector<metricDefinitionNode*> parts; // one per process
  if (!apply_to_process_list(instProcess, parts, id_, name_, focus,
			     agg_op_, type_, flag_cons, base_used,
			     stmts_, flag_dex, base_dex, temp_ctr_,
			     replace_components_if_present,
			     computingCost))
    return NULL;

  // construct aggregate for the metric instance parts
  metricDefinitionNode *ret = NULL;

  if (parts.size())
    // create aggregate mi, containing the process components "parts"
    ret = new metricDefinitionNode(name_, focus, flat_name, parts, agg_op_);

  // cout << "apply of " << name_ << " ok\n";
  mdl_env::pop();

  ////////////////////
  global_excluded_funcs.resize(0);

  return ret;
}

T_dyninstRPC::mdl_constraint::mdl_constraint() { }
T_dyninstRPC::mdl_constraint::mdl_constraint(string id, vector<string> *match_path,
					     vector<T_dyninstRPC::mdl_stmt*> *stmts,
					     bool replace, u_int d_type, bool& error)
: id_(id), match_path_(match_path), stmts_(stmts), replace_(replace),
  data_type_(d_type), hierarchy_(0), type_(0) { error = false; }
T_dyninstRPC::mdl_constraint::~mdl_constraint() {
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

  for(int pLen = 0; pLen < resource_.size(); pLen++) {
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
    case MDL_T_VARIABLE: {
        //bounds is defined in metric.h
        memory::bounds b = theMemory->getVariableBounds(trailingRes) ;
        mdl_env::add(caStr, false, MDL_T_VARIABLE) ;
        mdl_env::set(b, caStr) ;
    }
    break ;
    default:
      assert(0);
      break;
    }
  }
  return(true);
}


// Replace constraints not working yet
// Flag constraints need to return a handle to a data request node -- the flag
bool T_dyninstRPC::mdl_constraint::apply(metricDefinitionNode *mn,
					 dataReqNode *&flag,
					 vector<string>& resource,
					 process *proc, bool computingCost) {
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
#if defined(SHM_SAMPLING) && defined(MT_THREAD)
    // "true" means that we are going to create a sampled int counter but
    // we are *not* going to sample it, because it is just a temporary
    // counter - naim 4/22/97
    // By default, the last parameter is false - naim 4/23/97
    dataReqNode *drn = mn->addSampledIntCounter(0,computingCost,true);
#else
    dataReqNode *drn = mn->addUnSampledIntCounter(0,computingCost);
#endif
    // this flag will construct a predicate for the metric -- have to return it
    flag = drn;
    assert(drn);
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
  for (unsigned u=0; u<size; u++) {
    if (!(*stmts_)[u]->apply(mn, flags)) { // virtual fn call; several possibilities
      // cout << "apply of constraint " << id_ << " failed\n";
      return(false);
    }
  }
  mdl_env::pop();
  return(true);
}

T_dyninstRPC::mdl_constraint *mdl_data::new_constraint(string id, vector<string> *path,
						vector<T_dyninstRPC::mdl_stmt*> *stmts,
						bool replace, u_int d_type) {
  bool error;
  return (new T_dyninstRPC::mdl_constraint(id, path, stmts, replace, d_type, error));
}


T_dyninstRPC::mdl_rand::mdl_rand() {}

T_dyninstRPC::mdl_instr_rand::mdl_instr_rand() {}

T_dyninstRPC::mdl_instr_rand::mdl_instr_rand(u_int type): type_(type) {}

T_dyninstRPC::mdl_instr_rand::mdl_instr_rand(u_int type, u_int val)
: type_(type), val_(val) {}

T_dyninstRPC::mdl_instr_rand::mdl_instr_rand(u_int type, string name)
: type_(type), name_(name) {}

T_dyninstRPC::mdl_instr_rand::mdl_instr_rand(u_int type, string name, vector<mdl_instr_rand *>args)
: type_(type), name_(name) {
  for (unsigned u = 0; u < args.size(); u++)
    args_ += args[u];
}

T_dyninstRPC::mdl_instr_rand::~mdl_instr_rand() { } 

bool T_dyninstRPC::mdl_instr_rand::apply(AstNode *&ast) {
  function_base *pdf;
  mdl_var get_drn;

  switch (type_) {
  case MDL_T_RECORD://TO DO
       {
           int value ;
           mdl_var get_record ;
           mdl_env::get(get_record, string("$constraint0")) ;
           unsigned type = mdl_env::get_type(string("$constraint0")) ;
           switch(type)
           {
            case MDL_T_VARIABLE:
                memory::bounds b ;
                if (!get_record.get(b))
                {
                    return false;
                }else
                {
                    if(!strncmp(name_.string_of(), "upper", 5))
                        value = (int)b.upper ;
                    else
                        value = (int)b.lower;
                    ast = new AstNode(AstNode::Constant, (void*) value);
                }
		break ;
           }// switch
       }
       break ;

  
  case MDL_T_INT:
    if (name_.length()) {
      // variable in the expression.
      mdl_var get_int;
      int value;
      bool aflag;
      aflag=mdl_env::get(get_int, name_);
      assert(aflag);
      if (!get_int.get(value)) {
	  fprintf(stderr, "Unable to get value for %s\n", name_.string_of());
	  fflush(stderr);
	  return false;
      } else {
	  ast = new AstNode(AstNode::Constant, (void*) value);
      }
    } else {
      ast = new AstNode(AstNode::Constant, (void*) val_);
    }
    break;
  case MDL_ARG:
    ast = new AstNode(AstNode::Param, (void*) val_);
    break;
  case MDL_RETURN:
    ast = new AstNode(AstNode::ReturnVal, (void*)0);
    break;
  case MDL_READ_SYMBOL:
    // TODO -- I am relying on global_proc to be set in mdl_metric::apply
    if (global_proc) {
      Symbol info;
      Address baseAddr;
      if (global_proc->getSymbolInfo(name_, info, baseAddr)) {
	Address adr = info.addr();
	ast = new AstNode(AstNode::DataAddr, (void*) adr);
      } else {
	string msg = string("In metric '") + currentMetric + string("': ") +
	  string("unable to find symbol '") + name_ + string("'");
	showErrorCallback(95, msg);
	return false;
      }
    }
    break;
  case MDL_READ_ADDRESS:
    // TODO -- check on the range of this address!
    ast = new AstNode(AstNode::DataAddr, (void*) val_);
    break;
  case MDL_CALL_FUNC: {
    // don't confuse 'args' with 'args_' here!

    vector<AstNode *> args;
    for (unsigned u = 0; u < args_.size(); u++) {
      AstNode *arg=NULL;
      if (!args_[u]->apply(arg)) { 
        // fills in 'arg'
        removeAst(arg);
	return false;
      }
      args += assignAst(arg);
      removeAst(arg);
    }
    string temp = string(name_);
    pdf = global_proc->findOneFunctionFromAll(temp);
    if (!pdf) {
	string msg = string("In metric '") + currentMetric + string("': ") +
	  string("unable to find procedure '") + name_ + string("'");
	showErrorCallback(95, msg);
	return false;
    }
    ast = new AstNode(name_, args); //Cannot use simple assignment here!
    for (unsigned i=0;i<args.size();i++) removeAst(args[i]);
    break;
  }
  case MDL_T_COUNTER_PTR:
    { mdl_var get_drn;
      dataReqNode *drn;
      if (!mdl_env::get(get_drn, name_)) {
	string msg = string("In metric '") + currentMetric + string("' : ") +
	  string("undefined variable '") + name_ + string("'");
	showErrorCallback(92, msg);
	return false;
      }
      bool aflag;
      aflag=get_drn.get(drn);
      assert(aflag);
#if defined(SHM_SAMPLING)
  #if defined(MT_THREAD)
      ast = computeAddress((void *)(drn->getAllocatedLevel()),
			   (void *)(drn->getAllocatedIndex()),
			   0); // 0 is for intCounter
  #else
      ast = new AstNode(AstNode::DataPtr, (void *)(drn->getInferiorPtr(global_proc)));
  #endif
#else
      ast = new AstNode(AstNode::DataPtr, (void *)(drn->getInferiorPtr()));
#endif
    }
    break;
  case MDL_T_COUNTER:
    {
      mdl_var get_drn;
      dataReqNode *drn;
      bool aflag;
      aflag=mdl_env::get(get_drn, name_);
      assert(aflag);
      //
      // This code was added to support additional mdl evaluation time 
      //     variables.  To keep it simple, I left the parser alone and so
      //     any unknown identifier maps to MDL_T_COUNTER.  We lookup the
      //     variable's type here to generate the correct code.  MDL
      //     really should have a general type system and all functions
      //     signatures. - jkh 7/5/95
      //
      switch (get_drn.type()) {
	  case MDL_T_INT:
	      int value;
	      if (!get_drn.get(value)) {
		  fprintf(stderr, "Unable to get value for %s\n", 
		      name_.string_of());
		  fflush(stderr);
		  return false;
	      } else {
		  ast = new AstNode(AstNode::Constant, (void*) value);
	      }
	      break;
	  case MDL_T_COUNTER:	// is MDL_T_COUNTER used here ??? jkh 7/31/95
	  case MDL_T_DRN:
	      if (!get_drn.get(drn)) {
		  fprintf(stderr, "Unable to find variable %s\n", 
		      name_.string_of());
		  fflush(stderr);
		  return false;
	      } else {
#if defined(SHM_SAMPLING)
  #if defined(MT_THREAD)
                  AstNode *tmp_ast;
                  tmp_ast = computeAddress((void *)(drn->getAllocatedLevel()),
					   (void *)(drn->getAllocatedIndex()),
					   0); // 0 is for intCounter
                  // First we get the address, and now we get the value...
		  ast = new AstNode(AstNode::DataIndir,tmp_ast); 
		  removeAst(tmp_ast);
  #else
                  ast = new AstNode(AstNode::DataValue, 
				    (void*)(drn->getInferiorPtr(global_proc)));
  #endif
#else
                  // Note: getInferiorPtr could return a NULL pointer here if
                  // we are just computing cost - naim 2/18/97
		  ast = new AstNode(AstNode::DataValue, 
				    (void*)(drn->getInferiorPtr()));
#endif
	      }
	      break;
	  default:
	      fprintf(stderr, "type of variable %s is not known\n",
		  name_.string_of());
	      fflush(stderr);
	      return false;
      }
    }
    break;
  default:
    break;
  }
  return true;
}



T_dyninstRPC::mdl_instr_req::mdl_instr_req(T_dyninstRPC::mdl_instr_rand *rand,
                                           u_int type, string obj_name)
: type_(type), rand_(rand), timer_counter_name_(obj_name) { }

T_dyninstRPC::mdl_instr_req::mdl_instr_req(u_int type, string obj_name)
: type_(type), timer_counter_name_(obj_name) { }

T_dyninstRPC::mdl_instr_req::mdl_instr_req(u_int type,
                                           T_dyninstRPC::mdl_instr_rand *rand)
: type_(type), rand_(rand) { }

T_dyninstRPC::mdl_instr_req::mdl_instr_req() : type_(0) { }
T_dyninstRPC::mdl_instr_req::~mdl_instr_req() { }

bool T_dyninstRPC::mdl_instr_req::apply(AstNode *&mn, AstNode *pred,
                                        bool mn_initialized) {
  // a return value of true implies that "mn" was written to
  AstNode *ast_arg=NULL;

  vector<AstNode *> ast_args;
  string timer_fun;

  switch (type_) {
  case MDL_SET_COUNTER:
  case MDL_ADD_COUNTER:
  case MDL_SUB_COUNTER:
    if (! rand_->apply(ast_arg))
      return false;
    break;
  case MDL_START_WALL_TIMER:
    timer_fun = START_WALL_TIMER;
    break;
  case MDL_STOP_WALL_TIMER:
    timer_fun = STOP_WALL_TIMER;
    break;
  case MDL_START_PROC_TIMER:
    timer_fun = START_PROC_TIMER;
    break;
  case MDL_STOP_PROC_TIMER:
    timer_fun = STOP_PROC_TIMER;
    break;
  }

  dataReqNode *drn;
  mdl_var get_drn;
  if (type_ != MDL_CALL_FUNC) {
      bool aflag=mdl_env::get(get_drn, timer_counter_name_);
      assert(aflag);

      aflag=get_drn.get(drn);
      assert(aflag);
  }

  AstNode *code=NULL;

  switch (type_) {
  case MDL_SET_COUNTER:
#if defined(SHM_SAMPLING)
  #if defined(MT_THREAD)
    code = createCounter("setCounter", (void *)(drn->getAllocatedLevel()), 
			 (void *)(drn->getAllocatedIndex()),ast_arg);
  #else
    code = createCounter("setCounter", (void *)(drn->getInferiorPtr(global_proc)), ast_arg);
  #endif
#else
    code = createCounter("setCounter", (void *)(drn->getInferiorPtr()), ast_arg);
#endif
    break;
  case MDL_ADD_COUNTER:
#if defined(SHM_SAMPLING)
  #if defined(MT_THREAD)
    code = createCounter("addCounter", (void *)(drn->getAllocatedLevel()), 
			 (void *)(drn->getAllocatedIndex()), ast_arg);
  #else
    code = createCounter("addCounter", (void *)(drn->getInferiorPtr(global_proc)), ast_arg);
  #endif
#else
    code = createCounter("addCounter", (void *)(drn->getInferiorPtr()), ast_arg);
#endif
    break;
  case MDL_SUB_COUNTER:
#if defined(SHM_SAMPLING)
  #if defined(MT_THREAD)
    code = createCounter("subCounter", (void *)(drn->getAllocatedLevel()), 
			 (void *)(drn->getAllocatedIndex()), ast_arg);
  #else
    code = createCounter("subCounter", (void *)(drn->getInferiorPtr(global_proc)), ast_arg);
  #endif
#else
    code = createCounter("subCounter", (void *)(drn->getInferiorPtr()), ast_arg);
#endif
    break;
  case MDL_START_WALL_TIMER:
  case MDL_STOP_WALL_TIMER:
  case MDL_START_PROC_TIMER:
  case MDL_STOP_PROC_TIMER:
#if defined(SHM_SAMPLING)
  #if defined(MT_THREAD)
    code = createTimer(timer_fun, (void *)(drn->getAllocatedLevel()), 
		       (void *)(drn->getAllocatedIndex()), ast_args);
  #else
    code = createTimer(timer_fun, (void *)(drn->getInferiorPtr(global_proc)), ast_args);
  #endif
#else
    code = createTimer(timer_fun, (void *)(drn->getInferiorPtr()), ast_args);
#endif
    break;
  case MDL_CALL_FUNC:
    if (! rand_->apply(code))
      return false;
    break;
  default:
    return false;
  }
  if (pred) {
    // Note: we don't use assignAst on purpose here
    AstNode *tmp=code;
    code = createIf(pred, tmp);
    removeAst(tmp);
  }

  if (mn_initialized) {
    // Note: we don't use assignAst on purpose here
    AstNode *tmp=mn;
    mn = new AstNode(tmp, code);
    removeAst(tmp);
  } else {
    mn = assignAst(code);
  }

  removeAst(ast_arg);
  removeAst(code);
  return true;
}

T_dyninstRPC::mdl_stmt::mdl_stmt() { }

T_dyninstRPC::mdl_for_stmt::mdl_for_stmt(string index_name, T_dyninstRPC::mdl_expr *list_exp, T_dyninstRPC::mdl_stmt *body) 
: for_body_(body), index_name_(index_name), list_expr_(list_exp) { }
T_dyninstRPC::mdl_for_stmt::mdl_for_stmt() { }
T_dyninstRPC::mdl_for_stmt::~mdl_for_stmt() {
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

T_dyninstRPC::mdl_icode::mdl_icode() { }
T_dyninstRPC::mdl_icode::mdl_icode(T_dyninstRPC::mdl_instr_rand *iop1,
                                   T_dyninstRPC::mdl_instr_rand *iop2,
				   u_int bin_op, bool use_if,
				   T_dyninstRPC::mdl_instr_req *ireq)
: if_op1_(iop1), if_op2_(iop2),
  bin_op_(bin_op), use_if_(use_if), req_(ireq) { }
T_dyninstRPC::mdl_icode::~mdl_icode() { delete req_; }

static AstNode *do_rel_op(opCode op, T_dyninstRPC::mdl_instr_rand *if_op2,
                          AstNode *ast_left) {
   // NOTE: ast_left _must_ be defined
   AstNode *ast_right=NULL;
   AstNode *tmp=NULL;
   if (!if_op2->apply(ast_right)) {
      removeAst(ast_right);
      return(new AstNode());
   }
   tmp = new AstNode(op, ast_left, ast_right);
   removeAst(ast_right);
   return(tmp);;
}

bool T_dyninstRPC::mdl_icode::apply(AstNode *&mn, bool mn_initialized) {
  // a return value of true implies that "mn" has been written to
  // TODO -- handle the if case here
  // TODO -- call req_->apply() after building if

  if (!req_)
     return false;

  AstNode *pred=NULL;
  AstNode *pred_ptr=NULL;

  if (use_if_) {
    AstNode *ast1=NULL;
    if (!if_op1_->apply(ast1))
       return false;
    switch (bin_op_) {
    case MDL_LT:
      pred = do_rel_op(lessOp, if_op2_, ast1);
      break;
    case MDL_GT:
      pred = do_rel_op(greaterOp, if_op2_, ast1);
      break;
    case MDL_LE:
      pred = do_rel_op(leOp, if_op2_, ast1);
      break;
    case MDL_GE:
      pred = do_rel_op(geOp, if_op2_, ast1);
      break;
    case MDL_EQ:
      pred = do_rel_op(eqOp, if_op2_, ast1);
      break;
    case MDL_NE:
      pred = do_rel_op(neOp, if_op2_, ast1);
      break;
    case MDL_T_NONE:
      pred = new AstNode(ast1);
      break;
    default: return false;
    }
    removeAst(ast1);
    pred_ptr = new AstNode(pred);
  } // if ()
  else
    pred_ptr = NULL;

  bool result = req_->apply(mn, pred_ptr, mn_initialized);
     // note: a result of true implies that "mn" was written to
     // Hence, a result of true from this routine means the same.

  removeAst(pred);
  removeAst(pred_ptr);
  return result;
}

T_dyninstRPC::mdl_expr::mdl_expr() { }
T_dyninstRPC::mdl_expr::~mdl_expr() { }

T_dyninstRPC::mdl_v_expr::mdl_v_expr() 
: args_(NULL), literal_(0), arg_(0), left_(NULL), right_(NULL), type_(MDL_T_NONE),
  do_type_walk_(false), ok_(false) { }

T_dyninstRPC::mdl_v_expr::mdl_v_expr(string var, vector<string> fields) 
: var_(var), fields_(fields),
  args_(NULL), literal_(0), arg_(0), left_(NULL), right_(NULL),
  type_(MDL_RVAL_DEREF), do_type_walk_(false), ok_(false) { assert(0); }

T_dyninstRPC::mdl_v_expr::mdl_v_expr(string func_name,
				     vector<T_dyninstRPC::mdl_expr *> *a) 
: var_(func_name), args_(a),
  literal_(0), arg_(100000), left_(NULL), right_(NULL),
  type_(MDL_RVAL_FUNC), do_type_walk_(false), ok_(false) { assert(0); }

T_dyninstRPC::mdl_v_expr::mdl_v_expr(int int_lit) 
: args_(NULL), literal_(int_lit), arg_(0), left_(NULL), right_(NULL),
  type_(MDL_RVAL_INT), do_type_walk_(false), ok_(false) { assert(0); }

T_dyninstRPC::mdl_v_expr::mdl_v_expr(string string_lit) 
: var_(string_lit),
  args_(NULL), literal_(0), arg_(0), left_(NULL), right_(NULL),
  type_(MDL_RVAL_STRING), do_type_walk_(false), ok_(false) { assert(0); }

T_dyninstRPC::mdl_v_expr::mdl_v_expr(u_int bin_op, T_dyninstRPC::mdl_expr *left,
				 T_dyninstRPC::mdl_expr *right) 
: args_(NULL), literal_(0),
  arg_(bin_op), left_(left), right_(right),
  type_(MDL_RVAL_EXPR), do_type_walk_(false), ok_(false) { assert(0); }

T_dyninstRPC::mdl_v_expr::mdl_v_expr(string var, u_int array_index) 
: var_(var), args_(NULL), literal_(0), arg_(array_index), left_(NULL), right_(NULL),
  type_(MDL_RVAL_ARRAY), do_type_walk_(false), ok_(false) { assert(0); }

T_dyninstRPC::mdl_v_expr::~mdl_v_expr() {
  delete args_; delete left_; delete right_;
  if (args_) {
    unsigned size = args_->size();
    for (unsigned u=0; u<size; u++)
      delete (*args_)[u];
    delete args_;
  }
}

bool T_dyninstRPC::mdl_v_expr::apply(mdl_var& ret) {
  switch (type_) {
  case MDL_RVAL_INT: 
    return (ret.set(literal_));
  case MDL_RVAL_STRING:
    return (ret.set(var_));
  case MDL_RVAL_ARRAY:
    {
      mdl_var array(false);
      if (!mdl_env::get(array, var_)) return false;
      if (!array.is_list()) return false;  
      if (arg_ >= array.list_size()) return false;
      return (array.get_ith_element(ret, arg_));
    }
  case MDL_RVAL_EXPR:
    {
      mdl_var left_val(false), right_val(false);
      if (!left_ || !right_) return false;
      if (!left_->apply(left_val)) return false;
      if (!right_->apply(right_val)) return false;
      return (do_operation(ret, left_val, right_val, arg_));
    }
  case MDL_RVAL_FUNC:
    // TODO
    switch (arg_) {
    case 0:
      // lookupFunction
      {
	mdl_var arg0(false);
	if (!(*args_)[0]->apply(arg0)) return false;
	string func_name;
	if (!arg0.get(func_name)) return false;
	if (global_proc) {
	  // TODO -- what if the function is not found ?
	  function_base *pdf = global_proc->findOneFunction(func_name);
	  return (ret.set(pdf));
	} else {
	  assert(0); return false;
	}
      }
    case 1:
      // lookupModule
      {
	mdl_var arg0(false);
	if (!(*args_)[0]->apply(arg0)) return false;
	string mod_name;
	if (!arg0.get(mod_name)) return false;
	if (global_proc) {
	  // TODO -- what if the function is not found ?
	  module *mod = global_proc->findModule(mod_name,false);
	  if (!mod) { assert(0); return false; }
	  return (ret.set(mod));
	} else {
	  assert(0); return false;
	}
      }
    case 2:
      // libraryTag
      {
	mdl_var arg0(false);
	if (!(*args_)[0]->apply(arg0)) return false;
	int tag;
	if (!arg0.get(tag)) return false;
	int res = tag & TAG_LIB_FUNC;
	return (ret.set(res));
      }
    default:
      return false;
    }
  case MDL_RVAL_DEREF:
    if (!do_type_walk_)
      return (mdl_env::get(ret, var_));
    else
      return (walk_deref(ret, type_walk, var_)); 
  default:
    return false;
  }
  return true;
}

T_dyninstRPC::mdl_if_stmt::mdl_if_stmt(T_dyninstRPC::mdl_expr *expr, T_dyninstRPC::mdl_stmt *body) : expr_(expr), body_(body) { }
T_dyninstRPC::mdl_if_stmt::mdl_if_stmt() { }
T_dyninstRPC::mdl_if_stmt::~mdl_if_stmt() {
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

T_dyninstRPC::mdl_seq_stmt::mdl_seq_stmt(vector<T_dyninstRPC::mdl_stmt*> *stmts) : stmts_(stmts) { }
T_dyninstRPC::mdl_seq_stmt::mdl_seq_stmt() { }
T_dyninstRPC::mdl_seq_stmt::~mdl_seq_stmt() {
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
: type_(type), id_(ident), elements_(elems), is_lib_(is_lib), flavor_(flavor) { }
T_dyninstRPC::mdl_list_stmt::mdl_list_stmt() { }
T_dyninstRPC::mdl_list_stmt::~mdl_list_stmt() { delete elements_; }

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
  mdl_var expr_var;
  mdl_var list_var(id_, false);
  unsigned size = elements_->size();
  if (!list_var.make_list(type_)) return false;

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
      int i; float f; string s; mdl_var expr_var;
      switch (type_) {
      case MDL_T_INT:
	if (sscanf((*elements_)[u].string_of(), "%d", &i) != 1) return false;
	if (!expr_var.set(i)) return false; break;
      case MDL_T_FLOAT:
	if (sscanf((*elements_)[u].string_of(), "%f", &f) != 1) return false;
	if (!expr_var.set(f)) return false; break;
      case MDL_T_STRING: 
	if (!expr_var.set((*elements_)[u])) return false; break;
      default:
	return false;
      }
      if (!list_var.add_to_list(expr_var)) return false;
    }
  }
  return (mdl_env::add(list_var));
}

T_dyninstRPC::mdl_instr_stmt::mdl_instr_stmt(unsigned pos, T_dyninstRPC::mdl_expr *expr,
				      vector<T_dyninstRPC::mdl_icode*> *reqs,
				      unsigned where, bool constrained) 
: position_(pos), point_expr_(expr), icode_reqs_(reqs),
  where_instr_(where), constrained_(constrained) { }
T_dyninstRPC::mdl_instr_stmt::mdl_instr_stmt() { }
T_dyninstRPC::mdl_instr_stmt::~mdl_instr_stmt() {
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
        tmp_ast = computeAddress((void *)((inFlags[fi])->getAllocatedLevel()),(void *)((inFlags[fi])->getAllocatedIndex()), 0); // 0 is for intCounter
        AstNode *temp1 = new AstNode(AstNode::DataIndir,tmp_ast);
        removeAst(tmp_ast);
  #else
        // Note: getInferiorPtr could return a NULL pointer here if we are
        // just computing cost - naim 2/18/97
        AstNode *temp1 = new AstNode(AstNode::DataValue, 
				     (void*)((inFlags[fi])->getInferiorPtr(global_proc)));
  #endif
#else
        // Note: getInferiorPtr could return a NULL pointer here if we are
        // just computing cost - naim 2/18/97
        AstNode *temp1 = new AstNode(AstNode::DataValue, 
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

  // Here is where auto-activate should occur.
  // If there is 1 point and it is $start.entry then
  // do an inferiorRPC right now (of the ast 'code').
  // In this way, we can get metrics like cpu and exec-time for whole program
  // to work correctly even when the metrics are instantiated after
  // the entrypoint of main() is in execution.

  bool manuallyTrigger = false; // for now

  if (points.size() == 1) {
     // now look at the mdl variable to check for $start.entry.
     if (pointsVar.name() == "$start" && pointsVar.type()==MDL_T_POINT) {
        // having a type of MDL_T_POINT should mean $start.entry as opposed to
        // $start.exit, since $start.exit would yield a type of MDL_T_LIST_POINT,
        // since exit locations are always a list-of-points.  Sorry for the kludge.

        instPoint *theVrbleInstPoint; // NOTE: instPoint is defined in arch-specific files!!!
	bool aflag = pointsVar.get(theVrbleInstPoint);
        // theVrbleInstPoint set to equiv of points[0]
        assert(aflag);

	assert(theVrbleInstPoint == points[0]); // just a sanity check

	mdl_var theVar;
	string varName = pointsVar.name();
        aflag=mdl_env::get(theVar, varName);
	assert(aflag);

	function_base *theFunction;
	aflag=theVar.get(theFunction);
	assert(aflag);

	// Make a note to do an inferiorRPC to manually execute this code.
//	if (!manuallyTrigger)
//	   cerr << "mdl: found $start.entry; going to manually execute via inferior-RPC" << endl;

	manuallyTrigger = true;
     }
  }

  // for all of the inst points, insert the predicates and the code itself.
  for (unsigned i = 0; i < points.size(); i++) {
      mn->addInst(points[i], code, cwhen, corder,
		  manuallyTrigger && (i==0)); // manually trigger at most once
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

  return false;
}

metricDefinitionNode *mdl_do(vector< vector<string> >& canon_focus, 
                             string& met_name,
			     string& flat_name,
			     vector<process *> procs,
			     bool replace_components_if_present,
			     bool computingCost) {
  currentMetric = met_name;
  unsigned size = mdl_data::all_metrics.size();
  // NOTE: We can do better if there's a dictionary of <metric-name> to <metric>!
  for (unsigned u=0; u<size; u++) {
    if (mdl_data::all_metrics[u]->name_ == met_name) {
      return (mdl_data::all_metrics[u]->apply(canon_focus, flat_name, procs,
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

  self.name = "Process"; self.type = MDL_T_PROCESS; self.end_allowed = true;
  fe.self = self; fe.kids.resize(0);
  mdl_data::foci += fe;

  self.name = "Machine"; self.type = MDL_T_STRING; self.end_allowed = true;
  fe.self = self; fe.kids.resize(0);
  mdl_data::foci += fe;
#endif

  mdl_env::push();
  // These are pushed on per-process
  // mdl_env::add("$procedures", false, MDL_T_LIST_PROCEDURE);
  // mdl_env::add("$modules", false, MDL_T_LIST_MODULE);

  string vname = "$machine";
  mdl_env::add(vname, false, MDL_T_STRING);
  string nodename = getHostName();
  mdl_env::set(nodename, vname);

  /* Are these entered by hand at the new scope ? */
  /* $arg, $return */

  vector<mdl_type_desc> field_list;
  mdl_type_desc desc;
  desc.name = "name"; desc.type = MDL_T_STRING; field_list += desc;
  desc.name = "calls"; desc.type = MDL_T_LIST_POINT; field_list += desc;
  desc.name = "entry"; desc.type = MDL_T_POINT; field_list += desc;
  desc.name = "return"; desc.type = MDL_T_POINT; field_list += desc;
  desc.name = "tag"; desc.type = MDL_T_INT; field_list += desc;
  mdl_data::fields[MDL_T_PROCEDURE] = field_list;
  field_list.resize(0);

  desc.name = "name"; desc.type = MDL_T_STRING; field_list += desc;
  desc.name = "funcs"; desc.type = MDL_T_LIST_PROCEDURE; field_list += desc;
  mdl_data::fields[MDL_T_MODULE] = field_list;
  field_list.resize(0);

  desc.name = "upper"; desc.type = MDL_T_INT; field_list += desc;
  desc.name = "lower"; desc.type = MDL_T_INT; field_list += desc;
  mdl_data::fields[MDL_T_VARIABLE] = field_list;
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
    for(u_int i=0; i < libs->size(); i++){
	mdl_data::lib_constraints += (*libs)[i]; 
    }

}

// recieves notification that there are no excluded libraries 
void dynRPC::send_no_libs() {
    mdl_libs = true;
}

static bool do_operation(mdl_var& ret, mdl_var& left_val,
			 mdl_var& right_val, unsigned bin_op) {
  switch (bin_op) {
  case MDL_PLUS:
  case MDL_MINUS:
  case MDL_DIV:
  case MDL_MULT:
    if ((left_val.type() == MDL_T_INT) && (right_val.type() == MDL_T_INT)) {
      int v1, v2;
      if (!left_val.get(v1)) return false;
      if (!right_val.get(v2)) return false;
      switch (bin_op) {
      case MDL_PLUS: return (ret.set(v1+v2));
      case MDL_MINUS: return (ret.set(v1-v2));
      case MDL_DIV: return (ret.set(v1/v2));
      case MDL_MULT: return (ret.set(v1*v2));
      }
    } else if (((left_val.type() == MDL_T_INT) || (left_val.type() == MDL_T_FLOAT)) &&
	       ((right_val.type() == MDL_T_INT) || (right_val.type() == MDL_T_FLOAT))) {
      float v1, v2;
      if (left_val.type() == MDL_T_INT) {
	int i1; if (!left_val.get(i1)) return false; v1 = (float)i1;
      } else {
	if (!left_val.get(v1)) return false;
      }
      if (right_val.type() == MDL_T_INT) {
	int i1; if (!right_val.get(i1)) return false; v2 = (float)i1;
      } else {
	if (!right_val.get(v2)) return false;
      }
      switch (bin_op) {
      case MDL_PLUS: return (ret.set(v1+v2));
      case MDL_MINUS: return (ret.set(v1-v2));
      case MDL_DIV: return (ret.set(v1/v2));
      case MDL_MULT: return (ret.set(v1*v2));
      }
    } else
      return false;
  case MDL_LT:
  case MDL_GT:
  case MDL_LE:
  case MDL_GE:
  case MDL_EQ:
  case MDL_NE:
    if ((left_val.type() == MDL_T_STRING) && (right_val.type() == MDL_T_STRING)) {
      string v1, v2;
      if (!left_val.get(v1)) return false;
      if (!right_val.get(v2)) return false;
      switch (bin_op) {
      case MDL_LT: return (ret.set(v1 < v2));
      case MDL_GT: return (ret.set(v1 > v2));
      case MDL_LE: return (ret.set(v1 <= v2));
      case MDL_GE: return (ret.set(v1 >= v2));
      case MDL_EQ: return (ret.set(v1 == v2));
      case MDL_NE: return (ret.set(v1 != v2));
      }  
    }
    if ((left_val.type() == MDL_T_INT) && (right_val.type() == MDL_T_INT)) {
      int v1, v2;
      if (!left_val.get(v1)) return false;
      if (!right_val.get(v2)) return false;
      switch (bin_op) {
      case MDL_LT: return (ret.set(v1 < v2));
      case MDL_GT: return (ret.set(v1 > v2));
      case MDL_LE: return (ret.set(v1 <= v2));
      case MDL_GE: return (ret.set(v1 >= v2));
      case MDL_EQ: return (ret.set(v1 == v2));
      case MDL_NE: return (ret.set(v1 != v2));
      }
    } else if (((left_val.type() == MDL_T_INT) ||
		(left_val.type() == MDL_T_FLOAT)) &&
	       ((right_val.type() == MDL_T_INT) ||
		(right_val.type() == MDL_T_FLOAT))) {
      float v1, v2;
      if (left_val.type() == MDL_T_INT) {
	int i1; if (!left_val.get(i1)) return false; v1 = (float)i1;
      } else {
	if (!left_val.get(v1)) return false;
      }
      if (right_val.type() == MDL_T_INT) {
	int i1; if (!right_val.get(i1)) return false; v2 = (float)i1;
      } else {
	if (!right_val.get(v2)) return false;
      }
      switch (bin_op) {
      case MDL_LT: return (ret.set(v1 < v2));
      case MDL_GT: return (ret.set(v1 > v2));
      case MDL_LE: return (ret.set(v1 <= v2));
      case MDL_GE: return (ret.set(v1 >= v2));
      case MDL_EQ: return (ret.set(v1 == v2));
      case MDL_NE: return (ret.set(v1 != v2));
      }
    } else
      return false;
  case MDL_AND:
  case MDL_OR:
    if ((left_val.type() == MDL_T_INT) && (right_val.type() == MDL_T_INT)) {
      int v1, v2;
      if (!left_val.get(v1)) return false;
      if (!right_val.get(v2)) return false;
      switch (bin_op) {
      case MDL_AND: return (ret.set(v1 && v2));
      case MDL_OR: return (ret.set(v1 || v2));
      }
    } else
      return false;
  default:
      return false;
  }
  return false;
}

static bool walk_deref(mdl_var& ret, vector<unsigned>& types, string& var_name) {

  function_base *pdf = 0;
  if (!mdl_env::get(ret, var_name)) return false;
  
  unsigned index=0;
  unsigned max = types.size();

  while (index < max) {
    unsigned current_type = types[index++];
    unsigned next_field = types[index++];

    switch (current_type) {
    case MDL_T_PROCEDURE:
      // function_base *pdf = 0;
      if (!ret.get(pdf)) return false;
      switch (next_field) {
      case 0: {
	string prettyName = pdf->prettyName();
	if (!ret.set(prettyName)) return false;
	break;
      }
      // TODO: should these be passed a process?  yes, they definitely should!
      case 1:
	{
	  //
	  /*****
	    here we should check the calls and exclude the calls to fns in the
	    global_excluded_funcs list.
	    *****/
	  // ARI -- This is probably the spot!

	  vector<instPoint*> calls = pdf->funcCalls(global_proc);
	  // makes a copy of the return value (on purpose), since we may delete some
	  // items that shouldn't be a call site for this metric.
	  bool anythingRemoved = false; // so far

	  metric_cerr << "global_excluded_funcs size is: "
	              << global_excluded_funcs.size() << endl;

 	  metric_cerr << "pdf->funcCalls() returned the following call sites:" << endl;
 	  for (unsigned u = 0; u < calls.size(); u++) { // calls.size() can change!
 	     metric_cerr << u << ") ";

 	     instPoint *point = calls[u];
 	     function_base *callee = (function_base*)point->iPgetCallee();
	        // cast discards const

 	     const char *callee_name=NULL;

 	     if (callee == NULL) {
   	        // call Tia's new process::findCallee() to fill in point->callee
 	        if (!global_proc->findCallee(*point, callee)) {
 		   // an unanalyzable function call; sorry.
 		   callee_name = NULL;
 		   metric_cerr << "-unanalyzable-" << endl;
 		}
 		else {
 		   // success -- either (a) the call has been bound already, in which
		   // case the instPoint is updated _and_ callee is set, or (b) the
		   // call hasn't yet been bound, in which case the instPoint isn't
		   // updated but callee *is* updated.
 		   callee_name = callee->prettyName().string_of();
 		   metric_cerr << "(successful findCallee() was required) "
                                << callee_name << endl;
 		}
 	     }
 	     else {
 	        callee_name = callee->prettyName().string_of();
 	        metric_cerr << "(easy case) " << callee->prettyName() << endl;
 	     }

	     // If this callee is in global_excluded_funcs for this metric (a global
	     // vrble...sorry for that), then it's not really a callee (for this
	     // metric, at least), and thus, it should be removed from whatever
	     // we eventually pass to "ret.set()" below.

	     if (callee_name != NULL) // could be NULL (e.g. indirect fn call)
	        for (unsigned lcv=0; lcv < global_excluded_funcs.size(); lcv++) {
		  if (0==strcmp(global_excluded_funcs[lcv].string_of(), callee_name)) {
		      anythingRemoved = true;

		      // remove calls[u] from calls.  To do this, swap
		      // calls[u] with calls[maxndx], and resize-1.
		      const unsigned maxndx = calls.size()-1;
		      calls[u] = calls[maxndx];
		      calls.resize(maxndx);

		      metric_cerr << "removed something! -- " << callee_name << endl;

		      break;
		  }
		}
 	  }

	  if (!anythingRemoved) {
	     metric_cerr << "nothing was removed -- doing set() now" << endl;
	     vector<instPoint*> *setMe = &pdf->funcCalls(global_proc);
	     if (!ret.set(setMe))
	        return false;
	  }
	  else {
	     metric_cerr << "something was removed! -- doing set() now" << endl;
	     vector<instPoint*> *setMe = new vector<instPoint*>(calls);
	     assert(setMe);
	     
	     if (!ret.set(setMe))
	        return false;

	     // WARNING: "setMe" will now be leaked memory!  The culprit is
	     // "ret", which can only take in a _pointer_ to a vector of instPoint*'s;
	     // it can't take in a vector of instPoints*'s, which we'd prefer.
	  }
	}
	break;
      case 2: 
	if (!ret.set((instPoint *)pdf->funcEntry(global_proc)))
	return false; 
	break;
      case 3:
	if (!ret.set((vector<instPoint *>*)&pdf->funcExits(global_proc)))
	  return false;
	break;
      case 4:
	if (!ret.set((int)pdf->tag()))
	  return false;
	break;
      default:
	assert(0);
	break;
      }
      break;
    case MDL_T_MODULE:
      module *mod;
      if (!ret.get(mod)) return false;
      switch (next_field) {
      case 0: { string fileName = mod->fileName();
		if (!ret.set(fileName)) return false; 
	      } break;
      case 1: {
	if (global_proc) {
	    // this is the correct thing to do...get only the included funcs
	    // associated with this module, but since we seem to be testing
	    // for global_proc elsewhere in this file I guess we will here too
	    if (!ret.set(global_proc->getIncludedFunctions(mod))) return false; 
        }
	else {
	    // if there is not a global_proc, then just get all functions
	    // associtated with this module....under what circumstances
	    // would global_proc == 0 ???
	    if (!ret.set(mod->getFunctions())) return false; 
	}
	break;
      }
      default: assert(0); break;	       
      }
      break;
    default:
      assert(0); return false;
    }
  }
  return true;
}


bool mdl_get_initial(string flavor, pdRPC *connection) {
  mdl_init(flavor);
  while (!(mdl_met && mdl_cons && mdl_stmt && mdl_libs)) {
    switch (connection->waitLoop()) {
    case T_dyninstRPC::error:
      return false;
    default:
      break;
    }
    while (connection->buffered_requests()) {
      switch (connection->process_buffered()) {
      case T_dyninstRPC::error:
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

bool T_dyninstRPC::mdl_list_stmt::mk_list(vector<string> &funcs);
bool T_dyninstRPC::mdl_for_stmt::mk_list(vector<string> &funcs);
bool T_dyninstRPC::mdl_if_stmt::mk_list(vector<string> &funcs);
bool T_dyninstRPC::mdl_seq_stmt::mk_list(vector<string> &funcs);
bool T_dyninstRPC::mdl_instr_stmt::mk_list(vector<string> &funcs);

bool T_dyninstRPC::mdl_v_expr::mk_list(vector<string> &funcs);

bool T_dyninstRPC::mdl_v_expr::mk_list(vector<string> &funcs) {
  switch (type_) {
  case MDL_RVAL_INT: 
  case MDL_RVAL_STRING:
    return true;
  case MDL_RVAL_ARRAY:
    {
      mdl_var array(false);
      mdl_var elem(false);
      if (!mdl_env::get(array, var_)) return false;
      if (!array.is_list()) return false;
      if (arg_ >= array.list_size()) return false;
      if (!array.get_ith_element(elem, arg_)) return false;
      if (elem.get_type() == MDL_T_PROCEDURE_NAME) {
	functionName *fn;
	elem.get(fn);
	funcs += fn->get();
      }
      return true;
    }
  case MDL_RVAL_EXPR: {
      if (!left_ || !right_) return false;
      if (!left_->mk_list(funcs)) return false;
      if (!right_->mk_list(funcs)) return false;
      return true;
  }
  case MDL_RVAL_FUNC:
    return true;
    // TODO: should we add anything to the list here?
    // It seems that lookupFunction and lookupModule are useless, they can never
    // be used in a valid MDL expression.
#ifdef notdef
    switch (arg_) {
    case 0:
      // lookupFunction
      {
       mdl_var arg0(false);
       if (!(*args_)[0]->apply(arg0)) return false;
       string func_name;
       if (!arg0.get(func_name)) return false;
       if (global_proc) {
          // TODO -- what if the function is not found ?
         function_base *pdf = global_proc->findOneFunction(func_name);
         return (ret.set(pdf));
       } else {
         assert(0); return false;
       }
      }
    case 1:
      // lookupModule
      {
       mdl_var arg0(false);
       if (!(*args_)[0]->apply(arg0)) return false;
       string mod_name;
       if (!arg0.get(mod_name)) return false;
       if (global_proc) {
         // TODO -- what if the function is not found ?
         module *mod = global_proc->findModule(mod_name,false);
         if (!mod) { assert(0); return false; }
         return (ret.set(mod));
       } else {
         assert(0); return false;
       }
      }
    case 2:
      break;
    default:
      return false;
 }
#endif
  case MDL_RVAL_DEREF:
    return true;
    break;
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
