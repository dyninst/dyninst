


/* 
 * $Log: mdl.C,v $
 * Revision 1.27  1996/04/30 16:17:05  lzheng
 * The name of the functions to be instrumented for timer now become machine
 * dependent and the definations are moved to $(PLATFORM).h
 *
 * Revision 1.26  1996/04/29 22:09:42  mjrg
 * replaced an assert by an error checking
 *
 * Revision 1.25  1996/03/26 21:02:00  tamches
 * fixed a compile problem w/ previous commit
 * fixed a problem w/ prev commit by adding the line
 *       mn->addInst(p, code, cwhen, corder);
 * back in.
 *
 * Revision 1.24  1996/03/25 22:58:10  hollings
 * Support functions that have multiple exit points.
 *
 * Revision 1.23  1996/03/25  20:22:21  tamches
 * the reduce-mem-leaks-in-paradynd commit
 *
 * Revision 1.22  1996/03/20 17:02:51  mjrg
 * Added multiple arguments to calls.
 * Instrument pvm_send instead of pvm_recv to get tags.
 *
 * Revision 1.21  1996/03/12 20:48:22  mjrg
 * Improved handling of process termination
 * New version of aggregateSample to support adding and removing components
 * dynamically
 * Added error messages
 *
 * Revision 1.20  1996/03/01 22:35:56  mjrg
 * Added a type to resources.
 * Changes to the MDL to handle the resource hierarchy better.
 *
 * Revision 1.19  1996/02/02 14:31:30  naim
 * Eliminating old definition for observed cost - naim
 *
 * Revision 1.18  1996/02/01  17:42:26  naim
 * Redefining smooth_obs_cost, fixing some bugs related to internal metrics
 * and adding a new definition for observed_cost - naim
 *
 * Revision 1.17  1996/01/29  22:09:26  mjrg
 * Added metric propagation when new processes start
 * Adjust time to account for clock differences between machines
 * Daemons don't enable internal metrics when they are not running any 
 * processes
 * Changed CM5 start (paradynd doesn't stop application at first breakpoint;
 * the application stops only after it starts the CM5 daemon)
 *
 * Revision 1.16  1995/12/26 23:04:16  zhichen
 * Introduced START_NAME macro; usually set to "main", but paradyndCM5_blz
 * sets it to init_blk_acc.
 *
 * Revision 1.15  1995/12/18  23:27:02  newhall
 * changed metric's units type to have one of three values (normalized,
 * unnormalized, or sampled)
 *
 * Revision 1.14  1995/12/15 22:26:54  mjrg
 * Merged paradynd and paradyndPVM
 * Get module name for functions from symbol table in solaris
 * Fixed code generation for multiple instrumentation statements
 * Changed syntax of MDL resource lists
 *
 * Revision 1.13  1995/11/30 16:54:11  naim
 * Minor fix - naim
 *
 * Revision 1.12  1995/11/28  15:55:45  naim
 * Minor fix - naim
 *
 * Revision 1.11  1995/11/22  00:10:05  mjrg
 * Fixed "constrained" feature of MDL (fixed by Jeff, commited by mjrg)
 *
 * Revision 1.10  1995/11/21 15:14:27  naim
 * Changing the MDL grammar to allow more flexible metric definitions (i.e. we
 * can specify all elements in any order). Additionally, the option "fold"
 * has been removed - naim
 *
 * Revision 1.9  1995/11/17  17:24:26  newhall
 * support for MDL "unitsType" option, added normalized member to metric class
 *
 * Revision 1.8  1995/11/13  14:52:55  naim
 * Adding "mode" option to the Metric Description Language to allow specificacion
 * of developer mode for metrics (default mode is "normal") - naim
 *
 * Revision 1.7  1995/09/26  20:28:47  naim
 * Minor warning fixes and some other minor error messages fixes
 *
 * Revision 1.6  1995/09/18  18:31:01  newhall
 * fixed switch stmt. scope problem
 *
 * Revision 1.5  1995/08/24  15:04:14  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 *
 */

#include <iostream.h>
#include <stdio.h>
#include "dyninstRPC.xdr.SRVR.h"
#include "paradyn/src/met/globals.h"
#include "paradynd/src/metric.h"
#include "paradynd/src/inst.h"
#include "paradynd/src/ast.h"
#include "paradynd/src/main.h"
#include "paradynd/src/symtab.h"
#include "util/h/Timer.h"
#include <strstream.h>
#include "paradynd/src/mdld.h"
#include "showerror.h"

#ifdef paradyndCM5_blizzard
// START_NAME corresponds some routine with $start of your mdl
// Usually, main is what you want; blizzard needs something different
#define START_NAME "init_blk_acc" 
#else
#define START_NAME "main"
#endif

// Some global variables used to print error messages:
string currentMetric;  // name of the metric that is being processed.
string currentFocus;   // the focus


static string daemon_flavor;
static process *global_proc = NULL;
static bool mdl_met=false, mdl_cons=false, mdl_stmt=false;

inline unsigned ui_hash(const unsigned &u) { return u; }

vector<unsigned> mdl_env::frames;
vector<mdl_var> mdl_env::all_vars;

vector<T_dyninstRPC::mdl_stmt*> mdl_data::stmts;
vector<T_dyninstRPC::mdl_metric*> mdl_data::all_metrics;
dictionary_hash<unsigned, vector<mdl_type_desc> > mdl_data::fields(ui_hash);
vector<mdl_focus_element> mdl_data::foci;
vector<T_dyninstRPC::mdl_constraint*> mdl_data::all_constraints;

static bool walk_deref(mdl_var& ret, vector<unsigned>& types, string& var_name);
static bool do_operation(mdl_var& ret, mdl_var& left, mdl_var& right, unsigned bin_op);

class list_closure {
public:
  list_closure(string& i_name, mdl_var& list_v)
    : index_name(i_name), element_type(0), index(0), int_iter(NULL), float_iter(NULL),
  string_iter(NULL), bool_iter(NULL), func_iter(NULL), mod_iter(NULL), max(0)
  {
    assert(list_v.is_list());

    element_type = list_v.element_type();
    switch(element_type) {
    case MDL_T_INT:
      assert(list_v.get(int_iter)); max = int_iter->size(); break;
    case MDL_T_FLOAT:
      assert(list_v.get(float_iter)); max = float_iter->size(); break;
    case MDL_T_STRING:
      assert(list_v.get(string_iter)); max = string_iter->size(); break;
    case MDL_T_PROCEDURE:
      assert(list_v.get(func_iter));
      max = func_iter->size();
      break;
    case MDL_T_MODULE:
      assert(list_v.get(mod_iter));
      max = mod_iter->size();
      break;
    case MDL_T_POINT:
      assert(list_v.get(point_iter));
      max = point_iter->size();
      break;
    default:
      assert(0);
    }
  }
  ~list_closure() { }
  bool next() {
    string s;
    pdFunction *pdf; module *m;
    float f; int i;
    instPoint *ip;

    if (index >= max) return false;
    switch(element_type) {
    case MDL_T_INT:
      i = (*int_iter)[index++];      return (mdl_env::set(i, index_name));
    case MDL_T_FLOAT:
      f = (*float_iter)[index++];    return (mdl_env::set(f, index_name));
    case MDL_T_STRING:
      s = (*string_iter)[index++];   return (mdl_env::set(s, index_name));
    case MDL_T_PROCEDURE:
      pdf = (*func_iter)[index++];
      // cout << "Function: " << pdf->prettyName() << endl;
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
  vector<pdFunction*> *func_iter;
  vector<module*> *mod_iter;
  vector<instPoint*> *point_iter;
  unsigned max;
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

static inline bool other_machine_specified(vector< vector<string> > &focus,
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

static inline void add_processes(vector< vector<string> > &focus,
				 vector<process*> procs,
				 vector<process*> &ip) {
  assert(focus[resource::process][0] == "Process");
  unsigned pi, ps;

  switch(focus[resource::process].size()) {
  case 1:
#ifdef sparc_tmc_cmost7_3
    assert(nodePseudoProcess);
    ip += nodePseudoProcess;
#else
    ip = procs;
#endif    
    break;
  case 2:
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

static inline bool focus_matches(vector<string>& focus, vector<string> *match_path) {
  unsigned mp_size = match_path->size();
  unsigned f_size = focus.size();
  if (mp_size < 1 || f_size < 2 || mp_size != f_size - 1)
    return false;

  // need special rule to check for /Code/Module
  if (focus[0] == "Code" && (*match_path)[0] == "Code" && mp_size == 2 &&
      (*match_path)[1] == "Module")
    return true;

  for (unsigned u = 0; u < mp_size; u++)
    if (focus[u] != (*match_path)[u])
      return false;
  return true;
}

#ifdef notdef
// TODO -- determine if a focus matches
// What are the rules ?
static inline bool focus_matches(vector<string>& focus, vector<string> *match_path) {
  unsigned mp_size = match_path->size();
  unsigned f_size = focus.size();
  if (!mp_size) return false;
  if (f_size < 2) return false;
  if (mp_size != (f_size - 1)) return false;

  if (focus[0] == (*match_path)[0])
    return true;
  else
    return false;

  // TODO -- I am ignoring intermediate fields in the match path
  //      -- I only care about the first and the last fields
}
#endif

// Global constraints are specified by giving their name within a metric def
// Find the real constraint by searching the dictionary using this name
static inline T_dyninstRPC::mdl_constraint *flag_matches(vector<string>& focus, 
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

static inline bool check_constraints(vector<T_dyninstRPC::mdl_constraint*>& flag_cons,
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
	string focus_str;
	for (unsigned u = 0; u < focus.size(); u++) {
	  focus_str += string("/") + focus[u][0];
	  for (unsigned v = 1; v < focus[u].size(); v++) {
	    focus_str += string(",") + focus[u][v];
	  }
	}
	string msg = string("Metric ") + currentMetric + 
               string(" has no constraint that matches focus ") + 
               focus_str;
	showErrorCallback(93, msg.string_of());
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

static inline bool update_environment(process *proc) {
  string vname = "$exit";
  pdFunction *pdf = proc->symbols->findOneFunction(string(EXIT_NAME));
  if (!pdf) return false;
  mdl_env::add(vname, false, MDL_T_PROCEDURE);
  mdl_env::set(pdf, vname);

  vname = "$start";
  pdf = proc->symbols->findOneFunction(string(START_NAME));
  if (!pdf) return false;
  mdl_env::add(vname, false, MDL_T_PROCEDURE);
  mdl_env::set(pdf, vname);

  vname = "$procedures";
  mdl_env::add(vname, false, MDL_T_LIST_PROCEDURE);
  mdl_env::set(&proc->symbols->mdlNormal, vname);

  vname = "$modules";
  mdl_env::add(vname, false, MDL_T_LIST_MODULE);
  mdl_env::set(&proc->symbols->mods, vname);
  return true;
}

inline dataReqNode *create_data_object(unsigned mdl_data_type,
				       metricDefinitionNode *mn) {
  switch (mdl_data_type) {
  case MDL_T_COUNTER:
    return (mn->addIntCounter(0, true));
  case MDL_T_WALL_TIMER:
    return (mn->addTimer(wallTime));
  case MDL_T_PROC_TIMER:
    return (mn->addTimer(processTime));
  case MDL_T_NONE:
    // just to keep mdl apply allocate a dummy un-sampled counter.
    return (mn->addIntCounter(0, false));
  default:
    assert(0);
    return NULL;
  }
}

static bool apply_to_process_list(vector<process*>& instProcess,
				  vector<metricDefinitionNode*>& parts,
				  string& id, string& name,
				  vector< vector<string> >& focus,
				  string& flat_name,
				  unsigned& agg_op,
				  unsigned& type,
				  vector<T_dyninstRPC::mdl_constraint*>& flag_cons,
				  T_dyninstRPC::mdl_constraint *base_use,
				  vector<T_dyninstRPC::mdl_stmt*> *stmts,
				  vector<unsigned>& flag_dex,
				  unsigned& base_dex,
				  vector<string> *temp_ctr) {
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

    // skip exited processes.
    if (proc->status() == exited) continue;

    if (!update_environment(proc)) return false;

    // TODO -- Using aggOp value for this metric -- what about folds
    metricDefinitionNode *mn = new metricDefinitionNode(proc, name,
							focus, flat_name, agg_op);

    // Create the timer, counter
    dataReqNode *the_node = create_data_object(type, mn);
    assert(the_node);
    mdl_env::set(the_node, id);

    // Create the temporary counters - are these useful
    if (temp_ctr) {
      unsigned tc_size = temp_ctr->size();
      for (unsigned tc=0; tc<tc_size; tc++) {
	dataReqNode *temp_node;
	temp_node = mn->addIntCounter(0, false);
	mdl_env::set(temp_node, (*temp_ctr)[tc]);
      }
    }

    unsigned flag_size = flag_cons.size();
    vector<dataReqNode*> flags;
    if (flag_size) {
      for (unsigned fs=0; fs<flag_size; fs++) {
	// TODO -- cache these created flags
	dataReqNode *flag = NULL;
	assert(flag_cons[fs]->apply(mn, flag, focus[flag_dex[fs]], proc));
	assert(flag);
	flags += flag;
	// cout << "Applying constraint for " << flag_cons[fs]->id_ << endl;
      }
    }

    if (base_use) {
      dataReqNode *flag = NULL;
      if (!base_use->apply(mn, flag, focus[base_dex], proc)) {
	// cout << "apply of " << name << " failed\n";
	return false;
      }
    } else {
      unsigned size = stmts->size();
      for (unsigned u=0; u<size; u++) {
	if (!(*stmts)[u]->apply(mn, flags)) {
	  // cout << "apply of " << name << " failed\n";
	  return false;
	}
      }
    }

    if (mn && mn->nonNull()) 
      parts += mn;
    else {
      delete mn; mn = NULL;
    }
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

  return true;
}

metricDefinitionNode *T_dyninstRPC::mdl_metric::apply(vector< vector<string> > &focus,
						      string& flat_name, vector<process *> procs) {

  // TODO -- check to see if this is active ?
  // TODO -- create counter or timer
  // TODO -- put it into the environment ?
  // TODO -- this can be passed directly -- faster - later
  // TODO -- internal metrics
  // TODO -- what the heck is nodePseudoProcess, why should I care about it
  // TODO -- assume no constraints, all processes instrumented
  // TODO -- support two-level aggregation: one at the daemon, one at paradyn
  // TODO -- how is folding specified ?
  // TODO -- are lists updated here ?

  mdl_env::push();
  mdl_env::add(id_, false, MDL_T_DRN);
  assert(stmts_);
  vector<metricDefinitionNode*> parts;

  const unsigned tc_size = temp_ctr_->size();
  for (unsigned tc=0; tc<tc_size; tc++) {
    mdl_env::add((*temp_ctr_)[tc], false, MDL_T_DRN);
  }

  static string machine;
  static bool machine_init= false;
  if (!machine_init) {
    machine_init = true;
    struct utsname un; assert(!P_uname(&un) != -1); machine = un.nodename;
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

  // build the instrumentation request
  if (!apply_to_process_list(instProcess, parts, id_, name_, focus,
			     flat_name, agg_op_, type_, flag_cons, base_used,
			     stmts_, flag_dex, base_dex, temp_ctr_))
    return NULL;

  // construct aggregate for the metric instance parts
  metricDefinitionNode *ret = NULL;

  //  switch (parts.size()) {
  //  case 0: break;
  //  case 1: ret = parts[0]; break;
  //  default: ret = new metricDefinitionNode(name_, focus, flat_name, parts);
  //  }

  if (parts.size())
    ret = new metricDefinitionNode(name_, focus, flat_name, parts, agg_op_);

  if (ret) ret->set_inform(true);

  // cout << "apply of " << name_ << " ok\n";
  mdl_env::pop();
  return ret;
}

T_dyninstRPC::mdl_constraint::mdl_constraint() { }
T_dyninstRPC::mdl_constraint::mdl_constraint(string id, vector<string> *match_path,
					     vector<T_dyninstRPC::mdl_stmt*> *stmts,
					     bool replace, u_int d_type, bool& error)
: id_(id), match_path_(match_path), stmts_(stmts), replace_(replace),
  data_type_(d_type), hierarchy_(0), type_(0) { }
T_dyninstRPC::mdl_constraint::~mdl_constraint() {
  delete match_path_;
  if (stmts_) {
    for (unsigned u=0; u<stmts_->size(); u++)
      delete (*stmts_)[u];
    delete stmts_;
  }
}

// determine the type of the trailing part of the constraint's match path
// and put this into the environment
static bool do_trailing_resource(vector<string>& resource_, process *proc) {
  string c_string = "$constraint";
  assert(resource_.size());
  string trailing_res = resource_[resource_.size()-1];

  resource *r = resource::findResource(resource_);
  if (!r) {
    // internal error
     assert(0); 
  }

  switch (r->type()) {
  case MDL_T_INT: 
    {
      const char *p = trailing_res.string_of();
      char *q;
      int val = (int) strtol(p, &q, 0);
      if (p == q) {
	string msg = string("unable to convert resource '") + trailing_res + 
	             string("' to integer.");
	showErrorCallback(92,msg.string_of());
	return false;
      }
      mdl_env::add(c_string, false, MDL_T_INT);
      mdl_env::set(val, c_string);
    }
    break;
  case MDL_T_STRING:
    mdl_env::add(c_string, false, MDL_T_STRING);
    mdl_env::set(trailing_res, c_string);
    break;
  case MDL_T_PROCEDURE:
    {
      pdFunction *pdf = proc->symbols->findOneFunction(trailing_res);
      if (!pdf) {
	
	return false;
      }
      mdl_env::add(c_string, false, MDL_T_PROCEDURE);
      mdl_env::set(pdf, c_string);
    }
    break;
  case MDL_T_MODULE:
    {
      module *mod = proc->symbols->findModule(trailing_res);
      if (!mod) return false;
      mdl_env::add(c_string, false, MDL_T_MODULE);
      mdl_env::set(mod, c_string);
    }
    break;
  default:
    assert(0);
    break;
  }
  return true;
}

// Replace constraints not working yet
// Flag constraints need to return a handle to a data request node -- the flag
bool T_dyninstRPC::mdl_constraint::apply(metricDefinitionNode *mn,
					 dataReqNode *&flag,
					 vector<string>& resource,
					 process *proc) {
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
    dataReqNode *drn = mn->addIntCounter(0, false);
    // this flag will construct a predicate for the metric -- have to return it
    flag = drn;
    assert(drn);
    mdl_env::set(drn, id_);
  }

  // put "$constraint" into the environment
  if (!do_trailing_resource(resource, proc)) {
    mdl_env::pop();
    return false;
  }

  // Now evaluate the constraint statements
  unsigned size = stmts_->size();
  vector<dataReqNode*> flags;
  for (unsigned u=0; u<size; u++) {
    if (!(*stmts_)[u]->apply(mn, flags)) {
      // cout << "apply of constraint " << id_ << " failed\n";
      return false;
    }
  }
  mdl_env::pop();
  return true;
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

bool T_dyninstRPC::mdl_instr_rand::apply(AstNode &ast) {
  pdFunction *pdf;
  mdl_var get_drn;

  switch (type_) {
  case MDL_T_INT:
    if (name_.length()) {
      // variable in the expression.
      mdl_var get_int;
      int value;
      assert(mdl_env::get(get_int, name_));
      if (!get_int.get(value)) {
	  fprintf(stderr, "Unable to get value for %s\n", name_.string_of());
	  fflush(stderr);
	  return false;
      } else {
	  ast = AstNode(Constant, (void*) value);
      }
    } else {
      ast = AstNode(Constant, (void*) val_);
    }
    break;
  case MDL_ARG:
    ast = AstNode(Param, (void*) val_);
    break;
  case MDL_RETURN:
    break;
  case MDL_READ_SYMBOL:
    // TODO -- I am relying on global_proc to be set in mdl_metric::apply
    if (global_proc) {
      Symbol info;
      if (global_proc->symbols->symbol_info(name_, info)) {
	Address adr = info.addr();
	ast = AstNode(DataAddr, (void*) adr);
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
    ast = AstNode(DataAddr, (void*) val_);
    break;
  case MDL_CALL_FUNC: {
    // don't confuse 'args' with 'args_' here!
    vector<AstNode> args;
    AstNode arg;
    for (unsigned u = 0; u < args_.size(); u++) {
      if (!args_[u]->apply(arg))
	return false;

      args += arg;
    }
    pdf = global_proc->symbols->findOneFunction(string(name_));
    if (!pdf) {
	string msg = string("In metric '") + currentMetric + string("': ") +
	  string("unable to find procedure '") + name_ + string("'");
	showErrorCallback(95, msg);
	return false;
    }
    ast = AstNode(name_, args);
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
      assert(get_drn.get(drn));
      ast = AstNode(DataPtr, drn);      
    }
    break;
  case MDL_T_COUNTER:
    {
      mdl_var get_drn;
      dataReqNode *drn;
      assert(mdl_env::get(get_drn, name_));
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
		  ast = AstNode(Constant, (void*) value);
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
		  ast = AstNode(DataValue, (void*) drn);
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

bool T_dyninstRPC::mdl_instr_req::apply(AstNode &mn, const AstNode *pred,
                                        bool mn_initialized) {
  // a return value of true implies that "mn" was written to
  AstNode ast_arg;

  vector<AstNode> ast_args;
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
      assert(mdl_env::get(get_drn, timer_counter_name_));
      assert(get_drn.get(drn));
  }

  AstNode code;
  pdFunction *pdf;

  switch (type_) {
  case MDL_SET_COUNTER:
    code = createCall("setCounter", drn, ast_arg);
    break;
  case MDL_ADD_COUNTER:
    code = createCall("addCounter", drn, ast_arg);
    break;
  case MDL_SUB_COUNTER:
    code = createCall("subCounter", drn, ast_arg);
    break;
  case MDL_START_WALL_TIMER:
  case MDL_STOP_WALL_TIMER:
  case MDL_START_PROC_TIMER:
  case MDL_STOP_PROC_TIMER:
    ast_args += AstNode(DataValue, (void *) drn);
    code = AstNode(timer_fun, ast_args);
    break;
  case MDL_CALL_FUNC:
    if (! rand_->apply(code))
      return false;
    break;
  default:
    return false;
  }

  if (pred)
    code = createIf(*pred, code);

  if (mn_initialized)
    mn = AstNode(mn, code);
  else
    mn = code;

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
  vector<pdFunction*> *vp;
  list_var.get(vp);
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

static AstNode do_rel_op(opCode op, T_dyninstRPC::mdl_instr_rand *if_op2,
                         const AstNode &ast_left) {
   // NOTE: ast_left _must_ be defined
   AstNode ast_right;
   if (!if_op2->apply(ast_right))
      return AstNode(); // ???

   return AstNode(op, ast_left, ast_right);
}

bool T_dyninstRPC::mdl_icode::apply(AstNode &mn, bool mn_initialized) {
  // a return value of true implies that "mn" has been written to
  // TODO -- handle the if case here
  // TODO -- call req_->apply() after building if

  if (!req_)
     return false;

  AstNode pred;
  AstNode *pred_ptr;

  if (use_if_) {
    AstNode ast1;
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
      pred = ast1;
      break;
    default: return false;
    }

    pred_ptr = new AstNode(pred);
  } // if ()
  else
    pred_ptr = NULL;

  bool result = req_->apply(mn, pred_ptr, mn_initialized);
     // note: a result of true implies that "mn" was written to
     // Hence, a result of true from this routine means the same.

//  if (pred_ptr)
//     delete pred_ptr;

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
	  pdFunction *pdf = global_proc->symbols->findOneFunction(func_name);
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
	  module *mod = global_proc->symbols->findModule(mod_name);
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
  mdl_var res(false); int iv;
  if (!expr_->apply(res))
    return false;
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
  bool ret = body_->apply(mn, flags);
  return ret;
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
  if (!stmts_)
    return true;
  unsigned size = stmts_->size();
  for (unsigned index=0; index<size; index++)
    if (!(*stmts_)[index]->apply(mn, flags))
      return false;
  return true;
}

T_dyninstRPC::mdl_list_stmt::mdl_list_stmt(u_int type, string ident,
					   vector<string> *elems,
					   bool is_lib, vector<string>* flavor) 
: type_(type), id_(ident), elements_(elems), is_lib_(is_lib), flavor_(flavor) { }
T_dyninstRPC::mdl_list_stmt::mdl_list_stmt() { }
T_dyninstRPC::mdl_list_stmt::~mdl_list_stmt() { delete elements_; }

bool T_dyninstRPC::mdl_list_stmt::apply(metricDefinitionNode *mn,
					vector<dataReqNode*>& flags) {
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

  // make call to symtab code to watch for these procedures/modules
  if (type_ == MDL_T_PROCEDURE) {
    vector<pdFunction*> *pdict;
    assert(list_var.get(pdict)); assert(pdict);
    image::watch_functions(id_, elements_, is_lib_, pdict);
  } else if (type_ == MDL_T_MODULE) {
    // vector<module*> *mdict;
    // assert(list_var.get(mdict)); assert(mdict);
    // image::watch_modules(id_, elements_, is_lib_, mdict);
    // TODO
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
 
   vector<instPoint *> *points;
   vector<dataReqNode*> flags;
   if (constrained_) {
       // we are constrained so use the flags (boolean constraint variables
       flags = inFlags;
   }
  mdl_var temp(false);
  if (!icode_reqs_)
    return false;
  if (!point_expr_->apply(temp))
    return false;
  if (temp.type() == MDL_T_LIST_POINT) {
    if (!temp.get(points)) return false;
  } else if (temp.type() == MDL_T_POINT) {
    instPoint *p;
    if (!temp.get(p)) return false;
    points = new vector<instPoint *>;
    *points += p;
  } else {
    return false;
  }
  unsigned size = icode_reqs_->size();

  AstNode code;
  for (unsigned u=0; u<size; u++)
    if (!(*icode_reqs_)[u]->apply(code, u>0)) // when u is 0, code is un-initialized
      return false;

  enum callWhen cwhen; enum callOrder corder;
  switch (position_) {
      case MDL_PREPEND: 
	  corder = orderFirstAtPoint; 
	  break;
      case MDL_APPEND: 
	  corder = orderLastAtPoint; 
	  break;
      default: assert(0);
  }
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
  for (int i = 0; i < points->size(); i++) {
      instPoint *p = (*points)[i];
      AstNode code;

      for (unsigned u=0; u<size; u++)
	if (!(*icode_reqs_)[u]->apply(code, u > 0)) // code is initialized when u > 0
	  return false;

      // Instantiate all constraints (flags) here
      unsigned fsize = flags.size();
      for (int fi=fsize-1; fi>=0; fi--) {
        AstNode temp(DataValue, flags[fi]);
        code = createIf(temp, code);
      }

      mn->addInst(p, code, cwhen, corder);
  }

  return true;
}

bool mdl_can_do(string& met_name) {
  unsigned size = mdl_data::all_metrics.size();
  for (unsigned u=0; u<size; u++) 
    if (mdl_data::all_metrics[u]->name_ == met_name)
      return true;

  return false;
}

metricDefinitionNode *mdl_do(vector< vector<string> >& canon_focus, string& met_name,
			     string& flat_name, vector<process *> procs) {
  currentMetric = met_name;
  unsigned size = mdl_data::all_metrics.size();
  for (unsigned u=0; u<size; u++) 
    if (mdl_data::all_metrics[u]->name_ == met_name) {
      return (mdl_data::all_metrics[u]->apply(canon_focus, flat_name, procs));
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
  kid.name = "MsgTag"; kid.type = MDL_T_INT; kid.end_allowed = true; kids += kid;
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

  struct utsname unm;
  if (P_uname(&unm) == -1) assert(0);
  string vname = "$machine";
  mdl_env::add(vname, false, MDL_T_STRING);
  string nodename = unm.nodename;
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

  return true;
}

void dynRPC::send_metrics(vector<T_dyninstRPC::mdl_metric*>* var_0) {
  mdl_met = true;

//
// Old definition of observed_cost
//
// This code is not necessary if we define observed_cost as an internal
// metric -- naim
//
//  static bool been_here = false;
//  if (!been_here) {
//    been_here = true;
//      // Kludge -- declare observed cost
//      // I don't want to attempt to define this using the current mdl
//    string obs_cost = "observed_cost";
//    assert(mdl_data::new_metric("observedCost", obs_cost, "CPUs",
//				aggMax, EventCounter, 0,
// 			        NULL, NULL, NULL, NULL, true, true));
//  }

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
	int i1; if (!left_val.get(i1)) return false; v1 = i1;
      } else {
	if (!left_val.get(v1)) return false;
      }
      if (right_val.type() == MDL_T_INT) {
	int i1; if (!right_val.get(i1)) return false; v2 = i1;
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
	int i1; if (!left_val.get(i1)) return false; v1 = i1;
      } else {
	if (!left_val.get(v1)) return false;
      }
      if (right_val.type() == MDL_T_INT) {
	int i1; if (!right_val.get(i1)) return false; v2 = i1;
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

  if (!mdl_env::get(ret, var_name)) return false;
  
  unsigned index=0;
  unsigned max = types.size();

  while (index < max) {
    unsigned current_type = types[index++];
    unsigned next_field = types[index++];

    switch (current_type) {
    case MDL_T_PROCEDURE:
      pdFunction *pdf;
      if (!ret.get(pdf)) return false;
      switch (next_field) {
      case 0: { string prettyName = pdf->prettyName();
	        if (!ret.set(prettyName)) return false;
	      } break;
      case 1: if (!ret.set(&pdf->calls)) return false; break;
      case 2: if (!ret.set(pdf->funcEntry())) return false; break;
      case 3: if (!ret.set(&pdf->funcReturns)) return false; break;
      case 4: if (!ret.set((int)pdf->tag())) return false; break;
      default: assert(0); break;
      }
      break;
    case MDL_T_MODULE:
      module *mod;
      if (!ret.get(mod)) return false;
      switch (next_field) {
      case 0: { string fileName = mod->fileName();
		if (!ret.set(fileName)) return false; 
	      } break;
      case 1: if (!ret.set(&mod->funcs)) return false; break;
      default: assert(0); break;	       
      }
      break;
    default:
      assert(0); return false;
    }
  }
  return true;
}

//// Old definition of observed cost, this is no longer being used
//#ifdef notdef
//metricDefinitionNode *mdl_observed_cost(vector< vector<string> >& canon_focus,
//					string& met_name,
//					string& flat_name, vector<process *> procs) {
//  pdFunction *sampler;
//  dataReqNode *dataPtr;
//  string name("observed_cost");
//  static string machine;
//  static bool machine_init= false;
//  if (!machine_init) {
//    machine_init = true;
//    struct utsname un; assert(!P_uname(&un) != -1); machine = un.nodename;
//  }
//
//  if (other_machine_specified(canon_focus, machine)) return NULL;
//  vector<process*> ip;
//  add_processes(canon_focus, procs, ip);
//  unsigned ip_size, index;
//  if (!(ip_size = ip.size())) return NULL;
//
//  // Can't refine procedure or sync object
//  if (canon_focus[resource::sync_object].size() > 1) return NULL;
//  if (canon_focus[resource::procedure].size() > 1) return NULL;
//
//  vector<metricDefinitionNode*> parts;
//
//  for (index=0; index<ip_size; index++) {
//    process *proc = ip[index];
//    metricDefinitionNode *mn =
//      new metricDefinitionNode(proc, name, canon_focus, flat_name, aggMax);
//    assert(mn);
//    dataPtr = mn->addIntCounter(0, false);
//    assert(dataPtr);
//
//    sampler = ((mn->proc())->symbols)->findOneFunction("DYNINSTsampleValues");
//    assert(sampler);
//
//    AstNode reportNode ("DYNINSTreportCost", 
//			AstNode(DataPtr, dataPtr), AstNode(Constant, 0));
//
//    mn->addInst(sampler->funcEntry(), reportNode, callPreInsn, orderLastAtPoint);
//
//    if (mn && mn->nonNull()) 
//      parts += mn;
//    else {
//      delete mn; mn = NULL;
//    }
//  }
//
//  metricDefinitionNode *ret = NULL;
//  //  switch (parts.size()) {
//  //  case 0: break;
//  //  case 1: ret = parts[0]; break;
//  //  default: ret = new metricDefinitionNode(name, canon_focus, flat_name, parts);
//  //  }
//  if (parts.size())
//    ret = new metricDefinitionNode(name, canon_focus, flat_name, parts);
//
//  if (ret) ret->set_inform(true);
//  return ret;
//}
//#endif

bool mdl_get_initial(string flavor, pdRPC *connection) {
  mdl_init(flavor);
  while (!(mdl_met && mdl_cons && mdl_stmt)) {
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

bool mdl_metric_data(string& met_name, mdl_inst_data& md) {
  unsigned size = mdl_data::all_metrics.size();
  for (unsigned u=0; u<size; u++)
    if (mdl_data::all_metrics[u]->name_ == met_name) {
      md.aggregate = mdl_data::all_metrics[u]->agg_op_;
      md.style = (enum metricStyle) mdl_data::all_metrics[u]->style_;
      return true;
    }
  return false;
}
