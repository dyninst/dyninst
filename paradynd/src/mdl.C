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

// $Id: mdl.C,v 1.138 2003/05/19 03:02:58 schendel Exp $

#include <iostream.h>
#include <stdio.h>
#include "dyninstRPC.xdr.SRVR.h"
#include "paradyn/src/met/mdl_data.h"
#include "paradynd/src/machineMetFocusNode.h"
#include "paradynd/src/processMetFocusNode.h"
#include "paradynd/src/threadMetFocusNode.h"
#include "paradynd/src/instrCodeNode.h"
#include "paradynd/src/instrDataNode.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/ast.h"
#include "paradynd/src/main.h"
#include "dyninstAPI/src/symtab.h"
#include "common/h/Timer.h"
#include "paradynd/src/mdld.h"
#include "dyninstAPI/src/showerror.h"
#include "paradynd/src/pd_process.h"
#include "paradynd/src/pd_thread.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "common/h/debugOstream.h"
#include "pdutil/h/pdDebugOstream.h"
#include "dyninstAPI/src/instPoint.h" // new...for class instPoint
#include "paradynd/src/focus.h"

#include "paradynd/src/papiMgr.h"

#include "paradynd/src/init.h"

#include <ctype.h>

// The following vrbles were defined in process.C:

extern unsigned enable_pd_attach_detach_debug;

#if ENABLE_DEBUG_CERR == 1
#define attach_cerr if (enable_pd_attach_detach_debug) cerr
#else
#define attach_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern unsigned enable_pd_inferior_rpc_debug;

#if ENABLE_DEBUG_CERR == 1
#define inferiorrpc_cerr if (enable_pd_inferior_rpc_debug) cerr
#else
#define inferiorrpc_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern unsigned enable_pd_shm_sampling_debug;

#if ENABLE_DEBUG_CERR == 1
#define shmsample_cerr if (enable_pd_shm_sampling_debug) cerr
#else
#define shmsample_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern unsigned enable_pd_fork_exec_debug;

#if ENABLE_DEBUG_CERR == 1
#define forkexec_cerr if (enable_pd_fork_exec_debug) cerr
#else
#define forkexec_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern unsigned enable_pd_metric_debug;

#if ENABLE_DEBUG_CERR == 1
#define metric_cerr if (enable_pd_metric_debug) cerr
#else
#define metric_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

// Some global variables
static string currentMetric;  // name of the metric that is being processed.
static string daemon_flavor;
pd_process *global_proc = NULL;
static bool mdl_met=false, mdl_cons=false, mdl_stmt=false, mdl_libs=false;

inline unsigned ui_hash(const unsigned &u) { return u; }

// static members of mdl_env
pdvector<unsigned> mdl_env::frames;
pdvector<mdl_var> mdl_env::all_vars;

// static members of mdl_data
dictionary_hash<unsigned, pdvector<mdl_type_desc> > mdl_data::fields(ui_hash);
pdvector<mdl_focus_element> mdl_data::foci;
pdvector<T_dyninstRPC::mdl_stmt*> mdl_data::stmts;
pdvector<T_dyninstRPC::mdl_constraint*> mdl_data::all_constraints;
pdvector<string> mdl_data::lib_constraints;
pdvector<T_dyninstRPC::mdl_metric*> mdl_data::all_metrics;

/*
#if defined(MT_THREAD)
int index_in_data(unsigned lev, unsigned ind, pdvector<dataReqNode*>& data) {
  int size = data.size();
  
  for (int i=0; i<size; i++) {
    if ((lev == data[i]->getLevel())
	&& (ind == data[i]->getVarIndex()))
      return i;
  }
  
  // not matched: return < 0
  return -1;
}
#else
int index_in_data(Address v, pdvector<dataReqNode*>& data) {
  int size = data.size();
  
  for (int i=0; i<size; i++) {
     if (v == data[i]->getInferiorPtr())
	return i;
  }
  
  // not matched: return < 0
  return -1;
}
#endif
*/


//
// walk_deref() is used to process the fields of the MDL_EXPR_DOT
// expression.  the MDL_EXPR_DOT expression is used to be called
// MDL_EXPR_DEREF, which is inaccurate at best. --chun
//
static bool walk_deref(mdl_var& ret, pdvector<unsigned>& types);

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
      pdvector<function_base *> fbv;
      
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
        case MDL_T_PROCEDURE_NAME: {
           // lookup-up the functions defined in resource lists
           // the function may not exist in the image, in which case we get the
           // next one
           pdf = NULL;
           do 
           {
              functionName *fn = (*funcName_list)[index++];
              if (!global_proc->findAllFuncsByName(fn->get(), fbv)) {
                 //string msg = string("unable to find procedure '") + 
                 //             fn->get() + string("'");
                 //showErrorCallback(95, msg);
                 continue;
              }
              else if (fbv.size() > 1) {
#ifdef FIXME_AFTER_4
                 string msg = string("WARNING:  found ") + string(fbv.size()) +
                              string(" records of function '") + fn->get() +
                              string("'") + string(".  Using the first.(1)");
                 //showErrorCallback(95, msg);
                 cerr << msg << endl;
#endif
              }
              pdf = fbv[0];
           } while (pdf == NULL && index < max_index);
           
           if (pdf == NULL) 
              return false;
           
           pdvector<function_base *> *func_buf = new pdvector<function_base*>;
           (*func_buf).push_back(pdf);
           return (mdl_env::set(func_buf, index_name));
        }
        case MDL_T_PROCEDURE: {
           pdf = (*func_list)[index++];
           pdvector<function_base *> *func_buf = new pdvector<function_base*>;
           (*func_buf).push_back(pdf);
           assert(pdf);
           return (mdl_env::set(func_buf, index_name));
        }
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
  pdvector<int> *int_list;
  pdvector<float> *float_list;
  pdvector<string> *string_list;
  pdvector<bool> *bool_list;
  pdvector<function_base*> *func_list;
  pdvector<functionName*> *funcName_list;
  pdvector<module*> *mod_list;
  pdvector<instPoint*> *point_list;
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
		       u_int agg, u_int sty, u_int type, string hwcntr,
		       pdvector<T_dyninstRPC::mdl_stmt*> *mv, 
		       pdvector<string> *flav,
		       pdvector<T_dyninstRPC::mdl_constraint*> *cons,
		       pdvector<string> *temp_counters,
		       bool developerMode,
		       int unitstype)
: id_(id), name_(name), units_(units), agg_op_(agg), style_(sty),
  type_(type), hwcntr_(hwcntr), stmts_(mv), flavors_(flav), constraints_(cons),
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
			  u_int agg, u_int sty, u_int type, string hwcntr,
			  pdvector<T_dyninstRPC::mdl_stmt*> *mv,
			  pdvector<string> *flav,
			  pdvector<T_dyninstRPC::mdl_constraint*> *cons,
			  pdvector<string> *temp_counters,
			  bool developerMode,
			  int unitstype) 
{
  assert (0);
  T_dyninstRPC::mdl_metric *m = new T_dyninstRPC::mdl_metric(id, name, units, agg, sty, 
                  type, hwcntr, mv, flav, cons, temp_counters, developerMode, unitstype);
  if (!m)
    return false;
  else {
    all_metrics += m;
    return true;
  }
}

// will filter out processes from fromProcs that match the given focus
// and places the result in filteredOutProcs
static void filter_processes(const Focus &focus, pdvector<pd_process*> fromProcs,
			     pdvector<pd_process*> *filteredOutProcs) {
  // assumes that the machine we're on (and thus the daemon we're using)
  // matches the focus
   if(focus.process_defined()) {
      for (unsigned i=0; i<fromProcs.size(); i++) {
	 if (fromProcs[i]->get_rid()->part_name() == focus.get_process()) {
	    (*filteredOutProcs).push_back(fromProcs[i]);
	    break;
	 }    
      }
   } else {
      // if process isn't defined, then thread shouldn't be defined either
      assert(focus.thread_defined() == false);
      // if there's no process specified in the focus, then all of the 
      // processes on this machine (with this daemon) are selected
      (*filteredOutProcs) = fromProcs;
   }
}

// Global constraints are specified by giving their name within a metric def
// Find the real constraint by searching the dictionary using this name
static T_dyninstRPC::mdl_constraint *flag_matches(const Hierarchy* hier, 
						  T_dyninstRPC::mdl_constraint *match_me,
						  bool& is_default) 
{
  unsigned c_size = mdl_data::all_constraints.size();
  for (unsigned cs=0; cs<c_size; cs++) 
    if (mdl_data::all_constraints[cs]->id_ == match_me->id_) {
      match_me = mdl_data::all_constraints[cs];
      if (hier->focus_matches(*(mdl_data::all_constraints[cs]->match_path_))) {
          if (mdl_data::all_constraints[cs]->data_type_ == MDL_T_NONE) {
              is_default = true;
          }
          else
              is_default = false;
          return match_me;
      }
    }
  
  return NULL;
}

// Determine if the global and local constraints are applicable
// Global constraints that are applicable are put on flag_cons
// repl_cons returns the ONE replace constraint that matches

// Goes through pdvector of constraints, cons, and selects the constraints
// that match against the given focus.
// Matched external (flag) constraints are written into vector flag_cons.
// If the metric is defined as a replace constraint, this constraint is
//    written into repl_cons.
// The focus data for constraints in flag_cons is in flags_focus_data and
//     the focus data for a replace constraint is written in repl_cons.
static bool pick_out_matched_constraints(
			     const pdvector<T_dyninstRPC::mdl_constraint*> &cons,
			      const Focus& focus,			      
			      pdvector<T_dyninstRPC::mdl_constraint*> *flag_cons,
			      T_dyninstRPC::mdl_constraint **repl_cons,	      
			      pdvector<const Hierarchy *> *flags_focus_data,
			      const Hierarchy **repl_focus) {
   bool aMatch = false;
   bool aReplConsMatch = false;
   const codeHierarchy *codeHier    = focus.getCodeHierarchy();
   const syncObjHierarchy *syncHier = focus.getSyncObjHierarchy();

   for(unsigned i=0; i<2; i++) {
      const Hierarchy *curHierarchy;
      if(i==0)      curHierarchy = codeHier;
      else if(i==1) curHierarchy = syncHier;

      for(unsigned j=0; j<cons.size(); j++) {
          T_dyninstRPC::mdl_constraint *curCons = cons[j];
          if (! curCons->match_path_) { 
              //like: constraint procedureConstraint;
              T_dyninstRPC::mdl_constraint *mc;
              bool is_default;
              if ((mc = flag_matches(curHierarchy, curCons, is_default))) {
                  // we can have multiple external constraints match the focus
                  aMatch = true;
                  if (!is_default) {
                      (*flag_cons).push_back(mc);
                      (*flags_focus_data).push_back(curHierarchy);
                  }
              }
          } else {
              //like: constraint moduleConstraint /Code is replace processTimer {
              if( curHierarchy->focus_matches(*(curCons->match_path_))) {
                  if(aReplConsMatch == true) {
                      cerr << "error in pcl file, (at least) two replace "
                           << "constraints match the selected focus\n";
                      assert(false);
                  }
                  aMatch = true;   aReplConsMatch = true;
                  *repl_cons = curCons;
                  (*repl_focus) = curHierarchy;
              }
          }
      } 
   }
   if((!codeHier->allCode() || !syncHier->allSync()) && !aMatch) {
      return false;
   }
      
   return true;
}

// put $start in the environment
bool update_environment_start_point(instrCodeNode *codeNode) {
   pdvector<function_base *> *start_func_buf = new pdvector<function_base*>;
   pd_process *proc = codeNode->proc();
   function_base *pdf = NULL;
   pdvector<function_base *> fbv;

   pdvector<string> start_funcs;
#if defined(rs6000_ibm_aix4_1)
   start_funcs.push_back("_pthread_body");
#elif defined(sparc_sun_solaris2_4)
   start_funcs.push_back("_thread_start");
#endif

   if (proc->multithread_ready()) {
       for (unsigned start_iter = 0; 
            start_iter < start_funcs.size();
            start_iter++) {
           if (fbv.size()) fbv.clear();
           proc->findAllFuncsByName(start_funcs[start_iter], fbv);
           for (unsigned i = 0; i < fbv.size(); i++) {
               (*start_func_buf).push_back(fbv[i]);
           }
       }
       if ((*start_func_buf).size() == 0) {
           cerr << "Warning: no internal thread start function found"
                << ", some data may be missed." << endl;
       }       
   }   
   if (NULL != (pdf = proc->getMainFunction())) {
       (*start_func_buf).push_back(pdf);      
   }
   else {
       cerr << __FILE__ << __LINE__ << "getMainFunction() returned NULL!"<<endl;
   }
   if (start_func_buf->size()) {
       string vname = "$start";
       // change this to MDL_T_LIST_PROCEDURE
       mdl_env::add(vname, false, MDL_T_PROCEDURE);
       mdl_env::set(start_func_buf, vname);
   }
   return true;
}

// update the interpreter environment for this processor
// Variable updated: $procedures, $modules, $exit, $start
static bool update_environment(pd_process *proc) {
   // for cases when libc is dynamically linked, the exit symbol is not
   // correct
   string vname = "$exit";
   pdvector<function_base *> *exit_func_buf = new pdvector<function_base*>;
   pdvector<function_base *> fbv;
   function_base *pdf = NULL;
   
   proc->findAllFuncsByName(string(EXIT_NAME), *exit_func_buf); // JAW 04-03
   if (exit_func_buf->size() > 1) {
#ifdef FIXME_AFTER_4
      string msg = string("WARNING:  found ") + string(exit_func_buf->size()) +
                   string(" records of function '") + string(EXIT_NAME) +
                   string("'") + string(".  Using the first.(2)");
      //showErrorCallback(95, msg);
      cerr << msg << endl;      
#endif
      
      // findAllFuncs found more than one function, clear all but one
      pdf = (*exit_func_buf)[0];
      exit_func_buf->clear();
      exit_func_buf->push_back(pdf);
   }
   
   pdf = NULL;
#if !defined(i386_unknown_nt4_0)
   if (!proc->findAllFuncsByName("pthread_exit", fbv)) {
       // Not an error... what about ST programs :)
       //string msg = string("unable to find procedure '") + 
       //string("pthread_exit") + string("'");
       //showErrorCallback(95, msg);
   }
   else {
      if (fbv.size() > 1) {
#ifdef FIXME_AFTER_4
         string msg = string("WARNING:  found ") + string(fbv.size()) +
                      string(" records of function '") +string("pthread_exit")+
                      string("'") + string(".  Using the first.");
         //showErrorCallback(95, msg);
         cerr << msg << endl;;      
#endif
      }
      pdf = fbv[0];
   }
   
   if(pdf)  (*exit_func_buf).push_back(pdf);
   fbv.clear();
   pdf = NULL;
   
   if (!proc->findAllFuncsByName("thr_exit", fbv)) {
#ifdef FIXME_AFTER_4
      string msg = string("unable to find procedure '") + string("thr_exit") +
                   string("'");
      //showErrorCallback(95, msg);
      cerr << msg << endl;;      
#endif
   }
   else {
      if (fbv.size() > 1) {
#ifdef FIXME_AFTER_4
         string msg = string("WARNING:  found ") + string(fbv.size()) +
                      string(" records of function '") + string("thr_exit") +
                      string("'") + string(".  Using the first.");
         //showErrorCallback(95, msg);
         cerr << msg << endl;      
#endif
      }
      pdf = fbv[0];
   }
   
   if(pdf) (*exit_func_buf).push_back(pdf);
#else
    // findAllFuncsByName works on pretty names, but EXIT_NAME is 
    // mangled on Windows
    proc->findAllFuncsByName( "exit", *exit_func_buf);
    if (exit_func_buf->size() > 1) {
      // findAllFuncs found more than one function, clear all but one
      pdf = (*exit_func_buf)[0];
      exit_func_buf->clear();
      exit_func_buf->push_back(pdf);
    }

    proc->findAllFuncsByName( "ExitProcess", *exit_func_buf);
    if (exit_func_buf->size() > 1) {
      // findAllFuncs found more than one function, clear all but one
      pdf = (*exit_func_buf)[0];
      exit_func_buf->clear();
      exit_func_buf->push_back(pdf);
    }

    proc->findAllFuncsByName( "ExitThread", *exit_func_buf);
    if (exit_func_buf->size() > 1) {
      // findAllFuncs found more than one function, clear all but one
      pdf = (*exit_func_buf)[0];
      exit_func_buf->clear();
      exit_func_buf->push_back(pdf);
    }

    pdf = NULL;
#endif // !defined(i386_unknown_nt4_0)

   if ((*exit_func_buf).size() > 0) { 
      mdl_env::add(vname, false, MDL_T_PROCEDURE);
      mdl_env::set(exit_func_buf, vname);
   }

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

// allocate data and generate code for all threads
bool setup_sampled_code_node(const processMetFocusNode* procNode,
			     instrCodeNode* codeNode, pd_process *proc,
			     const string &id, unsigned type,
			     T_dyninstRPC::mdl_constraint *repl_cons,
			     pdvector<T_dyninstRPC::mdl_stmt*> *stmts,
			     const pdvector<string> &temp_ctr, 
			     const Hierarchy &repl_focus_data,
			     bool dontInsertData)
{
   update_environment_start_point(codeNode);
   
   instrDataNode *sampledDataNode = 
      new instrDataNode(proc, type, dontInsertData, codeNode->getHwEvent());

   codeNode->setSampledDataNode(sampledDataNode);
   mdl_env::set(sampledDataNode, id);

   // Create the temporary counters 
   for (unsigned tc=0; tc < temp_ctr.size(); tc++) {
      instrDataNode *tempCtrDataNode = 
         new instrDataNode(proc, MDL_T_COUNTER, dontInsertData);
      codeNode->addTempCtrDataNode(tempCtrDataNode);
      mdl_env::set(tempCtrDataNode, temp_ctr[tc]);
   }
   
   // create the ASTs for the code
   if(repl_cons!=NULL) {
      // mdl_constraint::apply()
      instrDataNode *notAssignedToDataNode;
      if (!repl_cons->apply(codeNode, &notAssignedToDataNode, 
                            repl_focus_data, proc, dontInsertData)) {
         return false;
      }
   } else {
      pdvector<const instrDataNode*> flagNodes = procNode->getFlagDataNodes();
      unsigned size = stmts->size();
      for (unsigned u=0; u<size; u++) {
         // virtual fn call depending on stmt type
         if (!(*stmts)[u]->apply(codeNode, flagNodes)) {
            return false;
         }
      }
   }

   return true;
}

bool setup_constraint_code_node(instrCodeNode* codeNode, pd_process *proc,
				T_dyninstRPC::mdl_constraint *flag_con,
				const Hierarchy &flag_focus_data,
				bool dontInsertData) 
{
   instrDataNode *consDataNode;
   // The following calls mdl_constraint::apply():
   if (!flag_con->apply(codeNode, &consDataNode, flag_focus_data,
			proc, dontInsertData)) {
      return false;
   }
   return true;
}

// returns true if success, false if failure
bool createCodeAndDataNodes(processMetFocusNode **procNode_arg,
		     const string &id, const string &name, 
		     const Focus &no_thr_focus,
		     unsigned type, 
                     string& hw_cntr_str,
		     pdvector<T_dyninstRPC::mdl_constraint*> &flag_cons,
		     T_dyninstRPC::mdl_constraint *repl_cons,
		     pdvector<T_dyninstRPC::mdl_stmt*> *stmts,
		     pdvector<const Hierarchy *> flags_focus_data, 
		     const Hierarchy &repl_focus_data,
		     const pdvector<string> &temp_ctr, 
		     bool /*replace_component*/)
{
   processMetFocusNode *procNode = (*procNode_arg);
   pd_process *proc = procNode->proc();
   bool dontInsertData = procNode->dontInsertData();
   // create the instrCodeNodes and instrDataNodes for the flag constraints
   
   if(repl_cons == NULL) {
      unsigned flag_size = flag_cons.size(); // could be zero
      
      for(unsigned fs=0; fs<flag_size; fs++) {
         string cons_name(flag_cons[fs]->id());
         
         instrCodeNode *consCodeNode = 
            instrCodeNode::newInstrCodeNode(cons_name, no_thr_focus,
                                            proc, dontInsertData);
         bool consCodeNodeComplete = (consCodeNode->numDataNodes() > 0);
         
         if(! consCodeNodeComplete) {
            if(! setup_constraint_code_node(consCodeNode, proc, flag_cons[fs],
                                        *flags_focus_data[fs], dontInsertData))
            {
               delete consCodeNode;
               return false;
            }
         } else {
            metric_cerr << "  flag already there " << endl;
            assert(consCodeNode);
         }
         procNode->addConstraintCodeNode(consCodeNode);	    
      }
   }
   instrCodeNode *metCodeNode = 
      instrCodeNode::newInstrCodeNode(name, no_thr_focus, proc,
                                      dontInsertData, hw_cntr_str);
   
   /* if hw_cntr_str is no good, metCodeNode is NULL */
   if (metCodeNode == NULL) {
      return false;
   }
    
    
   bool metCodeNodeComplete = (metCodeNode->numDataNodes() > 0);
   if(! metCodeNodeComplete) {
      // Create the data objects (timers/counters) and create the
      // astNodes which will be used to generate the instrumentation
      if(! setup_sampled_code_node(procNode, metCodeNode, proc, id, type, 
                                   repl_cons, stmts, temp_ctr,
                                   repl_focus_data, dontInsertData)) {
         delete metCodeNode;
         return false;
      }
   } else {
      //cerr << "  met code node already there, reuse it! " << endl;
   }
   
   if(!metCodeNode->nonNull()) {
      metric_cerr << "metCodeNode->nonNull()" << endl;
      delete metCodeNode;
      return false;
   }
    
   procNode->setMetricVarCodeNode(metCodeNode);
   return true;
}

bool createThreadNodes(processMetFocusNode **procNode_arg,
		       const string &metname, 
		       const Focus &no_thr_focus,
		       const Focus &full_focus) 
{
   processMetFocusNode *procNode = (*procNode_arg);
   pd_process *proc = procNode->proc();
   bool bMT = proc->multithread_capable();

   pdvector<threadMetFocusNode *> threadNodeBuf;
   if(! bMT) {   // --- single-threaded ---
      threadMetFocusNode *thrNode = 
         threadMetFocusNode::newThreadMetFocusNode(metname, no_thr_focus,
                                                   proc->STthread());
      threadNodeBuf.push_back(thrNode);
   } else {      // --- multi-threaded ---
     int thrSelected = full_focus.getThreadID();
      if(thrSelected == -1) {
         Focus focus_with_thr = no_thr_focus;
         threadMgr::thrIter itr = proc->beginThr();
         while(itr != proc->endThrMark()) {
            pd_thread *thr = *itr;
            itr++;
            string start_func_name;
            start_func_name = thr->get_start_func() ? 
               thr->get_start_func()->prettyName()
               : string("no start func!");
            
            string thrName = string("thr_") + string(thr->get_tid()) + "{" + 
                             start_func_name + "}";
            focus_with_thr.set_thread(thrName);
            threadMetFocusNode *thrNode = threadMetFocusNode::
               newThreadMetFocusNode(metname, focus_with_thr, thr);
            threadNodeBuf.push_back(thrNode);
         }
      } else {
         pd_thread *selThr = proc->thrMgr().find_pd_thread(thrSelected);
         if(selThr == NULL) return false;
         threadMetFocusNode *thrNode = 
            threadMetFocusNode::newThreadMetFocusNode(metname, full_focus,
                                                      selThr);
         threadNodeBuf.push_back(thrNode);
      }
   }
   
   for(unsigned k=0; k<threadNodeBuf.size(); k++) {
      procNode->addThrMetFocusNode(threadNodeBuf[k]);
   }
   return true;
}

// Creates the process and primitive metricFocusNodes, as well as mdn's 
// for any specified threads. Then allocate the data (the timers/counters) in
// the application, and create the astNode's which will be used to generate
// the instrumentation code
/* returns 1 if procMF_node is set
   returns 2 if thrMF_node is set
*/
processMetFocusNode *
apply_to_process(pd_process *proc, string& id, string& name,
                 const Focus &focus, unsigned agg_op, unsigned type,
                 string& hw_cntr_str,
                 pdvector<T_dyninstRPC::mdl_constraint*>& flag_cons,
                 T_dyninstRPC::mdl_constraint *repl_cons,
                 pdvector<T_dyninstRPC::mdl_stmt*> *stmts,
                 pdvector<const Hierarchy *> &flags_focus_data,
                 const Hierarchy &repl_focus_data,
                 const pdvector<string> &temp_ctr,
                 bool replace_component, bool dontInsertData) {
   metric_cerr << "apply_to_process()" << endl;
   if (!update_environment(proc)) return NULL;

   Focus full_focus = focus;
   if(! full_focus.machine_defined())
      full_focus.set_machine(machineResource->part_name());
   if(! full_focus.process_defined())
      full_focus.set_process(proc->get_rid()->part_name());
   // full_focus, has the select. machine, process, and possible thread defined

   Focus no_thr_focus = full_focus;
   no_thr_focus.set_thread(string(""));
   // no_thr_focus, has the thr info stripped

   processMetFocusNode *procNode = 
      processMetFocusNode::newProcessMetFocusNode(proc, name, full_focus, 
					  aggregateOp(agg_op), dontInsertData);

   bool ret = createCodeAndDataNodes(&procNode, id, name, no_thr_focus, 
                                     type, hw_cntr_str, flag_cons, repl_cons,
                                     stmts, flags_focus_data, repl_focus_data,
                                     temp_ctr, replace_component);

   if(ret == false) {
      delete procNode;
      return NULL;
   }
   if(createThreadNodes(&procNode, name, no_thr_focus, full_focus) == false) {
      delete procNode;
      return NULL;
   }

   return procNode;
}

static bool apply_to_process_list(pdvector<pd_process*>& instProcess,
				  pdvector<processMetFocusNode*> *procParts,
				  string& id, string& name,
				  const Focus& focus,
				  unsigned& agg_op,
				  unsigned& type,
                                  string& hw_cntr_str, 
			      pdvector<T_dyninstRPC::mdl_constraint*>& flag_cons,
				  T_dyninstRPC::mdl_constraint *repl_cons,
				  pdvector<T_dyninstRPC::mdl_stmt*> *stmts,
				  pdvector<const Hierarchy *> &flags_focus_data,
				  const Hierarchy &repl_focus_data,
				  const pdvector<string> &temp_ctr,
				  bool replace_components_if_present,
				  bool dontInsertData) {
   for(unsigned p=0; p<instProcess.size(); p++) {
      pd_process *proc = instProcess[p];
      assert(proc);
      global_proc = proc;     // TODO -- global
      
      // skip neonatal and exited processes.
      if (proc->status() == exited || proc->status() == neonatal) continue;
      
      processMetFocusNode *procRetNode = 
         apply_to_process(proc, id, name, focus, agg_op, type, hw_cntr_str, 
                          flag_cons, repl_cons, stmts, flags_focus_data, 
                          repl_focus_data, temp_ctr, 
                          replace_components_if_present, dontInsertData);

      if(procRetNode)  (*procParts).push_back(procRetNode);
   }
   
   if((*procParts).size() == 0) return false;  // no components
   return true;
}

///////////////////////////
pdvector<string>global_excluded_funcs;


bool T_dyninstRPC::mdl_metric::apply(
			    pdvector<processMetFocusNode *> *createdProcNodes,
			    const Focus &focus, pdvector<pd_process *> procs, 
	                    bool replace_components_if_present, bool enable) {
  mdl_env::push();
  mdl_env::add(id_, false, MDL_T_DATANODE);
  assert(stmts_);

  const unsigned tc_size = temp_ctr_->size();
  for (unsigned tc=0; tc<tc_size; tc++) {
    mdl_env::add((*temp_ctr_)[tc], false, MDL_T_DATANODE);
  }

  static string machine;
  static bool machine_init= false;
  if (!machine_init) {
    machine_init = true;
    machine = getNetworkName();
  }

  // if another machine was specified, return NULL
  if(! (focus.allMachines() || focus.get_machine() == machine)) {
    return false;
  }

  pdvector<pd_process*> instProcess;
  filter_processes(focus, procs, &instProcess);

  if (!instProcess.size())
    return false;

  // build the list of constraints to use
  pdvector<T_dyninstRPC::mdl_constraint*> flag_cons;

  // the first replace constraint that matches, if any
  T_dyninstRPC::mdl_constraint *repl_cons = NULL;

  // build list of global constraints that match and choose local replace constraint
  const Hierarchy *repl_focus = NULL;
  pdvector<const Hierarchy *> flags_focus_data;
  if (! pick_out_matched_constraints(*constraints_, focus, &flag_cons,
				 &repl_cons, &flags_focus_data, &repl_focus)) {
    return false;
  }

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
  if(repl_cons) {
     repl_cons->mk_list(global_excluded_funcs);
  } else {
     for (unsigned u1 = 0; u1 < flag_cons.size(); u1++) {
	flag_cons[u1]->mk_list(global_excluded_funcs);
     }
     for (unsigned u2 = 0; u2 < stmts_->size(); u2++) {
	(*stmts_)[u2]->mk_list(global_excluded_funcs);
     }
  }
  /*
  metric_cerr << "Metric: " << name_ << endl;
  for (unsigned x1 = 0; x1 < global_excluded_funcs.size(); x1++)
    metric_cerr << "  " << global_excluded_funcs[x1] << endl;
  */
  //////////


  // build the instrumentation request
  bool dontInsertData;
  if (enable) dontInsertData = false;
  else dontInsertData = true;

  pdvector<processMetFocusNode*> procParts; // one per process

  if (type_ == MDL_T_HW_COUNTER || type_ == MDL_T_HW_TIMER) {
#ifdef PAPI
    if (!isPapiInitialized()) {
        string msg = string("PAPI hardware events are unavailable");
        showErrorCallback(125,msg.c_str());
        return false;
    }
    else if (!papiMgr::isHwStrValid(hwcntr_)) {
        string msg = string(hwcntr_ + " PAPI hardware event is invalid");
        showErrorCallback(125,msg.c_str());
        return false;
    }
#else
    string msg = string("PAPI hardware events are not available");
    showErrorCallback(125,msg.c_str());
    return false;
#endif
  }

  if (!apply_to_process_list(instProcess, 
                             createdProcNodes, id_, name_,
			     focus, agg_op_, 
			     type_, hwcntr_, flag_cons, repl_cons,
			     stmts_, flags_focus_data, *repl_focus, *temp_ctr_,
			     replace_components_if_present,
			     dontInsertData)) {
    return false;
  }

  mdl_env::pop();

  ////////////////////
  global_excluded_funcs.resize(0);
  return true;
}

T_dyninstRPC::mdl_constraint::mdl_constraint()
: id_(""), match_path_(NULL), stmts_(NULL), replace_(false),
  data_type_(0), hierarchy_(0), type_(0) { }

T_dyninstRPC::mdl_constraint::mdl_constraint(string id, 
                               pdvector<string> *match_path,
			       pdvector<T_dyninstRPC::mdl_stmt*> *stmts,
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


static bool do_trailing_resources(const pdvector<string>& resource_,
				  pd_process *proc)
{
   pdvector<string>  resPath;

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
           const char* p = trailingRes.c_str();
           char*       q = NULL;
           int         val = (int) strtol(p, &q, 0);
           
           if (p == q) {
              string msg = string("unable to convert resource '") +
                           trailingRes + string("' to integer.");
              showErrorCallback(92,msg.c_str());
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
           pdvector<string> m_vec;
           for(u_int i=0; i < resPath.size()-1; i++){
              m_vec += resPath[i];
           }
           assert(m_vec.size());
           assert(m_vec.size() == (resPath.size()-1));
           resource *m_resource = resource::findResource(m_vec);
           if(!m_resource) {
              return(false);
           }
           
           pdvector<function_base *> *func_buf = new pdvector<function_base*>;
           if ( !proc->findAllFuncsByName(r, m_resource, *func_buf) ) {
              const pdvector<string> &f_names = r->names();
              const pdvector<string> &m_names = m_resource->names();
              string func_name = f_names[f_names.size() -1]; 
              string mod_name = m_names[m_names.size() -1]; 
              
              string msg = string("For requested metric-focus, ") +
                           string("unable to find  function ") +
                           func_name;
              showErrorCallback(95, msg);
              return false;
           }

           mdl_env::add(caStr, false, MDL_T_PROCEDURE);
           mdl_env::set(func_buf, caStr);
           break;
        }
        case MDL_T_MODULE: {
           module *mod = proc->findModule(trailingRes, true);
           if (!mod) {
              string msg = string("For requested metric-focus, ") + 
                 string("unable to find module ") + trailingRes;
              showErrorCallback(95, msg);
              
              /*
                image *img = proc->getImage();
                pdvector<pdmodule *> mods = img->getExcludedModules();
                for(unsigned i=0; i<mods.size(); i++) {
                cerr << "  i: " << i << ", filenm: " << mods[i]->fileName()
                << ", fullnm: " << mods[i]->fullName() << "\n";
                }
              */
              return(false);
           }
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
bool T_dyninstRPC::mdl_constraint::apply(instrCodeNode *codeNode,
					 instrDataNode **dataNode,
					 const Hierarchy &resource,
					 pd_process *proc,
					 bool dontInsertData)
{
   assert(dataNode);
   switch (data_type_) {
     case MDL_T_COUNTER:
     case MDL_T_WALL_TIMER:
     case MDL_T_PROC_TIMER:
     case MDL_T_HW_TIMER:
     case MDL_T_HW_COUNTER:
        break;
     default:
        assert(0);
   }
   mdl_env::push();

   if (!replace_) {
      // create the counter used as a flag
      mdl_env::add(id_, false, MDL_T_DATANODE);
      // "true" means that we are going to create a sampled int counter but
      // we are *not* going to sample it, because it is just a temporary
      // counter - naim 4/22/97
      // By default, the last parameter is false - naim 4/23/97
      
      (*dataNode) = new instrDataNode(proc, MDL_T_COUNTER, dontInsertData,
                                      NULL);
      codeNode->setConstraintDataNode(*dataNode);
      
      // this flag constructs a predicate for the metric -- have to return it
      mdl_env::set(*dataNode, id_);
   }

   // put $constraint[X] in the environment
   if(!do_trailing_resources(resource.tokenized(), proc)) {
      mdl_env::pop();
      return(false);
   }
   
   // Now evaluate the constraint statements
   unsigned size = stmts_->size();
   pdvector<const instrDataNode*> flags;
   bool wasRunning = global_proc->status()==running;
   global_proc->pause();
   for (unsigned u=0; u<size; u++) {
      if (!(*stmts_)[u]->apply(codeNode, flags)) { // virtual fn call         
         if (wasRunning) {
            global_proc->continueProc();
         }
         return(false);
      }
   }
   mdl_env::pop();
   if (wasRunning) {
      global_proc->continueProc();
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
                                        pdvector<string> *path,
					pdvector<T_dyninstRPC::mdl_stmt*> *Cstmts,
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

bool T_dyninstRPC::mdl_for_stmt::apply(instrCodeNode *mn,
				       pdvector<const instrDataNode*>& flags) {
  mdl_env::push();
  mdl_env::add(index_name_, false);
  mdl_var list_var(false);

  // TODO -- build iterator closure here -- list may be vector or dictionary
  if (!list_expr_->apply(list_var))
    return false;
  if (!list_var.is_list())
    return false;

  // TODO
  //  pdvector<function_base*> *vp;
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

// The process is used in generating code which updates the observed cost
// global variable when if statements are called in some cases.  This is to
// account for the cost of the body of the if statement in the observed cost.
bool T_dyninstRPC::mdl_icode::apply(AstNode *&mn, bool mn_initialized,
				    pd_process *proc) 
{
  // a return value of true implies that "mn" has been written to

  if (!expr_)
     return false;

  AstNode* pred = NULL;
  AstNode* ast = NULL;

  if (if_expr_) {
    if (!if_expr_->apply(pred)) {
       return false;
    }
  }

  if (!expr_->apply(ast)) {
    return false;
  }

  AstNode* code = NULL;
  if (pred) 
  {
    // Note: we don't use assignAst on purpose here
    code = createIf(pred, ast, proc->get_dyn_process());
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

T_dyninstRPC::mdl_v_expr::mdl_v_expr(T_dyninstRPC::mdl_expr* expr, pdvector<string> fields) 
: type_(MDL_EXPR_DOT), int_literal_(0), bin_op_(0),
  u_op_(0), left_(expr), right_(NULL), args_(NULL),
  fields_(fields), do_type_walk_(false), ok_(false)
{ assert(0); }

T_dyninstRPC::mdl_v_expr::mdl_v_expr(string func_name,
				     pdvector<T_dyninstRPC::mdl_expr *> *a) 
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

T_dyninstRPC::mdl_v_expr::mdl_v_expr(string var, T_dyninstRPC::mdl_expr* index_expr) 
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
           // of using str_literal_.c_str() directly, just to get rid
           // of the compiler warning of "cast discards const". --chun
           //
           char* tmp_str = new char[strlen(str_literal_.c_str())+1];
           strcpy (tmp_str, str_literal_.c_str());
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
                      // of using value.c_str() directly, just to get rid of the 
                      // compiler warning of "cast discards const".  --chun
                      //
                      char* tmp_str = new char[strlen(value.c_str())+1];
                      strcpy (tmp_str, value.c_str());
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
              instrDataNode* dn;
              if (!(*args_)[0]->apply(timer)) return false;
              if (!timer.get(dn)) return false;

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
              pdvector<AstNode *> ast_args;
              ast = createTimer(timer_func, (void*)(dn->getInferiorPtr()),
                                ast_args);
           }
           else if (var_ == "sampleHwCounter" || var_ == "startHwTimer" || var_ == "stopHwTimer") {

              if (!args_) return false;
              unsigned size = args_->size();
              if (size != 1) return false;

              mdl_var timer(false);
              instrDataNode* dn;
              if (!(*args_)[0]->apply(timer)) return false;
              if (!timer.get(dn)) return false;

              string timer_func;
 
              if (var_ == "startHwTimer")
                 timer_func = "DYNINSTstartHwTimer";
              else if (var_ == "stopHwTimer")
                 timer_func = "DYNINSTstopHwTimer";
              else if (var_ == "sampleHwCounter")
                 timer_func = "DYNINSTsampleHwCounter";

              pdvector<AstNode *> ast_args;

              /* hwCounter can be treated the same way as a hwTimer because
                 both types need to invoke a DYNINST* method 
              */

              HwEvent* hw = dn->getHwEvent();
              assert(hw != NULL);
              int hwCntrIndex = hw->getIndex();  /* temp */

              ast = createHwTimer(timer_func, (void*)(dn->getInferiorPtr()),
                                  ast_args, hwCntrIndex);

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
                 fflush(stderr);
                 return false;
              }
              ast = new AstNode(AstNode::DataAddr, (void*)addr);
           }
           else
           {
              pdvector<AstNode *> astargs;
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

              pdvector<function_base *> fbv;
              function_base *pdf = NULL;

              if (!global_proc->findAllFuncsByName(var_, fbv)) {
                 string msg = string("In metric '") + currentMetric + string("': ")
                    + string("unable to find procedure '") + var_ + string("'");
                 //showErrorCallback(95, msg);
                 cerr << msg;
                 return false;
              }
              else {
                 if (fbv.size() > 1) {
                    string msg = 
                       string("WARNING:  found ") + string(fbv.size()) +
                       string(" records of function '") + var_ + string("'") +
                       string(".  Using the first.(3)");
                    //showErrorCallback(95, msg);
                    cerr << msg;
                 }
                 pdf = fbv[0];
              }

              if (!pdf) 
              {
                 string msg = string("In metric '") + currentMetric + string("': ")
                    + string("unable to find procedure '") + var_ + string("'");
                 //showErrorCallback(95, msg);
                 cerr << msg;
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
        mdl_var get_dn;
        instrDataNode* dn;
        if (!mdl_env::get(get_dn, var_)) return false;
        if (!get_dn.get(dn)) return false;
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
        ast = createCounter(func_str, 
                            (void*)(dn->getInferiorPtr()), ast_arg);
        removeAst (ast_arg);
        return true;
     }
  case MDL_EXPR_VAR:
     {
        if (var_ == "$return") {
           ast = new AstNode(AstNode::ReturnVal, (void*)0);
           return true;
        }
        mdl_var get_dn;
        assert (mdl_env::get(get_dn, var_));
        switch (get_dn.type())
        {
          case MDL_T_INT:
             {
                int value;
                if (!get_dn.get(value)) 
                {
                   fprintf(stderr, "Unable to get value for %s\n", var_.c_str());
                   fflush(stderr);
                   return false;
                }
                else 
                   ast = new AstNode(AstNode::Constant, (void*)value);
                return true;
             }
             //case MDL_T_COUNTER:
          case MDL_T_DATANODE:
             {
                instrDataNode* dn;
                if (!get_dn.get(dn))
                {
                   fprintf(stderr, "Unable to get value for %s\n", var_.c_str());
                   fflush(stderr);
                   return false;
                }
#if defined(MT_THREAD)
                AstNode *tmp_ast;
                tmp_ast = getCounterAddress((void *)(dn->getInferiorPtr()), dn->getSize());
                // First we get the address, and now we get the value...
                ast = new AstNode(AstNode::DataIndir,tmp_ast);
                removeAst(tmp_ast);
#else
                // ast = new AstNode(AstNode::DataValue,  // restore AstNode::DataValue
                ast = new AstNode(AstNode::DataAddr,
                                  (void*)(dn->getInferiorPtr()));
#endif
                return true;
             }
          default:
             fprintf(stderr, "type of variable %s is not known\n", 
                     var_.c_str());
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
                mdl_var get_dn;
                if (!left_->apply(get_dn))
                {
                   string msg = string("In metric '") + currentMetric 
                      + string("' : ") + string("error in operand of address operator");
                   showErrorCallback(92, msg);
                   return false;
                }
                instrDataNode *dn;
                assert (get_dn.get(dn));
#if defined(MT_THREAD)
                ast = getTimerAddress((void *)(dn->getInferiorPtr()), dn->getSize());
#else
                // ast = new AstNode(AstNode::Constant,  // was AstNode::DataPtr
                ast = new AstNode(AstNode::DataPtr,  // restore AstNode::DataPtr
                                  (void*)(dn->getInferiorPtr()));
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
                instrDataNode* dn;
                mdl_var dn_var;
                if (!left_->apply(dn_var)) return false;
                if (!dn_var.get(dn)) return false;

                int value = 1;
                AstNode* ast_arg = new AstNode(AstNode::Constant, (void*)value);

                ast = createCounter("addCounter", 
                                    (void*)(dn->getInferiorPtr()), ast_arg);
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
            || var_ == "startProcessTimer_lwp"
#else
            || var_ == "startProcessTimer" || var_ == "stopProcessTimer"
#endif
            )
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
              pdvector<function_base *> fbv;
              function_base *pdf;
              
              if (!global_proc->findAllFuncsByName(func_name, fbv)) {
                 string msg = string("In metric '") + currentMetric +
                              string("': ") + 
                              string("unable to find procedure '") +
                              func_name + string("'");
                 showErrorCallback(95, msg);
                 assert(0);
              }
              else {
                 if (fbv.size() > 1) {
                    string msg = string("WARNING:  found ") +string(fbv.size())
                               + string(" records of function '") + func_name +
                                 string("'") + string(".  Using the first.");
                    //showErrorCallback(95, msg);
                    cerr << msg << endl;
                 }
                 pdf = fbv[0];
              }
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
           return walk_deref(ret, type_walk);
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

bool T_dyninstRPC::mdl_if_stmt::apply(instrCodeNode *mn,
				      pdvector<const instrDataNode*>& flags) {
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

T_dyninstRPC::mdl_seq_stmt::mdl_seq_stmt(pdvector<T_dyninstRPC::mdl_stmt*> *stmts)
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

bool T_dyninstRPC::mdl_seq_stmt::apply(instrCodeNode *mn,
				       pdvector<const instrDataNode*>& flags) {
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
					   pdvector<string> *elems,
					   bool is_lib, pdvector<string>* flavor) 
  : type_(type), id_(ident), elements_(elems), is_lib_(is_lib), flavor_(flavor) 
  { assert (0); }

T_dyninstRPC::mdl_list_stmt::mdl_list_stmt()
  : type_(MDL_T_NONE), id_(""), elements_(NULL), is_lib_(false), flavor_(NULL)
  { }

T_dyninstRPC::mdl_list_stmt::~mdl_list_stmt() 
{ assert(0); delete elements_; }

bool T_dyninstRPC::mdl_list_stmt::apply(instrCodeNode * /*mn*/,
				       pdvector<const instrDataNode*>& /*flags*/)
{
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
    pdvector<functionName*> *list_fn;
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
	if (sscanf((*elements_)[u].c_str(), "%d", &i) != 1) return false;
	if (!expr_var.set(i)) return false; 
        break;
      case MDL_T_FLOAT:
	if (sscanf((*elements_)[u].c_str(), "%f", &f) != 1) return false;
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
				      pdvector<T_dyninstRPC::mdl_icode*> *reqs,
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

bool T_dyninstRPC::mdl_instr_stmt::apply(instrCodeNode *mn,
					 pdvector<const instrDataNode*>& inFlags) {
   // An instr statement is like:
   //    append preInsn $constraint[0].entry constrained
   //       (* setCounter(procedureConstraint, 1); *)
   // (note that there are other kinds of statements; i.e. there are other classes
   //  derived from the base class mdl_stmt; see dyninstRPC.I)

   if (icode_reqs_ == NULL) {
    return false; // no instrumentation code to put in!
  }

  mdl_var pointsVar(false);
  if (!point_expr_->apply(pointsVar)) { // process the 'point(s)' e.g. "$start.entry"
    return false;
  }

  pdvector<instPoint *> points;
  if (pointsVar.type() == MDL_T_LIST_POINT) {
    pdvector<instPoint *> *pts;
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
    if (!(*icode_reqs_)[u]->apply(code, u>0, mn->proc())) {
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
#if defined(MT_THREAD)
        AstNode *tmp_ast;
        tmp_ast = getCounterAddress((void *)((inFlags[fi])->getInferiorPtr()), inFlags[fi]->getSize());
        AstNode *temp1 = new AstNode(AstNode::DataIndir,tmp_ast);
        removeAst(tmp_ast);
#else
        // Note: getInferiorPtr could return a NULL pointer here if we are
        // just computing cost - naim 2/18/97

	
	// AstNode *temp1 = new AstNode(AstNode::DataValue,
        AstNode *temp1 = new AstNode(AstNode::DataAddr, 
		     (void*)((inFlags[fi])->getInferiorPtr()));
#endif
        // Note: we don't use assignAst on purpose here
        AstNode *temp2 = code;
        code = createIf(temp1, temp2, mn->proc()->get_dyn_process());
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
     mn->addInst(points[i], code, cwhen, corder);
     // appends an instReqNode to mn's instRequests; actual 
     // instrumentation only
     // takes place when mn->loadInstrIntoApp() is later called.
  }

  removeAst(code); 
  return true;
}

bool mdl_can_do(const string &met_name) {
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

bool mdl_do(pdvector<processMetFocusNode *> *createdProcNodes, 
	    const Focus& focus, const string &met_name,
	    const pdvector<pd_process *> &procs,
	    bool replace_components_if_present, bool enable, 
	    aggregateOp *aggOpToUse) {
   currentMetric = met_name;
   unsigned size = mdl_data::all_metrics.size();
   // NOTE: We can do better if there's a dictionary of <metric-name> to
   // <metric>!
   
   for (unsigned u=0; u<size; u++) {
      T_dyninstRPC::mdl_metric *curMetric = mdl_data::all_metrics[u];
      if (curMetric->name_ == met_name) {
         // calls mdl_metric::apply()
         bool ret = curMetric->apply(createdProcNodes, focus, procs,
                                     replace_components_if_present, enable);
         (*aggOpToUse) = aggregateOp(curMetric->agg_op_);
         return ret;
      }
   }
   return false;
}

machineMetFocusNode *makeMachineMetFocusNode(int mid, const Focus& focus, 
			    const string &met_name, 
			    pdvector<pd_process *> procs,
			    bool replace_components_if_present, bool enable) {
  pdvector<processMetFocusNode *> createdProcNodes;
  aggregateOp aggOp;
  bool result = mdl_do(&createdProcNodes, focus, met_name, procs, 
		       replace_components_if_present, enable, &aggOp);

  machineMetFocusNode *retNode = NULL; 
  if(result == true && createdProcNodes.size()>0) {
    retNode = new machineMetFocusNode(mid, met_name, focus, createdProcNodes,
				      aggOp, enable);
  }
  return retNode;
}

processMetFocusNode *makeProcessMetFocusNode(const Focus& focus, 
			    const string &met_name, pd_process *proc,
			    bool replace_components_if_present, bool enable) {
  pdvector<processMetFocusNode *> createdProcNodes;
  aggregateOp aggOp;
  pdvector<pd_process *> procs;
  procs.push_back(proc);
  bool result = mdl_do(&createdProcNodes, focus, met_name, procs, 
		       replace_components_if_present, enable, &aggOp);
  processMetFocusNode *retNode = NULL; 
  if(result == true && createdProcNodes.size()>0) {
    retNode = createdProcNodes[0];
  }
  return retNode;
}

bool mdl_init(string& flavor) { 

  daemon_flavor = flavor;

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

  pdvector<mdl_type_desc> field_list;
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

void dynRPC::send_metrics(pdvector<T_dyninstRPC::mdl_metric*>* var_0) {
  mdl_met = true;

  if (var_0) {
    unsigned var_size = var_0->size();
    for (unsigned v=0; v<var_size; v++) {
      // fprintf(stderr, "Got metric %s\n", (*var_0)[v]->name_.c_str());
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

void dynRPC::send_constraints(pdvector<T_dyninstRPC::mdl_constraint*> *cv) {

  mdl_cons = true;
  if (cv) {
      unsigned var_size = cv->size();
      for (unsigned v=0; v<var_size; v++) {
          bool found = false;
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
void dynRPC::send_stmts(pdvector<T_dyninstRPC::mdl_stmt*> *vs) {
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
    pdvector<const instrDataNode*> flags;

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
void dynRPC::send_libs(pdvector<string> *libs) {

    mdl_libs = true;
    //metric_cerr << "void dynRPC::send_libs(pdvector<string> *libs) called" << endl;
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

static bool walk_deref(mdl_var& ret, pdvector<unsigned>& types) 
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
        case MDL_T_PROCEDURE: {
           pdvector<function_base *> *func_buf_ptr;
           if (!ret.get(func_buf_ptr)) return false;
           pdvector<instPoint *> *inst_point_buf = new pdvector<instPoint*>;
                          
           switch (next_field) {
             case 0:  // .name
             {
                pdvector<string> *nameBuf = new pdvector<string>;
                for(unsigned i=0; i<(*func_buf_ptr).size(); i++) {
                   string prettyName = (*func_buf_ptr)[i]->prettyName();
                   (*nameBuf).push_back(prettyName);
                }
                if (!ret.set(nameBuf)) return false;
                break;
                // TODO: should these be passed a process?  yes, they definitely should!
             }
             case 1:  // .calls
             {
                //  here we should check the calls and exclude the calls to
                //  fns in the global_excluded_funcs list.
                //
                // ARI -- This is probably the spot!
                for(unsigned i=0; i<(*func_buf_ptr).size(); i++) {
                   function_base *pdf = (*func_buf_ptr)[i];
                   
                   pdvector<instPoint*> calls =
                      pdf->funcCalls(global_proc->get_dyn_process());
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
                            // success -- either (a) the call has been bound
                            // already, in which case the instPoint is updated
                            // _and_ callee is set, or (b) the call hasn't yet
                            // been bound, in which case the instPoint isn't
                            // updated but callee *is* updated.
                            callee_name = callee->prettyName().c_str();
                            // metric_cerr << "(successful findCallee() was required) "
                            // << callee_name << endl;
                         }
                      }
                      else 
                      {
                         callee_name = callee->prettyName().c_str();
                         // metric_cerr << "(easy case) " << callee->prettyName() << endl;
                      }
                      
                      // If this callee is in global_excluded_funcs for this
                      // metric (a global vrble...sorry for that), then it's not
                      // really a callee (for this metric, at least), and thus,
                      // it should be removed from whatever we eventually pass
                      // to "ret.set()" below.

                      if (callee_name != NULL) // could be NULL (e.g. indirect fn call)
                         for (unsigned lcv=0; lcv < global_excluded_funcs.size(); lcv++) 
                         {
                            if (0==strcmp(global_excluded_funcs[lcv].c_str(),callee_name))
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
                      const pdvector<instPoint*> *pt_hold = 
                         &(pdf->funcCalls(global_proc->get_dyn_process()));
                      for(unsigned i=0; i<(*pt_hold).size(); i++)
                         (*inst_point_buf).push_back((*pt_hold)[i]);
                   }
                   else 
                   {
                      // metric_cerr << "something was removed! -- doing set() now" << endl;
                      pdvector<instPoint*> pt_hold(calls);
                      for(unsigned i=0; i<pt_hold.size(); i++)
                         (*inst_point_buf).push_back(pt_hold[i]);
                      
                      // WARNING: "setMe" will now be leaked memory!  The
                      // culprit is "ret", which can only take in a _pointer_ to
                      // a vector of instPoint*'s; it can't take in a vector of
                      // instPoints*'s, which we'd prefer.
                   }
                }
                if(! ret.set(inst_point_buf))
                   return false;
                break;
             }
             case 2:  // .entry  (eg. $start.entry or $exit.entry)
             {
                for(unsigned i=0; i<(*func_buf_ptr).size(); i++) {
                   function_base *pdf = (*func_buf_ptr)[i];
                   instPoint *entryPt;
                   entryPt = const_cast<instPoint *>(pdf->funcEntry(
                                             global_proc->get_dyn_process()));
                   (*inst_point_buf).push_back(entryPt);
                }
                if(! ret.set(inst_point_buf))
                   return false;
                break;
             }
             case 3:   // .return
             {
                for(unsigned i=0; i<(*func_buf_ptr).size(); i++) {
                   function_base *pdf = (*func_buf_ptr)[i];
                   const pdvector<instPoint *> func_exit_pts = 
                      pdf->funcExits(global_proc->get_dyn_process());
                   for(unsigned j=0; j<func_exit_pts.size(); j++)
                      (*inst_point_buf).push_back(func_exit_pts[j]);
                }
                if(! ret.set(const_cast<pdvector<instPoint*>*>(inst_point_buf)))
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
                   // this is the correct thing to do...get only the included
                   // funcs associated with this module, but since we seem to
                   // be testing for global_proc elsewhere in this file I
                   // guess we will here too
                   if (!ret.set(global_proc->getIncludedFunctions(mod))) return false; 
                }
                else 
                {
                   // if there is not a global_proc, then just get all
                   // functions associtated with this module....under what
                   // circumstances would global_proc == 0 ???
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

bool mdl_get_lib_constraints(pdvector<string> &lc){
   for(u_int i=0; i < mdl_data::lib_constraints.size(); i++){
      lc += mdl_data::lib_constraints[i];
   }
   return (lc.size()>0);
}

void mdl_get_info(pdvector<T_dyninstRPC::metricInfo>& metInfo) {
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
//bool T_dyninstRPC::mdl_list_stmt::mk_list(pdvector<string> &funcs);
//bool T_dyninstRPC::mdl_for_stmt::mk_list(pdvector<string> &funcs);
//bool T_dyninstRPC::mdl_if_stmt::mk_list(pdvector<string> &funcs);
//bool T_dyninstRPC::mdl_seq_stmt::mk_list(pdvector<string> &funcs);
//bool T_dyninstRPC::mdl_instr_stmt::mk_list(pdvector<string> &funcs);
//bool T_dyninstRPC::mdl_v_expr::mk_list(pdvector<string> &funcs);

bool T_dyninstRPC::mdl_v_expr::mk_list(pdvector<string> &funcs) 
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


bool T_dyninstRPC::mdl_list_stmt::mk_list(pdvector<string> &funcs) {
  if (type_ == MDL_T_PROCEDURE_NAME) {
    unsigned size = elements_->size();
    for (unsigned u = 0; u < size; u++)
      funcs += (*elements_)[u];
  }
  return true;
}

bool T_dyninstRPC::mdl_for_stmt::mk_list(pdvector<string> &funcs) {
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
    pdvector<functionName *> *funcNames;
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

bool T_dyninstRPC::mdl_if_stmt::mk_list(pdvector<string> &funcs) {
  return expr_->mk_list(funcs) && body_->mk_list(funcs);
}

bool T_dyninstRPC::mdl_seq_stmt::mk_list(pdvector<string> &funcs) {
  for (unsigned u = 0; u < stmts_->size(); u++) {
    if (!(*stmts_)[u]->mk_list(funcs))
      return false;
  }
  return true;
}

bool T_dyninstRPC::mdl_instr_stmt::mk_list(pdvector<string> &funcs) {
  return point_expr_->mk_list(funcs);
}

bool T_dyninstRPC::mdl_constraint::mk_list(pdvector<string> &funcs) {
  for (unsigned u = 0; u < stmts_->size(); u++)
    (*stmts_)[u]->mk_list(funcs);
  return true;
}
