


/* 
 * $Log: mdl.C,v $
 * Revision 1.9  1995/11/17 17:24:26  newhall
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
static AstNode *do_instr_rand(u_int arg_type, u_int arg_val, string& arg_name, string& a2);

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
				    unsigned fold,
				    unsigned agg, unsigned sty, u_int type,
				    vector<T_dyninstRPC::mdl_stmt*> *mv, 
				    vector<string> *flav,
				    vector<T_dyninstRPC::mdl_constraint*> *cons,
				    vector<string> *temp_counters,
				    bool developerMode,
				    bool normalized)
: id_(id), name_(name), units_(units), fold_op_(fold), agg_op_(agg), 
  style_(sty), developerMode_(developerMode), normalized_(normalized),
  type_(type), stmts_(mv), flavors_(flav), constraints_(cons), temp_ctr_(temp_counters) { }
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
			  unsigned fold, unsigned agg, unsigned sty, u_int type,
			  vector<T_dyninstRPC::mdl_stmt*> *mv,
			  vector<string> *flav,
			  vector<T_dyninstRPC::mdl_constraint*> *cons,
			  vector<string> *temp_counters,
			  bool developerMode,
			  bool normalized) {
  T_dyninstRPC::mdl_metric *m = new T_dyninstRPC::mdl_metric(id, name, 
							     units, fold, agg,
							     sty, type, mv,
							     flav, cons,
							     temp_counters,
							     developerMode,
							     normalized);
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
				 vector<process*> &ip) {
  assert(focus[resource::process][0] == "Process");
  unsigned pi, ps;

  switch(focus[resource::process].size()) {
  case 1:
#ifdef sparc_tmc_cmost7_3
    assert(nodePseudoProcess);
    ip += nodePseudoProcess;
#else
    ip = processVec;
#endif    
    break;
  case 2:
    ps = processVec.size();
    for (pi=0; pi<ps; pi++) 
      if (processVec[pi]->rid->part_name() == focus[resource::process][1]) {
	ip += processVec[pi];
	break;
      }
    break;
  default:
    assert(0);
  }
}

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
	  if (mc = flag_matches(focus[fi], (*cons)[ci], is_default)) {
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
      if (!matched && (el_size>1)) return false;
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
  pdf = proc->symbols->findOneFunction(string("main"));
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

static inline bool apply_to_process_list(vector<process*>& instProcess,
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
						      string& flat_name) {

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
  add_processes(focus, instProcess);
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
  switch (parts.size()) {
  case 0: break;
  case 1: ret = parts[0]; break;
  default: ret = new metricDefinitionNode(name_, focus, flat_name, parts);
  }

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
static inline bool do_trailing_resource(unsigned& type, unsigned& hierarchy,
					vector<string>& resource, process *proc) {
  string c_string = "$constraint";
  switch (hierarchy) {
  case MDL_RES_CODE:
    if (type == MDL_T_PROCEDURE) {
      if (resource.size() != 3) return false;
      pdFunction *pdf = proc->symbols->findOneFunction(resource[2]);
      if (!pdf) return false;
      mdl_env::add(c_string, false, MDL_T_PROCEDURE);
      mdl_env::set(pdf, c_string);
    } else if (type == MDL_T_MODULE) {
      if (resource.size() != 2) return false;
      module *mod = proc->symbols->findModule(resource[1]);
      if (!mod) return false;
      mdl_env::add(c_string, false, MDL_T_MODULE);
      mdl_env::set(mod, c_string);
    } else {
      assert(0);
    }
    break;
  case MDL_RES_SYNCOBJECT:
    {
    if (resource.size() != 3) return false;
    if (type != MDL_T_INT) assert(0);
    mdl_env::add(c_string, false, MDL_T_INT);
    int tag = atoi(resource[2].string_of());
    if (tag < 0) return false;
    mdl_env::set(tag, c_string);
    break;
    }
  case MDL_RES_PROCESS:
  case MDL_RES_MACHINE:
  default:
    assert(0);
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
  if (!do_trailing_resource(type_, hierarchy_, resource, proc)) {
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

T_dyninstRPC::mdl_instr_req::mdl_instr_req(u_int a_type, u_int a_val, string a_name,
					   string arg2, u_int type, string obj_name)
: arg_type_(a_type), arg_val_(a_val), arg_name_(a_name), arg_name_2_(arg2),
  type_(type), timer_counter_name_(obj_name) { }

T_dyninstRPC::mdl_instr_req::mdl_instr_req(u_int type, string obj_name)
: type_(type), timer_counter_name_(obj_name) { }

T_dyninstRPC::mdl_instr_req::mdl_instr_req() : type_(0) { }
T_dyninstRPC::mdl_instr_req::~mdl_instr_req() { }

bool T_dyninstRPC::mdl_instr_req::apply(unsigned where, instPoint *p, unsigned where_instr,
					metricDefinitionNode *mn, AstNode *pred,
					vector<dataReqNode*>& flags) {
  enum callWhen cwhen; enum callOrder corder;
  switch (where) {
  case MDL_PREPEND: corder = orderFirstAtPoint; break;
  case MDL_APPEND: corder = orderLastAtPoint; break;
  default: assert(0);
  }
  switch (where_instr) {
  case MDL_PRE_INSN: cwhen = callPreInsn; break;
  case MDL_POST_INSN: cwhen = callPostInsn; break;
  default: assert(0);
  }

  AstNode *ast_arg = NULL;
  // TODO -- args to calls
  switch (type_) {
  case MDL_SET_COUNTER:
  case MDL_ADD_COUNTER:
  case MDL_SUB_COUNTER:
  case MDL_CALL_FUNC:
  case MDL_CALL_FUNC_COUNTER:
    if (!(ast_arg = do_instr_rand(arg_type_, arg_val_, arg_name_, arg_name_2_)))
      return false;
    break;
  }

  dataReqNode *drn;
  mdl_var get_drn;
  if (type_ != MDL_CALL_FUNC) {
      assert(mdl_env::get(get_drn, timer_counter_name_));
      assert(get_drn.get(drn));
  }

  AstNode *code = NULL;
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
    code = createPrimitiveCall("DYNINSTstartWallTimer", drn, 0);
    break;
  case MDL_STOP_WALL_TIMER:
    code = createPrimitiveCall("DYNINSTstopWallTimer", drn, 0);
    break;
  case MDL_START_PROC_TIMER:
    // TODO -- are these ok?
    code = createPrimitiveCall("DYNINSTstartProcessTimer", drn, 0);
    break;
  case MDL_STOP_PROC_TIMER:
    code = createPrimitiveCall("DYNINSTstopProcessTimer", drn, 0);
    break;
  case MDL_CALL_FUNC:
    pdf = global_proc->symbols->findOneFunction(string(timer_counter_name_));
    if (!pdf) {
	fprintf(stderr, "Unable to locate procedure %s for metric definition\n",
		timer_counter_name_.string_of());
	fflush(stderr);
	return false;
    }
    code = new AstNode(timer_counter_name_, ast_arg, NULL);
    break;
  case MDL_CALL_FUNC_COUNTER:
    pdf = global_proc->symbols->findOneFunction(string(arg_name_));
    if (!pdf) return false;
    code = createPrimitiveCall(arg_name_, drn, 0);
    break;
  default:
    return false;
  }

  if (pred) code = createIf(pred, code);

  // Instantiate all constraints (flags) here
  unsigned fsize = flags.size();
  for (int fi=fsize-1; fi>=0; fi--) {
    AstNode *temp = new AstNode(DataValue, flags[fi]);
    code = createIf(temp, code);
  }

  mn->addInst(p, code, cwhen, corder);
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
T_dyninstRPC::mdl_icode::mdl_icode(u_int iop1, u_int ival1, string str1,
				   u_int iop2, u_int ival2, string str2,
				   u_int bin_op, bool use_if,
				   T_dyninstRPC::mdl_instr_req *ireq)
: if_op1_(iop1), if_val1_(ival1), if_str1_(str1),
  if_op2_(iop2), if_val2_(ival2), if_str2_(str2),
  bin_op_(bin_op), use_if_(use_if), req_(ireq) { }
T_dyninstRPC::mdl_icode::~mdl_icode() { delete req_; }

static inline AstNode *do_rel_op(opCode op, u_int& if_op, u_int& if_val,
				 string& if_str, AstNode *ast_left) {
  AstNode *ast_right, *rel_ast=NULL;
  assert(ast_left);
  string empty;
  if (!(ast_right = do_instr_rand(if_op, if_val, if_str, empty))) {
    delete ast_left; return NULL;
  }
  rel_ast = new AstNode(op, ast_left, ast_right);
  assert(rel_ast);
  return rel_ast;
}

bool T_dyninstRPC::mdl_icode::apply(u_int position, instPoint *p, u_int where_instr,
				    metricDefinitionNode *mn,
				    vector<dataReqNode*>& flags) {
  // TODO -- handle the if case here
  // TODO -- call req_->apply() after building if
  AstNode *ast1, *pred = NULL;
  if (!req_) return false;
  if (use_if_) {
    string empty;
    if (!(ast1 = do_instr_rand(if_op1_, if_val1_, if_str1_, empty))) {
      delete ast1; return false;
    }
    switch (bin_op_) {
    case MDL_LT:
      if (!(pred = do_rel_op(lessOp, if_op2_, if_val2_, if_str2_, ast1)))
	return false;
      break;
    case MDL_GT:
      if (!(pred = do_rel_op(greaterOp, if_op2_, if_val2_, if_str2_, ast1)))
	return false;
      break;
    case MDL_LE:
      if (!(pred = do_rel_op(leOp, if_op2_, if_val2_, if_str2_, ast1)))
	return false;
      break;
    case MDL_GE:
      if (!(pred = do_rel_op(geOp, if_op2_, if_val2_, if_str2_, ast1)))
	return false;
      break;
    case MDL_EQ:
      if (!(pred = do_rel_op(eqOp, if_op2_, if_val2_, if_str2_, ast1)))
	return false;
      break;
    case MDL_NE:
      if (!(pred = do_rel_op(neOp, if_op2_, if_val2_, if_str2_, ast1)))
	return false;
      break;
    case MDL_T_NONE: pred = ast1; break;
    default: return false;
    }
  }
  return (req_->apply(position, p, where_instr, mn, pred, flags));
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
					   bool is_lib, string flavor) 
: type_(type), id_(ident), elements_(elems), is_lib_(is_lib), flavor_(flavor) { }
T_dyninstRPC::mdl_list_stmt::mdl_list_stmt() { }
T_dyninstRPC::mdl_list_stmt::~mdl_list_stmt() { delete elements_; }

bool T_dyninstRPC::mdl_list_stmt::apply(metricDefinitionNode *mn,
					vector<dataReqNode*>& flags) {
  if (flavor_ != daemon_flavor) return false;
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
					 vector<dataReqNode*>& flags) {
  mdl_var temp(false);
  if (!icode_reqs_)
    return false;
  if (!point_expr_->apply(temp))
    return false;
  if (temp.type() != MDL_T_POINT)
    return false;
  instPoint *p;
  if (!temp.get(p))
    return false;
  unsigned size = icode_reqs_->size();

  for (unsigned u=0; u<size; u++)
    if (!(*icode_reqs_)[u]->apply(position_, p, where_instr_, mn, flags))
      return false;
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
			     string& flat_name) {
  unsigned size = mdl_data::all_metrics.size();
  for (unsigned u=0; u<size; u++) 
    if (mdl_data::all_metrics[u]->name_ == met_name) {
      return (mdl_data::all_metrics[u]->apply(canon_focus, flat_name));
    }
  return NULL;
}

bool mdl_init(string& flavor) { 

  daemon_flavor = flavor;
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

  mdl_env::push();
  // These are pushed on per-process
  // mdl_env::add("$procedures", false, MDL_T_LIST_PROCEDURE);
  // mdl_env::add("$modules", false, MDL_T_LIST_MODULE);

  struct utsname unm;
  if (P_uname(&unm) == -1) assert(0);
  string vname = "$machine";
  mdl_env::add(vname, false, MDL_T_STRING);
  mdl_env::set(string(unm.nodename), vname);

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

  static bool been_here = false;
  if (!been_here) {
    been_here = true;
    // Kludge -- declare observed cost
    // I don't want to attempt to define this using the current mdl
    string obs_cost = "observed_cost";
    assert(mdl_data::new_metric("observedCost", obs_cost, "# CPUS",
				aggMax, aggMax, EventCounter, 0,
 			        NULL, NULL, NULL, NULL, true, true));
  }

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
      case 0: if (!ret.set(pdf->prettyName())) return false; break;
      case 1: if (!ret.set(&pdf->calls)) return false; break;
      case 2: if (!ret.set(pdf->funcEntry())) return false; break;
      case 3: if (!ret.set(pdf->funcReturn())) return false; break;
      case 4: if (!ret.set((int)pdf->tag())) return false; break;
      default: assert(0); break;
      }
      break;
    case MDL_T_MODULE:
      module *mod;
      if (!ret.get(mod)) return false;
      switch (next_field) {
      case 0: if (!ret.set(mod->fileName())) return false; break;
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

AstNode *do_instr_rand(u_int arg_type, u_int arg_val, string& arg_name, string& arg2) {
  AstNode *ret=NULL;
  pdFunction *pdf;
  mdl_var get_drn;

  switch (arg_type) {
  case MDL_T_INT:
    if (arg_name.prefixed_by("$")) {
      // variable in the expression.
      mdl_var get_int;
      int value;
      assert(mdl_env::get(get_int, arg_name));
      if (!get_int.get(value)) {
	  fprintf(stderr, "Unable to get value for %s\n", arg_name.string_of());
	  fflush(stderr);
	  return NULL;
      } else {
	  ret = new AstNode(Constant, (void*) value);
      }
    } else {
      ret = new AstNode(Constant, (void*) arg_val);
    }
    break;
  case MDL_ARG:
    ret = new AstNode(Param, (void*) arg_val);
    break;
  case MDL_RETURN:
    break;
  case MDL_READ_SYMBOL:
    // TODO -- I am relying on global_proc to be set in mdl_metric::apply
    if (global_proc) {
      Symbol info;
      if (global_proc->symbols->symbol_info(arg_name, info)) {
	Address adr = info.addr();
	ret = new AstNode(DataAddr, (void*) adr);
      }
    }
    break;
  case MDL_READ_ADDRESS:
    // TODO -- check on the range of this address!
    ret = new AstNode(DataAddr, (void*) arg_val);
    break;
  case MDL_CALL_FUNC:
    // call a function with either and integer or a counter argument
    //    XXXX This code should generalized function call.  - jkh 7/31/95.
    // 	       Also should handle multiple arguments.
    //
    pdf = global_proc->symbols->findOneFunction(string(arg_name));
    if (!pdf) {
	fprintf(stderr, "Unable to locate procedure %s for metric definition\n",
		arg_name.string_of());
	fflush(stderr);
	return NULL;
    }
    // Hack to tell counter from timer.
    if (arg2 == "s") {
        ret = new AstNode(arg_name,
			  new AstNode(Constant, (void*) arg_val),
			  new AstNode(Constant, (void*) 0));
    } else {
        mdl_var get_drn;
        dataReqNode *drn;

	if (!mdl_env::get(get_drn, arg2)) {
	    fprintf(stderr,"Unable to find variable %s for metric definition\n",
		arg2.string_of());
	    return NULL;
	}
	assert(get_drn.get(drn));
	ret = createPrimitiveCall(arg_name, drn, 0);
    }
    break;
  case MDL_T_COUNTER:
    {
      mdl_var get_drn;
      dataReqNode *drn;
      assert(mdl_env::get(get_drn, arg_name));
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
		      arg_name.string_of());
		  fflush(stderr);
		  return NULL;
	      } else {
		  ret = new AstNode(Constant, (void*) value);
	      }
	      break;
	  case MDL_T_COUNTER:	// is MDL_T_COUNTER used here ??? jkh 7/31/95
	  case MDL_T_DRN:
	      if (!get_drn.get(drn)) {
		  fprintf(stderr, "Unable to find variable %s\n", 
		      arg_name.string_of());
		  fflush(stderr);
		  return NULL;
	      } else {
		  ret = new AstNode(DataValue, (void*) drn);
	      }
	      break;
	  default:
	      fprintf(stderr, "type of variable %s is not known\n",
		  arg_name.string_of());
	      fflush(stderr);
	      return NULL;
      }
    }
    break;
  default:
    break;
  }
  return ret;
}

metricDefinitionNode *mdl_observed_cost(vector< vector<string> >& canon_focus,
					string& met_name,
					string& flat_name) {
  pdFunction *sampler;
  AstNode *reportNode;
  dataReqNode *dataPtr;
  string name("observed_cost");
  static string machine;
  static bool machine_init= false;
  if (!machine_init) {
    machine_init = true;
    struct utsname un; assert(!P_uname(&un) != -1); machine = un.nodename;
  }

  if (other_machine_specified(canon_focus, machine)) return NULL;
  vector<process*> ip;
  add_processes(canon_focus, ip);
  unsigned ip_size, index;
  if (!(ip_size = ip.size())) return NULL;

  // Can't refine procedure or sync object
  if (canon_focus[resource::sync_object].size() > 1) return NULL;
  if (canon_focus[resource::procedure].size() > 1) return NULL;

  vector<metricDefinitionNode*> parts;

  for (index=0; index<ip_size; index++) {
    process *proc = ip[index];
    metricDefinitionNode *mn =
      new metricDefinitionNode(proc, name, canon_focus, flat_name, aggMax);
    assert(mn);
    dataPtr = mn->addIntCounter(0, false);
    assert(dataPtr);

    sampler = ((mn->proc())->symbols)->findOneFunction("DYNINSTsampleValues");
    assert(sampler);
    reportNode = new AstNode("DYNINSTreportCost", 
			     new AstNode(DataPtr, dataPtr), new AstNode(Constant, 0));
    assert(reportNode);
    mn->addInst(sampler->funcEntry(), reportNode, callPreInsn, orderLastAtPoint);

    if (mn && mn->nonNull()) 
      parts += mn;
    else {
      delete mn; mn = NULL;
    }
  }

  metricDefinitionNode *ret = NULL;
  switch (parts.size()) {
  case 0: break;
  case 1: ret = parts[0]; break;
  default: ret = new metricDefinitionNode(name, canon_focus, flat_name, parts);
  }
  if (ret) ret->set_inform(true);
  return ret;
}

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
    element.normalized = mdl_data::all_metrics[u]->normalized_;
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
