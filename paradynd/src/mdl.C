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

// $Id: mdl.C,v 1.173 2005/03/07 21:19:00 bernat Exp $

#include <iostream>
#include <stdio.h>
#include "dyninstRPC.xdr.SRVR.h"
#include "pdutil/h/mdl_data.h"
#include "paradynd/src/machineMetFocusNode.h"
#include "paradynd/src/processMetFocusNode.h"
#include "paradynd/src/threadMetFocusNode.h"
#include "paradynd/src/instrCodeNode.h"
#include "paradynd/src/instrDataNode.h"
#include "paradynd/src/main.h"
#include "common/h/Timer.h"
#include "paradynd/src/mdld.h"
#include "paradynd/src/pd_process.h"
#include "paradynd/src/pd_image.h"
#include "paradynd/src/pd_thread.h"
#include "common/h/debugOstream.h"
#include "pdutil/h/pdDebugOstream.h"
#include "paradynd/src/focus.h"
#include "paradynd/src/papiMgr.h"
#include "paradynd/src/init.h"
#include "paradynd/src/mdld_data.h"
#include "paradynd/src/debug.h"

// for REG_MT_POS
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)
#include "dyninstAPI/src/inst-sparc.h"
#elif defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1)
#include "dyninstAPI/src/inst-power.h"
#elif defined(i386_unknown_solaris2_5) || defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0)
#include "dyninstAPI/src/inst-x86.h"
#endif

#include <ctype.h>

// Some global variables
pdvector<pdstring>global_excluded_funcs;
static pdstring currentMetric;  // name of the metric that is being processed.
static pdstring daemon_flavor;
pd_process *global_proc = NULL;
static bool mdl_met=false, mdl_cons=false, mdl_stmt=false, mdl_libs=false;
bool saw_mdl = false;


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

static bool do_trailing_resources(const pdvector<pdstring>& resource_,
                                    pd_process *proc);

static void filter_processes(const Focus &focus,
                                    pdvector<pd_process*> fromProcs,
                                    pdvector<pd_process*> *filteredOutProcs);

static bool pick_out_matched_constraints(
			     const pdvector<T_dyninstRPC::mdl_constraint*> &cons,
			      const Focus& focus,			      
			      pdvector<T_dyninstRPC::mdl_constraint*> *flag_cons,
			      T_dyninstRPC::mdl_constraint **repl_cons,	      
			      pdvector<const Hierarchy *> *flags_focus_data,
			      const Hierarchy **repl_focus);

static bool apply_to_process_list(pdvector<pd_process*>& instProcess,
				  pdvector<processMetFocusNode*> *procParts,
				  pdstring& id, pdstring& name,
				  const Focus& focus,
				  unsigned& agg_op,
				  unsigned& type,
                  pdstring& hw_cntr_str, 
			      pdvector<T_dyninstRPC::mdl_constraint*>& flag_cons,
				  T_dyninstRPC::mdl_constraint *repl_cons,
				  pdvector<T_dyninstRPC::mdl_stmt*> *stmts,
				  pdvector<const Hierarchy *> &flags_focus_data,
				  const Hierarchy &repl_focus_data,
				  const pdvector<pdstring> &temp_ctr,
				  bool replace_components_if_present,
				  bool dontInsertData);


class list_closure 
{
public:
  list_closure(pdstring& i_name, mdl_var& list_v)
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
      pdstring s;
      BPatch_function *bpf; BPatch_module *m;
      BPatch_Vector<BPatch_function *> bpfv;
      
      float f; int i;
      BPatch_point *ip;
      
      if (index >= max_index) return false;
      switch(element_type) 
      {
        case MDL_T_INT:
           i = (*int_list)[index++];
           return (mdl_data::cur_mdl_data->env->set(i, index_name));
        case MDL_T_FLOAT:
           f = (*float_list)[index++];
           return (mdl_data::cur_mdl_data->env->set(f, index_name));
        case MDL_T_STRING:
           s = (*string_list)[index++];
           return (mdl_data::cur_mdl_data->env->set(s, index_name));
        case MDL_T_PROCEDURE_NAME: {
           // lookup-up the functions defined in resource lists
           // the function may not exist in the image, in which case we get the
           // next one
           bpf = NULL;
           do 
           {
              functionName *fn = (*funcName_list)[index++];
              if (!global_proc->findAllFuncsByName(fn->get(), bpfv)) {
                 //string msg = pdstring("unable to find procedure '") + 
                 //             fn->get() + pdstring("'");
                 //showErrorCallback(95, msg);
                 continue;
              }
              else if (bpfv.size() > 1) {
#ifdef FIXME_AFTER_4
                 pdstring msg = pdstring("WARNING:  found ") + pdstring(bpfv.size()) +
                              pdstring(" records of function '") + fn->get() +
                              pdstring("'") + pdstring(".  Using the first.(1)");
                 //showErrorCallback(95, msg);
                 cerr << msg << endl;
#endif
              }
              bpf = bpfv[0];
           } while (bpf == NULL && index < max_index);
           
           if (bpf == NULL) 
              return false;
           
           BPatch_Vector<BPatch_function *> *func_buf = new BPatch_Vector<BPatch_function*>;
           (*func_buf).push_back(bpf);
           return (mdl_data::cur_mdl_data->env->set(func_buf, index_name));
        }
        case MDL_T_PROCEDURE: {
           bpf = (*func_list)[index++];
           BPatch_Vector<BPatch_function *> *func_buf = new BPatch_Vector<BPatch_function*>;
           (*func_buf).push_back(bpf);
           assert(bpf);
           return (mdl_data::cur_mdl_data->env->set(func_buf, index_name));
        }
        case MDL_T_MODULE: 
           m = (*mod_list)[index++];
           assert (m);
           return (mdl_data::cur_mdl_data->env->set(m, index_name));
        case MDL_T_POINT:
           ip = (*point_list)[index++];
           assert (ip);
           return (mdl_data::cur_mdl_data->env->set(ip, index_name));
        default:
           assert(0);
      }
      return false;
   }

private:
  pdstring index_name;
  unsigned element_type;
  unsigned index;
  pdvector<int> *int_list;
  pdvector<float> *float_list;
  pdvector<pdstring> *string_list;
  pdvector<bool> *bool_list;
  BPatch_Vector<BPatch_function*> *func_list;
  pdvector<functionName*> *funcName_list;
  pdvector<BPatch_module*> *mod_list;
  pdvector<BPatch_point*> *point_list;
  unsigned max_index;
};

//  A helper wrapper for findFunction to get rid of a bunch of messy looking
//  error handling, message-printing stuff.

BPatch_function *mdlFindFunction(const char *name, BPatch_image *appImage, 
				 bool ignore_warnings = false)
{
  BPatch_Vector<BPatch_function *> funcs;
  BPatch_function *ret = NULL;
   if (!appImage->findFunction(name, funcs)) {
     fprintf(stderr, "%s[%d]:  cannot find function %s\n", __FILE__, __LINE__, name);
     return NULL;
   }
   if (!funcs.size()) {
     fprintf(stderr, "%s[%d]:  cannot find function %s\n", __FILE__, __LINE__, name);
     return NULL;
   }
   if (funcs.size() > 1 && !ignore_warnings)
     fprintf(stderr, "%s[%d]:  warning, found more than one function called '%s'\n",
             __FILE__, __LINE__, name);
   ret = funcs[0];
   return ret;
}


/*
 * Simple: return base + (POS * size). ST: POS==0, so return base
 */
 
BPatch_snippet *getTimerSlow(BPatch_variableExpr *base,
                             BPatch_image *appImage,
                             bool for_multithreaded)
{
   if(! for_multithreaded) {
      return base;
      //return new BPatch_constExpr(base->getBaseAddr());
   }

   static BPatch_function *pthread_self_fn = NULL;
   static BPatch_function *index_slow_fn = NULL;

   if (!pthread_self_fn)
     if (! (pthread_self_fn = mdlFindFunction("pthread_self", appImage, true)))
       return NULL;

   if (!index_slow_fn)
     if (! (index_slow_fn = mdlFindFunction("DYNINSTthreadIndexSLOW", appImage)))
       return NULL;

   //  construct expr:
   //  (base) + DYNINSTthreadIndexSLOW(pthread_self()) * struct_size

   BPatch_Vector<BPatch_snippet *> snip_args;
   BPatch_snippet *get_thr = new BPatch_funcCallExpr(*pthread_self_fn, snip_args);
   snip_args.push_back(get_thr);
   BPatch_snippet *get_index = new BPatch_funcCallExpr(*index_slow_fn, snip_args);

   BPatch_snippet *var = new BPatch_arithExpr(BPatch_ref, *base, *get_index);
   if (var->is_trivial()) {
     delete var;
     delete get_index;
     delete get_thr;
     fprintf(stderr, "%s[%d]:  generate array reference failed\n", __FILE__, __LINE__);
     return NULL;
   }
   return var;
}

BPatch_snippet *getTimerFast(BPatch_variableExpr *base,
                             bool for_multithreaded)
{
   if(! for_multithreaded)
      //return new BPatch_constExpr(base->getBaseAddr());
      return base;
      //return new BPatch_snippet(*base);

   // Return base + struct_size*POS. Problem is, POS is unknown
   // until we are running the instrumentation

   //  construct expr:
   //  (base) + register[REG_MT_POS] * struct_size

   BPatch_snippet *reg_mt = new BPatch_regExpr(REG_MT_POS);
   BPatch_snippet *var = new BPatch_arithExpr(BPatch_ref, *base, *reg_mt);

   // delete reg_mt;
   if (var->is_trivial()) {
     delete reg_mt;
     delete var;
     fprintf(stderr, "%s[%d]:  generate array reference failed\n", __FILE__, __LINE__);
     return NULL;
   }

   return var;
}

BPatch_snippet *getTimerAddress(BPatch_variableExpr *base,
                                BPatch_image *appImage,
                                bool for_multithreaded)
{
  BPatch_snippet *temp;

#if defined(i386_unknown_linux2_0)
  temp =  getTimerSlow(base, appImage, for_multithreaded);
#else
  temp =  getTimerFast(base, for_multithreaded);
#endif

  if (!temp) {
    fprintf(stderr, "%s[%d]:  Fatal internal error in getTimerAddress()\n",
           __FILE__, __LINE__);
    return NULL;
  }

  BPatch_snippet * ret = new BPatch_arithExpr(BPatch_address, (*temp));
  //delete temp;
  return ret;
}

BPatch_snippet *createTimer(const pdstring &func, BPatch_variableExpr *dataPtr,
                            BPatch_Vector<BPatch_snippet *> &snip_args,
                            BPatch_image *appImage,
                            bool for_multithreaded)
{
   BPatch_snippet *var_base = NULL;

   BPatch_function *timer_fn = mdlFindFunction(func.c_str(), appImage);
   if (NULL == timer_fn)
     return NULL;

   if (NULL == (var_base = getTimerAddress(dataPtr, appImage, for_multithreaded))){
     fprintf(stderr, "%s[%d]:  getTimerAddress() failed\n", __FILE__, __LINE__);
     return NULL;
   }

   snip_args.push_back(var_base);
   return new BPatch_funcCallExpr(*(timer_fn), snip_args);
}

// If a process is passed, the returned Ast code will also update the
// observed cost of the process according to the cost of the if-body when the
// if-body is executed.  However, it won't add the the update code if it's
// considered that the if-body code isn't significant.  We consider an
// if-body not worth updating if (cost-if-body < 5*cost-of-update).  proc ==
// NULL (the default argument) will mean to not add to the AST's the code
// which adds this observed cost update code.

BPatch_snippet *createIf(BPatch_boolExpr *expression, BPatch_snippet *action,
                        BPatch_thread *appThread)
{
  if(appThread != NULL) {
    // add code to the snippet to update the global observed cost variable
    // we want to add the minimum cost of the body.  Observe the following
    // example
    //    if(condA) { if(condB) { STMT-Z; } }
    // This will generate the following code:
    //    if(condA) { if(condB) { STMT-Z; <ADD-COSTOF:STMT-Z>; }
    //               <ADD-COSTOF:if(condB)>;
    //              }
    // the <ADD-COSTOF: > is what's returned by minCost, which is what
    // we want.
    // Each if statement will be constructed by createIf().

    //  Var in Dyninst RT:  DYNINSTobsCostLow -- this should be moved!
    BPatch_variableExpr *obsCostVar;
    obsCostVar = appThread->getImage()->findVariable("DYNINSTobsCostLow", true);
    if (!obsCostVar) {
      fprintf(stderr, "%s[%d]:  cannot find variable: DYNINSTobsCostLow!\n",
             __FILE__, __LINE__);
      return NULL;
    }

    int costOfIfBody = action->PDSEP_astMinCost();
    BPatch_constExpr *fake_addConstCost = new BPatch_constExpr(costOfIfBody);
    BPatch_arithExpr *fake_addCode = new BPatch_arithExpr(BPatch_plus,
                                                          *fake_addConstCost,
                                                          *obsCostVar);
    BPatch_arithExpr *fake_updateCode = new BPatch_arithExpr(BPatch_assign,
                                                             *obsCostVar,
                                                             *fake_addCode);


    // fake_updateCode is just created to get the cost. "real" snippet generated below
    int updateCost = fake_updateCode->PDSEP_astMinCost();
    BPatch_constExpr *addConstCost = new BPatch_constExpr(costOfIfBody + updateCost);
    BPatch_arithExpr *addCode = new BPatch_arithExpr(BPatch_plus,
                                                     *addConstCost,
                                                     *obsCostVar);
    BPatch_arithExpr *updateCode = new BPatch_arithExpr(BPatch_assign,
                                                        *obsCostVar,
                                                        *addCode);

    // eg. if costUpdate=10 cycles, won't do update if bodyCost=40 cycles
    //                               will do update if bodyCost=60 cycles
    const int updateThreshFactor = 5;
    if(costOfIfBody > updateThreshFactor * updateCost) {
      BPatch_Vector<BPatch_snippet *> seq;
      seq.push_back(action);
      seq.push_back(updateCode);
      BPatch_sequence *act_with_cost_compute = new BPatch_sequence(seq);
      action = act_with_cost_compute;
      //  mem leak ?? ...  ssssssssssssss
    }
  }

  BPatch_ifExpr *t = new BPatch_ifExpr(*expression, *action);
  return(t);
}

BPatch_snippet *createHwTimer(const pdstring &func, void *dataPtr,
                              BPatch_Vector<BPatch_snippet *> &snip_args,
                              int hwCntrIndex, BPatch_image *appImage)
{

  BPatch_function *timer_fn = mdlFindFunction(func.c_str(), appImage);
  if (NULL == timer_fn) return NULL;

  BPatch_constExpr var_base(dataPtr);
  BPatch_constExpr hw_var(hwCntrIndex);

  snip_args.push_back(&var_base);
  snip_args.push_back(&hw_var);

  BPatch_funcCallExpr *timer = new BPatch_funcCallExpr(*timer_fn, snip_args);

  return(timer);
}

BPatch_snippet *getCounterSlow(BPatch_variableExpr *base, BPatch_image *appImage)
{
   static BPatch_function *pthread_self_fn = NULL;
   static BPatch_function *index_slow_fn = NULL;

   if (!pthread_self_fn)
     if (! (pthread_self_fn = mdlFindFunction("pthread_self", appImage, true)))
       return NULL;

   if (!index_slow_fn)
     if (! (index_slow_fn = mdlFindFunction("DYNINSTthreadIndexSLOW", appImage)))
       return NULL;

   //  construct expr:
   //  (base) + DYNINSTthreadIndexSLOW(pthread_self()) * struct_size

   BPatch_Vector<BPatch_snippet *> snip_args;
   BPatch_snippet *get_thr = new BPatch_funcCallExpr(*pthread_self_fn, snip_args);
   snip_args.push_back(get_thr);
   BPatch_snippet *get_index = new BPatch_funcCallExpr(*index_slow_fn, snip_args);

   //delete get_thr;
   //delete get_index;

   BPatch_snippet *var = new BPatch_arithExpr(BPatch_ref, *base, *get_index);
   if (var->is_trivial()) {
     delete get_thr;
     delete var;
     fprintf(stderr, "%s[%d]:  generate array reference failed\n", __FILE__, __LINE__);
     return NULL;
   }
   return var;
}

BPatch_snippet *getCounterFast(BPatch_variableExpr *base)
{
   //  construct expr:
   //  (base) + register[REG_MT_POS] * struct_size

   BPatch_snippet *reg_mt = new BPatch_regExpr(REG_MT_POS);
   BPatch_snippet *var = new BPatch_arithExpr(BPatch_ref, *base, *reg_mt);

   if (var->is_trivial()) {
     delete reg_mt;
     fprintf(stderr, "%s[%d]:  generate array reference failed\n", __FILE__, __LINE__);
     return NULL;
   }

   //delete reg_mt;
   return var;
}

BPatch_snippet *getCounter(BPatch_variableExpr *base,
                           BPatch_image *appImage,
                           bool for_multithreaded)
{
   BPatch_snippet *ret;

   if (!for_multithreaded) {
     //ret= new BPatch_arithExpr(BPatch_deref, *base);
     //  try just returning base here -- not sure if copy is necessary
     return base;
     //return ret;
     //return new BPatch_constExpr(base->getBaseAddr());
   }
   else {
#if defined(i386_unknown_linux2_0)
     ret = getCounterSlow(base, appImage);
#else
     ret = getCounterFast(base);
#endif
   }

   return ret;
}

BPatch_snippet *createCounter(const pdstring &func,
                              BPatch_variableExpr *dataPtr,
                              BPatch_snippet *arg, bool for_multithreaded,
                              BPatch_image *appImage)
{
   BPatch_snippet *calc=NULL, *store=NULL;
   BPatch_snippet *counter_base = NULL;

   // We keep the different MT__THREAD code, because otherwise we really
   // de-optimize the singlethread case

   counter_base = getCounter(dataPtr, appImage, for_multithreaded);
   if (!counter_base) {
     fprintf(stderr, "%s[%d]:  fatal internal error in createCounter()\n",
             __FILE__, __LINE__);
     return NULL;
   }

   if (func=="addCounter") {
      // counter = counter + arg;
      calc = new BPatch_arithExpr(BPatch_plus, *counter_base, *arg);
      store = new BPatch_arithExpr(BPatch_assign, *counter_base, *calc);
   }
   else if (func=="subCounter") {
      // counter = counter - arg;
      calc = new BPatch_arithExpr(BPatch_minus, *counter_base, *arg);
      store = new BPatch_arithExpr(BPatch_assign, *counter_base, *calc);
   }
   else if (func=="setCounter") {
      // counter = arg;
      store = new BPatch_arithExpr(BPatch_assign, *counter_base, *arg);
   }
   else abort();

   //if (counter_base && alloced_counter) delete counter_base;
   //if (calc) delete calc;

   return store;
}

//----------------------------------------------------------------------------
// mdl_v_expr backend methods
//----------------------------------------------------------------------------

#define START_WALL_TIMER       "DYNINSTstartWallTimer"
#define STOP_WALL_TIMER        "DYNINSTstopWallTimer"
#define START_PROC_TIMER       "DYNINSTstartProcessTimer"
#define STOP_PROC_TIMER        "DYNINSTstopProcessTimer" 
#define MT_START_PROC_TIMER    "DYNINSTstartThreadTimer"
#define MT_STOP_PROC_TIMER     "DYNINSTstopThreadTimer" 
#define DESTROY_PROC_TIMER     "DYNINSTdestroyThreadTimer"


bool
mdld_v_expr::apply_be(BPatch_snippet*& snip)
{
   switch (type_) 
   {
     case MDL_EXPR_INT: 
        {
           snip = new BPatch_constExpr(int_literal_);
           return true;
        }
     case MDL_EXPR_STRING:
        {
           // create another pdstring here and pass it to AstNode(), instead
           // of using str_literal_.c_str() directly, just to get rid
           // of the compiler warning of "cast discards const". --chun
           //
           char* tmp_str = new char[strlen(str_literal_.c_str())+1];
           strcpy (tmp_str, str_literal_.c_str());
           snip = new BPatch_constExpr((const char *)tmp_str);
           delete[] tmp_str;
           return true;
        }
     case MDL_EXPR_INDEX:
        {
           mdl_var index_var;
           if (!dynamic_cast<mdld_expr*>(left_)->apply_be(index_var))
              return false;
           int index_value;
           if (!index_var.get(index_value))
              return false;

           if (var_ == pdstring ("$arg"))
              snip = new BPatch_paramExpr ((int)index_value);
           else if (var_ == pdstring ("$constraint"))
           {
              pdstring tmp = pdstring("$constraint") + pdstring(index_value);
              mdl_var int_var;
              assert (mdl_data::cur_mdl_data->env->get(int_var, tmp));
              int value;
              if (!int_var.get(value))
              {
                 fflush(stderr);
                 return false;
              }
              snip = new BPatch_constExpr((int)value);
           }
           else
           {
              mdl_var array_var;
              if (!mdl_data::cur_mdl_data->env->get(array_var, var_)) return false;
              if (!array_var.is_list()) return false;  
              mdl_var element;
              assert (array_var.get_ith_element(element, index_value));
              switch (element.get_type())
              {
                case MDL_T_INT:
                   {
                      int value;
                      assert (element.get(value));
                      snip = new BPatch_constExpr((int)value);
                      break;
                   }
                case MDL_T_STRING:
                   {
                      pdstring value;
                      assert (element.get(value));
                      //
                      // create another pdstring here and pass it to AstNode(), instead
                      // of using value.c_str() directly, just to get rid of the 
                      // compiler warning of "cast discards const".  --chun
                      //
                      char* tmp_str = new char[strlen(value.c_str())+1];
                      strcpy (tmp_str, value.c_str());
                      snip = new BPatch_constExpr((const char *)tmp_str);
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
           BPatch_snippet* lnode_p;
           BPatch_snippet* rnode_p;
           if (!dynamic_cast<mdld_expr*>(left_)->apply_be (lnode_p)) return false;
           if (!dynamic_cast<mdld_expr*>(right_)->apply_be (rnode_p)) return false;
           BPatch_snippet &lnode = *lnode_p;
           BPatch_snippet &rnode = *rnode_p;

           switch (bin_op_) 
           {
             case MDL_PLUS:
                snip = new BPatch_arithExpr(BPatch_plus, lnode, rnode);
                break;
             case MDL_MINUS:
                snip = new BPatch_arithExpr(BPatch_minus, lnode, rnode);
                break;
             case MDL_DIV:
                snip = new BPatch_arithExpr(BPatch_divide, lnode, rnode);
                break;
             case MDL_MULT:
                snip = new BPatch_arithExpr(BPatch_times, lnode, rnode);
                break;
             case MDL_LT:
                snip = new BPatch_boolExpr(BPatch_lt, lnode, rnode);
                break;
             case MDL_GT:
                snip = new BPatch_boolExpr(BPatch_gt, lnode, rnode);
                break;
             case MDL_LE:
                snip = new BPatch_boolExpr(BPatch_le, lnode, rnode);
                break;
             case MDL_GE:
                snip = new BPatch_boolExpr(BPatch_ge, lnode, rnode);
                break;
             case MDL_EQ:
                snip = new BPatch_boolExpr(BPatch_eq, lnode, rnode);
                break;
             case MDL_NE:
                snip = new BPatch_boolExpr(BPatch_ne, lnode, rnode);
                break;
             case MDL_AND:
                snip = new BPatch_boolExpr(BPatch_and, lnode, rnode);
                break;
             case MDL_OR:
                snip = new BPatch_boolExpr(BPatch_or, lnode, rnode);
                break;
             default: 
                return false;
           }
           return true;
        }
     case MDL_EXPR_FUNC:
        {
           if (var_ == "startWallTimer" || var_ == "stopWallTimer"
               || var_ == "startProcessTimer" || var_ == "stopProcessTimer"
               || var_ == "destroyProcessTimer")
           {
              if (!args_) return false;
              unsigned size = args_->size();
              if (size != 1) return false;

              mdl_var timer(false);
              instrDataNode* dn;
              if (!dynamic_cast<mdld_expr*>((*args_)[0])->apply_be(timer))
                 return false;
              if (!timer.get(dn))
                 return false;

              //XXX 
              // set miss cycle count reader func here

              pdstring timer_func;
              if (var_ == "startWallTimer")
                 timer_func = START_WALL_TIMER;
              else if (var_ == "stopWallTimer")
                 timer_func = STOP_WALL_TIMER;
              else if (var_ == "startProcessTimer") {
                 if(global_proc->multithread_capable())
                    timer_func = MT_START_PROC_TIMER;
                 else
                    timer_func = START_PROC_TIMER;
              } else if (var_ == "stopProcessTimer") {
                 if(global_proc->multithread_capable())                    
                    timer_func = MT_STOP_PROC_TIMER;
                 else
                    timer_func = STOP_PROC_TIMER;                    
              } else if (var_ == "destroyProcessTimer")
                 timer_func = DESTROY_PROC_TIMER;

              BPatch_image *appImage = global_proc->getImage()->get_dyn_image();
              BPatch_variableExpr *base_var = dn->getVariableExpr();
              if (!base_var) return false;

              BPatch_Vector<BPatch_snippet *> snip_args;
              snip = createTimer(timer_func, base_var, snip_args,
                                appImage, global_proc->multithread_capable());
              if (!snip) return false;
           }
           else if (var_ == "sampleHwCounter" || var_ == "startHwTimer" || var_ == "stopHwTimer") {

              if (!args_) return false;
              unsigned size = args_->size();
              if (size != 1) return false;

              mdl_var timer(false);
              instrDataNode* dn;
              if (!dynamic_cast<mdld_expr*>((*args_)[0])->apply_be(timer)) return false;
              if (!timer.get(dn)) return false;

              pdstring timer_func;
 
              if (var_ == "startHwTimer")
                 timer_func = "DYNINSTstartHwTimer";
              else if (var_ == "stopHwTimer")
                 timer_func = "DYNINSTstopHwTimer";
              else if (var_ == "sampleHwCounter")
                 timer_func = "DYNINSTsampleHwCounter";

              /* hwCounter can be treated the same way as a hwTimer because
                 both types need to invoke a DYNINST* method 
              */

              HwEvent* hw = dn->getHwEvent();
              assert(hw != NULL);
              int hwCntrIndex = hw->getIndex();  /* temp */

              BPatch_Vector<BPatch_snippet *> snip_args;
              BPatch_image *appImage = global_proc->getImage()->get_dyn_image();
              snip = createHwTimer(timer_func, (void*)(dn->getInferiorPtr()),
                                  snip_args, hwCntrIndex, appImage);

           }
      
           else if (var_ == "readSymbol")
           {
              mdl_var symbol_var;
              if (!dynamic_cast<mdld_expr*>((*args_)[0])->apply_be(symbol_var))
                 return false;
              pdstring symbol_name;
              if (!symbol_var.get(symbol_name))
              {
                 fprintf (stderr, "Unable to get symbol name for readSymbol()\n");
                 fflush(stderr);
                 return false;
              }

              // relying on global_proc to be set in mdl_metric::apply
              if (global_proc) 
              {
                 fprintf(stderr, "%s[%d]:  PDSEP:  getSymbolInfo(%s)\n",
                         __FILE__, __LINE__, symbol_name.c_str());

                 pd_image *pdimg = global_proc->getImage();
                 BPatch_image *appImage = pdimg->get_dyn_image();

                 snip = appImage->findVariable(symbol_name.c_str(), true);
                 if (!snip) {
                    pdstring msg = pdstring("In metric '") + currentMetric
                                   + pdstring("': ")
                                   + pdstring("unable to find symbol '")
                                   + symbol_name + pdstring("'");
                    mdl_data::cur_mdl_data->env->appendErrorString( msg );
                    return false;
                 }

              }
           }
           else if (var_ == "readAddress")
           {

             fprintf(stderr, "%s[%d]:  PDSEP -- FIXME -- readAddress()\n",
                     __FILE__, __LINE__);
             //  I've never seen this clause executed.  The following
             //  formulation _should_ work, but its flagged with a warning
             //  until this can be verified.

             //  The previous ast representation of this was an untyped
             //  DataAddr ast node, so there might be a loss of generality
             //  here, as this now assumes that the <addr> points to "int"

             mdl_var addr_var;
             if (!dynamic_cast<mdld_expr*>((*args_)[0])->apply_be (addr_var))
                return false;
             int addr;
             if (!addr_var.get(addr))
             {
                fflush(stderr);
                return false;
             }

             BPatch_image *appImage = global_proc->getImage()->get_dyn_image();
             BPatch_type *inttype = appImage->findType("int");
             if (! inttype) {
               fprintf(stderr, "%s[%d]:  cannot find int type\n", 
                      __FILE__, __LINE__);
               return false;
             }

             // There might be a better way to name this variable using
             // a mdl name...
             char addr_name[64];
             sprintf(addr_name, "addr_%p", (void *)(long)addr);
             BPatch_thread *appThread = global_proc->get_dyn_process();
             snip = new BPatch_variableExpr(addr_name, appThread,
                                           (void *)(long)addr, inttype);
           }
           else
           {
              BPatch_Vector<BPatch_snippet *> snipargs;
              for (unsigned u = 0; u < args_->size(); u++)
              {
                 BPatch_snippet *tmparg=NULL;
                 if (!dynamic_cast<mdld_expr*>((*args_)[u])->apply_be(tmparg))
                   return false;
                 snipargs.push_back(tmparg);
              }


              BPatch_image *appImage = global_proc->getImage()->get_dyn_image();
              BPatch_function *bpf = NULL;

              if (NULL == (bpf = mdlFindFunction(var_.c_str(), appImage)))
                return false;

              snip = new BPatch_funcCallExpr(*bpf, snipargs);
              break;
           }
        return true;
   }
  case MDL_EXPR_DOT:
     {
        //??? only allow left hand of DOT to be "$constraint[i]"?

        mdl_var dot_var;
        if (!dynamic_cast<mdld_expr*>(left_)->apply_be(dot_var))
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
        if (!mdl_data::cur_mdl_data->env->get(get_dn, var_)) return false;
        if (!get_dn.get(dn)) return false;
        BPatch_snippet* snip_arg;
        if (!dynamic_cast<mdld_expr*>(left_)->apply_be(snip_arg)) return false;

        pdstring func_str;
        switch (bin_op_)
        {
          case MDL_ASSIGN: func_str = "setCounter"; break;
          case MDL_PLUSASSIGN: func_str = "addCounter"; break;
          case MDL_MINUSASSIGN: func_str = "subCounter"; break;
          default: return false;
        }

        BPatch_variableExpr *base_var = dn->getVariableExpr();
        if (!base_var) return false;
        //BPatch_variableExpr *temp_var = dn->getVariableExpr();
        //if (!temp_var) return false;
        //BPatch_snippet *base_var = new BPatch_snippet(*temp_var);


        snip = createCounter(func_str, (BPatch_variableExpr *)base_var, snip_arg,
                             global_proc->multithread_capable(),
                             global_proc->get_dyn_process()->getImage());
        if (NULL == snip)
          fprintf(stderr, "%s[%d]:  createCounter failed!\n", __FILE__, __LINE__);
        return (snip != NULL);
     }
  case MDL_EXPR_VAR:
     {
        if (var_ == "$return") {
           snip = new BPatch_retExpr();
           return true;
        }
        mdl_var get_dn;
        assert (mdl_data::cur_mdl_data->env->get(get_dn, var_));
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
                   snip = new BPatch_constExpr(value);

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

                BPatch_variableExpr *base_var = dn->getVariableExpr();
                if (!base_var) return false;

                snip = getCounter(base_var, global_proc->get_dyn_process()->getImage(),
                                  global_proc->multithread_capable());
                if (!snip) {
                   fprintf(stderr, "%s[%d]:  getCounter() failed!\n", __FILE__, __LINE__);
                   return false;
                }
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
                if (!dynamic_cast<mdld_expr*>(left_)->apply_be(get_dn))
                {
                   pdstring msg = pdstring("In metric '") + currentMetric 
                      + pdstring("' : ") + pdstring("error in operand of address operator");
                   mdl_data::cur_mdl_data->env->appendErrorString( msg );
                   return false;
                }
                instrDataNode *dn;
                assert (get_dn.get(dn));

                BPatch_variableExpr *base_var = dn->getVariableExpr();
                if (!base_var) return false;

                snip = getTimerAddress(base_var, global_proc->get_dyn_process()->getImage(),
                                       global_proc->multithread_capable());
                if (!snip) {
                   fprintf(stderr, "%s[%d]: getTimerAddress() failed.\n", __FILE__, __LINE__);
                   return false;
                }

                break;
             }
          case MDL_MINUS:
             {
                mdl_var tmp;
                if (!dynamic_cast<mdld_expr*>(left_)->apply_be (tmp)) return false;
                int value;
                if (!tmp.get(value)) return false;
                snip = new BPatch_constExpr(-value);

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
                if (!dynamic_cast<mdld_expr*>(left_)->apply_be(dn_var)) return false;
                if (!dn_var.get(dn)) return false;

                int value = 1;
                BPatch_snippet* snip_arg = new BPatch_constExpr(value);
                BPatch_variableExpr *base_var = dn->getVariableExpr();
                snip = createCounter("addCounter",(BPatch_variableExpr *)base_var,
                                    snip_arg, global_proc->multithread_capable(),
                                    global_proc->get_dyn_process()->getImage());
                if (!snip) {
                   fprintf(stderr, "%s[%d]:  createCounter failed!\n", __FILE__, __LINE__);
                   return false;
                }

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

bool
mdld_v_expr::apply_be(mdl_var& ret) 
{
   switch (type_) 
   {
     case MDL_EXPR_INT: 
         {
             return ret.set(int_literal_);
         }
     case MDL_EXPR_STRING:
         {
             return ret.set(str_literal_);
         }
     case MDL_EXPR_INDEX: 
        {
           if (var_ == pdstring ("$arg"))
           {
              // we only allow $arg to appear inside icode (is this right?),
              // and therefore, the other mdl_v_expr::apply_be() should be used for
              // $arg, and not this one. --chun
              assert (0);
           }
           if (var_ == pdstring ("$constraint"))
           {
              mdl_var ndx(false);

              if (!dynamic_cast<mdld_expr*>(left_)->apply_be(ndx)) {
                  return false;
              }

              int x;
              if (!ndx.get(x)) {
                  return false;
              }
              
              //XXX
              // this is executing before do_trailing resources has a chance
              // to set the buf. 

              bool tmp = (mdl_data::cur_mdl_data->env->get(ret, var_+pdstring(x)));
              return tmp;
              //              return (mdl_data::cur_mdl_data->env->get(ret, var_+pdstring(x)));
           }

           mdl_var array(false);

           if (!mdl_data::cur_mdl_data->env->get(array, var_)) {
               return false;
           }

           if (!array.is_list()) {
               return false;  
           }

           mdl_var index_var;
           if (!dynamic_cast<mdld_expr*>(left_)->apply_be(index_var)) {
               return false;
           }

           int index_value;
           if (!index_var.get(index_value)) {
               return false;
           }

           if (index_value >= (int)array.list_size()) {
               return false;
           }

           return array.get_ith_element(ret, index_value);
        }
     case MDL_EXPR_BINOP:
        {
           mdl_var left_val(false), right_val(false);
           if (!left_ || !right_) return false;
           if (!dynamic_cast<mdld_expr*>(left_)->apply_be(left_val)) return false;
           if (!dynamic_cast<mdld_expr*>(right_)->apply_be(right_val)) return false;
           return (do_operation(ret, left_val, right_val, bin_op_));
        }
     case MDL_EXPR_FUNC:
        {
        if (var_ == "startWallTimer" || var_ == "stopWallTimer"
            || var_ == "startProcessTimer" || var_ == "stopProcessTimer"
            || var_ == "startProcessTimer_lwp"
            )
        {
           // this mdl_v_expr::apply_be() is for expressions outside
           // instrumentation blocks.  these timer functions are not
           // supposed to be used here
           return false;
        }
        else
        {
           mdl_var arg0(false);
           if (!dynamic_cast<mdld_expr*>((*args_)[0])->apply_be(arg0)) return false;
           pdstring func_name;
           if (!arg0.get(func_name)) return false;
           if (global_proc)
           {
              BPatch_image *appImage = global_proc->get_dyn_process()->getImage();
              BPatch_function *bpf = mdlFindFunction(func_name.c_str(), appImage);
              if (!bpf) return false; // mdlFindFunction prints warnings

              return (ret.set(bpf));
           }
           else { assert(0); return false; }
        }
     }
  case MDL_EXPR_DOT:
     {
        if (!dynamic_cast<mdld_expr*>(left_)->apply_be(ret)) {
            //fprintf(stderr,"mdl error left of EXPR dot is false\n");
            return false;
        }

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
        if (!mdl_data::cur_mdl_data->env->get(lval, var_)) return false;
        if (!left_ || !dynamic_cast<mdld_expr*>(left_)->apply_be(rval))
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
        if (var_ == pdstring("$cmin") || var_ == pdstring("$cmax"))
           ok_ = true;
        else
           ok_ = mdl_data::cur_mdl_data->env->get (ret, var_);
        return ok_;
     }
  case MDL_EXPR_PREUOP:
     {
        mdl_var lval(false);
        if (!left_ || !dynamic_cast<mdld_expr*>(left_)->apply_be (lval)) return false;
        ok_ = do_operation(ret, lval, u_op_, true);
        return ok_;
     }
  case MDL_EXPR_POSTUOP:
     {
        mdl_var lval(false);
        if (!left_ || !dynamic_cast<mdld_expr*>(left_)->apply_be (lval)) return false;
        ok_ = do_operation(ret, lval, u_op_, false);
        return ok_;
     }
  default:
      {
         return false;
      }
}
return true;
}



bool
mdld_v_expr::mk_list(pdvector<pdstring> &funcs) 
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
      if (!mdl_data::cur_mdl_data->env->get(array, var_)) return false;
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
      if (!dynamic_cast<mdld_expr*>(left_)->mk_list(funcs)) return false;
      if (!dynamic_cast<mdld_expr*>(right_)->mk_list(funcs)) return false;
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


//----------------------------------------------------------------------------
// mdl_icode be functions
//----------------------------------------------------------------------------

// The process is used in generating code which updates the observed cost
// global variable when if statements are called in some cases.  This is to
// account for the cost of the body of the if statement in the observed cost.
bool
mdld_icode::apply_be(BPatch_snippet *&mn, bool mn_initialized,
				          pd_process *proc) 
{
  // a return value of true implies that "mn" has been written to

  if (!expr_)
     return false;

  BPatch_snippet* pred = NULL;
  BPatch_snippet* snip = NULL;

  if (if_expr_) {
    if (!dynamic_cast<mdld_expr*>(if_expr_)->apply_be(pred)) {
       return false;
    }
  }

  if (!dynamic_cast<mdld_expr*>(expr_)->apply_be(snip)) {
    return false;
  }

  BPatch_snippet* code = NULL;
  if (pred)
  {
    code = createIf((BPatch_boolExpr *)pred, snip, proc->get_dyn_process());
    //delete pred;
    //delete snip;
  }
  else
    code = snip;

  if (mn_initialized)
  {
    BPatch_Vector<BPatch_snippet *> seq;
    BPatch_snippet *tmp = new BPatch_snippet(*mn);
    seq.push_back(tmp);
    seq.push_back(code);
    //delete mn;
    mn = new BPatch_sequence(seq);
  }
  else
    mn = code;

  return true;
}


//----------------------------------------------------------------------------
// mdl_list_stmt be functions
//----------------------------------------------------------------------------


bool
mdld_list_stmt::apply_be(instrCodeNode * /*mn*/,
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
  } else if (type_ == MDL_T_LOOP) { 
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
  return (mdl_data::cur_mdl_data->env->add(list_var));
}


bool
mdld_list_stmt::mk_list(pdvector<pdstring> &funcs) {
  if (type_ == MDL_T_PROCEDURE_NAME) {
    unsigned size = elements_->size();
    for (unsigned u = 0; u < size; u++)
      funcs += (*elements_)[u];
  }
  return true;
}


//----------------------------------------------------------------------------
// mdl_for_stmt be functions
//----------------------------------------------------------------------------


bool
mdld_for_stmt::apply_be(instrCodeNode *mn,
				       pdvector<const instrDataNode*>& flags) {
  mdl_data::cur_mdl_data->env->push();
  mdl_data::cur_mdl_data->env->add(index_name_, false);
  mdl_var list_var(false);

  // TODO -- build iterator closure here -- list may be vector or dictionary
  if (!dynamic_cast<mdld_expr*>(list_expr_)->apply_be(list_var))
    return false;
  if (!list_var.is_list())
    return false;

  // TODO
  //  pdvector<int_function*> *vp;
  //  list_var.get(vp);
  list_closure closure(index_name_, list_var);

  // mdl_data::cur_mdl_data->env->set_type(list_var.element_type(), index_name_);
  while (closure.next()) {
    if (!dynamic_cast<mdld_stmt*>(for_body_)->apply_be(mn, flags)) return false;
  }

  mdl_data::cur_mdl_data->env->pop();
  return true;
}



//
// Compute the list of excluded functions recursively. 
// The mk_list functions are similar to apply, but they make a list
// of the excluded functions
// 

bool
mdld_for_stmt::mk_list(pdvector<pdstring> &funcs) {
  mdl_data::cur_mdl_data->env->push();
  mdl_var list_var(false);

  if (!dynamic_cast<mdld_expr*>(list_expr_)->apply_be(list_var)) {
    mdl_data::cur_mdl_data->env->pop();
   return false;
  }
  if (!list_var.is_list()) {
    mdl_data::cur_mdl_data->env->pop();
    return false;
  }

  if (list_var.element_type() == MDL_T_PROCEDURE_NAME) {
    pdvector<functionName *> *funcNames;
    if (!list_var.get(funcNames)) {
      mdl_data::cur_mdl_data->env->pop();
      return false;
    }    
    for (unsigned u = 0; u < funcNames->size(); u++)
       funcs += (*funcNames)[u]->get();
  }

  if (!dynamic_cast<mdld_stmt*>(for_body_)->mk_list(funcs)) {
    mdl_data::cur_mdl_data->env->pop();
    return false;
  }
  mdl_data::cur_mdl_data->env->pop();
  return true;
}


//----------------------------------------------------------------------------
// mdl_if_stmt be functions
//----------------------------------------------------------------------------

bool
mdld_if_stmt::apply_be(instrCodeNode *mn,
                      pdvector<const instrDataNode*>& flags) {
  // An if stmt is comprised of (1) the 'if' expr and (2) the body to
  // execute if true.
  mdl_var res(false);
  if (!dynamic_cast<mdld_expr*>(expr_)->apply_be(res))
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

  return dynamic_cast<mdld_stmt*>(body_)->apply_be(mn, flags);
}

bool mdld_if_stmt::mk_list(pdvector<pdstring> &funcs)
{
  return dynamic_cast<mdld_expr*>(expr_)->mk_list(funcs) && 
            dynamic_cast<mdld_stmt*>(body_)->mk_list(funcs);
}



//----------------------------------------------------------------------------
// mdl_seq_stmt be methods
//----------------------------------------------------------------------------

bool
mdld_seq_stmt::apply_be(instrCodeNode *mn,
				       pdvector<const instrDataNode*>& flags) {
  // a seq_stmt is simply a sequence of statements; apply them all.
  if (!stmts_)
    return true;

  unsigned size = stmts_->size();
  for (unsigned index=0; index<size; index++)
    if (!dynamic_cast<mdld_stmt*>((*stmts_)[index])->apply_be(mn, flags)) // virtual fn call
      return false;

  return true;
}


bool
mdld_seq_stmt::mk_list(pdvector<pdstring> &funcs) {
  for (unsigned u = 0; u < stmts_->size(); u++) {
    if (!dynamic_cast<mdld_stmt*>((*stmts_)[u])->mk_list(funcs))
      return false;
  }
  return true;
}


//----------------------------------------------------------------------------
// mdl_instr_stmt be methods
//----------------------------------------------------------------------------


bool
mdld_instr_stmt::apply_be(instrCodeNode *mn,
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
  if (!dynamic_cast<mdld_expr*>(point_expr_)->apply_be(pointsVar)) { 
      // process the 'point(s)' e.g. "$start.entry"
    return false;
  }

  pdvector<BPatch_point *> points;
  if (pointsVar.type() == MDL_T_LIST_POINT) {
    pdvector<BPatch_point *> *pts;
    if (!pointsVar.get(pts)) return false;
    points = *pts;
  } else if (pointsVar.type() == MDL_T_POINT) {
    BPatch_point *p;
    if (!pointsVar.get(p)) return false; // where the point is located...
    points += p;
  } else {
    return false;
  }

  // Let's generate the code now (we used to calculate it in the loop below,
  // which was a waste since the code is the same for all points).
  BPatch_snippet *code = NULL;
  unsigned size = icode_reqs_->size();
  for (unsigned u=0; u<size; u++) {
    if (!((mdld_icode*)(*icode_reqs_)[u])->apply_be(code, u>0, mn->proc())) {
      // when u is 0, code is un-initialized
      return false;
    }
  }

  // Instantiate all constraints (flags) here (if any)
  // (if !constrained_ then don't do the following)
  if (constrained_) {
     unsigned fsize = inFlags.size();
     for (int fi=fsize-1; fi>=0; fi--) { // any reason why we go backwards?
        instrDataNode *idn = const_cast<instrDataNode *> (inFlags[fi]);

        BPatch_variableExpr *base_var = idn->getVariableExpr();
        if (!base_var) {
           fprintf(stderr, "%s[%d]:  getVariableExpr() failed for intCounter\n",
                   __FILE__, __LINE__);
           return false;
        }

        BPatch_snippet *temp1 = NULL;
        temp1 = getCounter(base_var, global_proc->get_dyn_process()->getImage(),
                           global_proc->multithread_capable());
        if (!temp1) {
          fprintf(stderr, "%s[%d]:  getCounter() failed\n", __FILE__, __LINE__);
        }

        BPatch_snippet *temp2 = code;
        code = createIf((BPatch_boolExpr *)temp1, temp2,
                        mn->proc()->get_dyn_process());
        // delete temp1;
        // delete temp2;
     }
  }

  if (!code) {
    // we are probably defining an empty metric
    //  --haven't seen this get executed...  this makes me a bit queasy
    //  not sure if it needs to be "fixed", left as-is, or removed
    fprintf(stderr, "%s[%d]:  FIXME before null snip\n", __FILE__, __LINE__);
    code = new BPatch_snippet();
  }

  BPatch_snippetOrder corder = (MDL_PREPEND == position_)
                                ? BPatch_firstSnippet
                                : BPatch_lastSnippet;
  BPatch_callWhen cwhen = (MDL_PRE_INSN == where_instr_)
                           ? BPatch_callBefore
                           : BPatch_callAfter;


  // for all of the inst points, insert the predicates and the code itself.
  for (unsigned i = 0; i < points.size(); i++) {
     mn->addInst(points[i], code, cwhen, corder);
     // appends an instReqNode to mn's instRequests; actual 
     // instrumentation only
     // takes place when mn->loadInstrIntoApp() is later called.
  }

  return true;
}

bool
mdld_instr_stmt::mk_list(pdvector<pdstring> &funcs) {
  return dynamic_cast<mdld_expr*>(point_expr_)->mk_list(funcs);
}

//----------------------------------------------------------------------------
// mdl_constraint be methods
//----------------------------------------------------------------------------

#if READY
mdld_constraint::mdl_constraint_be(pdstring id, 
                               pdvector<pdstring> *match_path,
			       pdvector<T_dyninstRPC::mdl_stmt*> *stmts,
			       bool replace, u_int d_type, bool& err)
: id_(id), match_path_(match_path), stmts_(stmts), replace_(replace),
  data_type_(d_type), hierarchy_(0), type_(0) 
{ err = false; }

mdl_constraint_be::~mdl_constraint_be( void )
{
  delete match_path_;
  if (stmts_) {
    for (unsigned u=0; u<stmts_->size(); u++)
      delete (*stmts_)[u];
    delete stmts_;
  }
}
#endif // READY

// Flag constraints need to return a handle to a data request node -- the flag
bool
mdld_constraint::apply_be(instrCodeNode *codeNode,
                          instrDataNode **dataNode,
                          const Hierarchy &resource,
                          pd_process *proc,
                          bool dontInsertData)
{
   assert(dataNode);
   assert(proc);
   assert(codeNode);
   
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
   mdl_data::cur_mdl_data->env->push();

   if (!replace_) {
      // create the counter used as a flag
      mdl_data::cur_mdl_data->env->add(id_, false, MDL_T_DATANODE);
      // "true" means that we are going to create a sampled int counter but
      // we are *not* going to sample it, because it is just a temporary
      // counter - naim 4/22/97
      // By default, the last parameter is false - naim 4/23/97
      
      (*dataNode) = new instrDataNode(proc, MDL_T_COUNTER, dontInsertData,
                                      NULL);
      codeNode->setConstraintDataNode(*dataNode);
      
      // this flag constructs a predicate for the metric -- have to return it
      mdl_data::cur_mdl_data->env->set(*dataNode, id_);
   }

   // put $constraint[X] in the environment
   if (!do_trailing_resources(resource.tokenized(), proc)) {
       mdl_data::cur_mdl_data->env->pop();

        return false;
   }
   
   // Now evaluate the constraint statements
   unsigned size = stmts_->size();
   pdvector<const instrDataNode*> flags;

   for (unsigned u=0; u<size; u++) {
      if (!dynamic_cast<mdld_stmt*>((*stmts_)[u])->apply_be(codeNode, flags)) {
          return false;
      }
   }
   mdl_data::cur_mdl_data->env->pop();

   return true;
}

bool
mdld_constraint::mk_list(pdvector<pdstring> &funcs)
{
  for (unsigned u = 0; u < stmts_->size(); u++)
    dynamic_cast<mdld_stmt*>((*stmts_)[u])->mk_list(funcs);
  return true;
}


//----------------------------------------------------------------------------
// mdl_metric be methods
//----------------------------------------------------------------------------

bool
mdld_metric::apply_be( pdvector<processMetFocusNode *> *createdProcNodes,
			    const Focus &focus, pdvector<pd_process *> procs, 
	                    bool replace_components_if_present, bool enable) {

    //cerr << "\nmdld_metric::apply_be start " << focus.get_loop() << endl;

  mdl_data::cur_mdl_data->env->push();
  mdl_data::cur_mdl_data->env->add(id_, false, MDL_T_DATANODE);
  assert(stmts_);

  const unsigned tc_size = temp_ctr_->size();
  for (unsigned tc=0; tc<tc_size; tc++) {
    mdl_data::cur_mdl_data->env->add((*temp_ctr_)[tc], false, MDL_T_DATANODE);
  }

  static pdstring machine;
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

  if (!instProcess.size()) {
    return false;
  }

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

  //cerr << "mdld_metric::apply_be picked matched constraints" << endl;

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
     ((mdld_constraint*)repl_cons)->mk_list(global_excluded_funcs);
  } else {
     for (unsigned u1 = 0; u1 < flag_cons.size(); u1++) {
        ((mdld_constraint*)flag_cons[u1])->mk_list(global_excluded_funcs);
     }
     for (unsigned u2 = 0; u2 < stmts_->size(); u2++) {
        dynamic_cast<mdld_stmt*>((*stmts_)[u2])->mk_list(global_excluded_funcs);
     }
  }
  
  // cerr << "Metric: " << name_ << endl;
  // for (unsigned x1 = 0; x1 < global_excluded_funcs.size(); x1++)
    // cerr << "  " << global_excluded_funcs[x1] << endl;
  
  //////////


  // build the instrumentation request
  bool dontInsertData;
  if (enable) dontInsertData = false;
  else dontInsertData = true;

  pdvector<processMetFocusNode*> procParts; // one per process

  if (type_ == MDL_T_HW_COUNTER || type_ == MDL_T_HW_TIMER) {
#ifdef PAPI
    if (!isPapiInitialized()) {
        pdstring msg = pdstring("PAPI hardware events are unavailable");
        mdl_data::cur_mdl_data->env->appendErrorString( msg );
        return false;
    }
    else if (!papiMgr::isHwStrValid(hwcntr_)) {
        pdstring msg = pdstring(hwcntr_ + " PAPI hardware event is invalid");
        mdl_data::cur_mdl_data->env->appendErrorString( msg );
        return false;
    }
#else
    pdstring msg = pdstring("PAPI hardware events are not available");
    mdl_data::cur_mdl_data->env->appendErrorString( msg );
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

  mdl_data::cur_mdl_data->env->pop();

  ////////////////////
  global_excluded_funcs.resize(0);
  return true;
}


//----------------------------------------------------------------------------
// utility functions
//----------------------------------------------------------------------------

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
  unsigned c_size = mdl_data::cur_mdl_data->all_constraints.size();
  for (unsigned cs=0; cs<c_size; cs++) 
    if (mdl_data::cur_mdl_data->all_constraints[cs]->id_ == match_me->id_) {
      match_me = mdl_data::cur_mdl_data->all_constraints[cs];
      if (hier->focus_matches(*(mdl_data::cur_mdl_data->all_constraints[cs]->match_path_))) {
          if (mdl_data::cur_mdl_data->all_constraints[cs]->data_type_ == MDL_T_NONE) {
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
   BPatch_Vector<BPatch_function *> *start_func_buf = new BPatch_Vector<BPatch_function*>;
   pd_process *proc = codeNode->proc();
   BPatch_function *bpf = NULL;
   BPatch_Vector<BPatch_function *> bpfv;

   pdvector<pdstring> start_funcs;
#if defined(rs6000_ibm_aix4_1)
   start_funcs.push_back("_pthread_body");
#elif defined(sparc_sun_solaris2_4)
   start_funcs.push_back("_thread_start");
   start_funcs.push_back("_thr_setup");
#elif defined(i386_unknown_linux2_0)
   start_funcs.push_back("start_thread");
#endif

   if (proc->multithread_ready()) {
       for (unsigned start_iter = 0; 
            start_iter < start_funcs.size();
            start_iter++) {
           if (bpfv.size()) bpfv.clear();
           proc->findAllFuncsByName(start_funcs[start_iter], bpfv);
           for (unsigned i = 0; i < bpfv.size(); i++) {
               (*start_func_buf).push_back(bpfv[i]);
           }
       }
       if ((*start_func_buf).size() == 0) {
           cerr << "Warning: no internal thread start function found"
                << ", some data may be missed." << endl;
       }       
   }   

   if (NULL != (bpf = proc->getMainFunction())) {
       (*start_func_buf).push_back(bpf);      
   }
   else {
       cerr << __FILE__ << __LINE__ << "getMainFunction() returned NULL!"<<endl;
   }
   if (start_func_buf->size()) {
       pdstring vname = "$start";
       // change this to MDL_T_LIST_PROCEDURE
       mdl_data::cur_mdl_data->env->add(vname, false, MDL_T_PROCEDURE);
       mdl_data::cur_mdl_data->env->set(start_func_buf, vname);
   }
   return true;
}

// update the interpreter environment for this processor
// Variable updated: $procedures, $modules, $exit, $start
static bool update_environment(pd_process *proc) {
   // for cases when libc is dynamically linked, the exit symbol is not
   // correct
   pdstring vname = "$exit";
   BPatch_Vector<BPatch_function *> *exit_func_buf = new BPatch_Vector<BPatch_function*>;
   BPatch_Vector<BPatch_function *> bpfv;
   BPatch_function *bpf = NULL;
   
   proc->findAllFuncsByName(pdstring(EXIT_NAME), *exit_func_buf); 
   if (exit_func_buf->size() > 1) {
#ifdef FIXME_AFTER_4
      pdstring msg = pdstring("WARNING:  found ") + pdstring(exit_func_buf->size()) +
                   pdstring(" records of function '") + pdstring(EXIT_NAME) +
                   pdstring("'") + pdstring(".  Using the first.(2)");
      //showErrorCallback(95, msg);
      cerr << msg << endl;      
#endif
      
      // findAllFuncs found more than one function, clear all but one
      bpf = (*exit_func_buf)[0];
      exit_func_buf->clear();
      exit_func_buf->push_back(bpf);
   }
   
   bpf = NULL;
#if !defined(i386_unknown_nt4_0)
   if (!proc->findAllFuncsByName("pthread_exit", bpfv)) {
       // Not an error... what about ST programs :)
       //string msg = pdstring("unable to find procedure '") + 
       //pdstring("pthread_exit") + pdstring("'");
       //showErrorCallback(95, msg);
   }
   else {
      if (bpfv.size() > 1) {
#ifdef FIXME_AFTER_4
         pdstring msg = pdstring("WARNING:  found ") + pdstring(bpfv.size()) +
                      pdstring(" records of function '") +pdstring("pthread_exit")+
                      pdstring("'") + pdstring(".  Using the first.");
         //showErrorCallback(95, msg);
         cerr << msg << endl;;      
#endif
      }
      bpf = bpfv[0];
   }
   
   if(bpf)  (*exit_func_buf).push_back(bpf);
   bpfv.clear();
   bpf = NULL;
   
   if (!proc->findAllFuncsByName("thr_exit", bpfv)) {
#ifdef FIXME_AFTER_4
      pdstring msg = pdstring("unable to find procedure '") + pdstring("thr_exit") +
                   pdstring("'");
      //showErrorCallback(95, msg);
      cerr << msg << endl;;      
#endif
   }
   else {
      if (bpfv.size() > 1) {
#ifdef FIXME_AFTER_4
         pdstring msg = pdstring("WARNING:  found ") + pdstring(bpfv.size()) +
                      pdstring(" records of function '") + pdstring("thr_exit") +
                      pdstring("'") + pdstring(".  Using the first.");
         //showErrorCallback(95, msg);
         cerr << msg << endl;      
#endif
      }
      bpf = bpfv[0];
   }
   
   if(bpf) (*exit_func_buf).push_back(bpf);
#else
    // findAllFuncsByName works on pretty names, but EXIT_NAME is 
    // mangled on Windows
    proc->findAllFuncsByName( "exit", *exit_func_buf);
    if (exit_func_buf->size() > 1) {
      // findAllFuncs found more than one function, clear all but one
      bpf = (*exit_func_buf)[0];
      exit_func_buf->clear();
      exit_func_buf->push_back(bpf);
    }

    proc->findAllFuncsByName( "ExitProcess", *exit_func_buf);
    if (exit_func_buf->size() > 1) {
      // findAllFuncs found more than one function, clear all but one
      bpf = (*exit_func_buf)[0];
      exit_func_buf->clear();
      exit_func_buf->push_back(bpf);
    }

    proc->findAllFuncsByName( "ExitThread", *exit_func_buf);
    if (exit_func_buf->size() > 1) {
      // findAllFuncs found more than one function, clear all but one
      bpf = (*exit_func_buf)[0];
      exit_func_buf->clear();
      exit_func_buf->push_back(bpf);
    }

    bpf = NULL;
#endif // !defined(i386_unknown_nt4_0)

   if ((*exit_func_buf).size() > 0) { 
      mdl_data::cur_mdl_data->env->add(vname, false, MDL_T_PROCEDURE);
      mdl_data::cur_mdl_data->env->set(exit_func_buf, vname);
   }

   vname = "$procedures";
   mdl_data::cur_mdl_data->env->add(vname, false, MDL_T_LIST_PROCEDURE);
   // only get the functions that are not excluded by exclude_lib or 
   // exclude_func
   mdl_data::cur_mdl_data->env->set(proc->getIncludedFunctions(), vname);
   
   vname = "$modules";
   mdl_data::cur_mdl_data->env->add(vname, false, MDL_T_LIST_MODULE);
   // only get functions that are not excluded by exclude_lib or exclude_func
   pdvector<BPatch_module *> incmods;
   mdl_data::cur_mdl_data->env->set(proc->getIncludedModules(&incmods), vname);

  return true;
}

// allocate data and generate code for all threads
bool setup_sampled_code_node(const processMetFocusNode* procNode,
                             instrCodeNode* codeNode, pd_process *proc,
                             const pdstring &id, unsigned type,
                             T_dyninstRPC::mdl_constraint *repl_cons,
                             pdvector<T_dyninstRPC::mdl_stmt*> *stmts,
                             const pdvector<pdstring> &temp_ctr, 
                             const Hierarchy &repl_focus_data,
                             bool dontInsertData)
{
   update_environment_start_point(codeNode);
   
   instrDataNode *sampledDataNode = 
      new instrDataNode(proc, type, dontInsertData, codeNode->getHwEvent());

   //fprintf(stderr, "'nCode Nnode %x \tData Node %x\n", (&codeNode), (&sampledDataNode));

   codeNode->setSampledDataNode(sampledDataNode);
   mdl_data::cur_mdl_data->env->set(sampledDataNode, id);

   // Create the temporary counters 
   for (unsigned tc=0; tc < temp_ctr.size(); tc++) {
      instrDataNode *tempCtrDataNode = 
         new instrDataNode(proc, MDL_T_COUNTER, dontInsertData);
      codeNode->addTempCtrDataNode(tempCtrDataNode);
      mdl_data::cur_mdl_data->env->set(tempCtrDataNode, temp_ctr[tc]);
   }
   
   // create the ASTs for the code
   if(repl_cons!=NULL) {
      // mdl_constraint::apply_be()

       //XXX apply_be take a focus

      instrDataNode *notAssignedToDataNode;
      if (!((mdld_constraint*)repl_cons)->apply_be(codeNode,
                                &notAssignedToDataNode, 
                                repl_focus_data, proc, dontInsertData)) {
         return false;
      }
   } else {
      pdvector<const instrDataNode*> flagNodes = procNode->getFlagDataNodes();
      unsigned size = stmts->size();
      for (unsigned u=0; u<size; u++) {
         // virtual fn call depending on stmt type
         if (!dynamic_cast<mdld_stmt*>((*stmts)[u])->apply_be(codeNode, flagNodes)) {
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
   // The following calls mdl_constraint::apply_be():
   if (!((mdld_constraint*)flag_con)->apply_be(codeNode,
                                                &consDataNode,
                                                flag_focus_data,
			                                    proc, dontInsertData)) {
      return false;
   }
   return true;
}

// returns true if success, false if failure
bool createCodeAndDataNodes(processMetFocusNode **procNode_arg,
		     const pdstring &id, const pdstring &name, 
		     const Focus &no_thr_focus,
		     unsigned type, 
                     pdstring& hw_cntr_str,
		     pdvector<T_dyninstRPC::mdl_constraint*> &flag_cons,
		     T_dyninstRPC::mdl_constraint *repl_cons,
		     pdvector<T_dyninstRPC::mdl_stmt*> *stmts,
		     pdvector<const Hierarchy *> flags_focus_data, 
		     const Hierarchy &repl_focus_data,
		     const pdvector<pdstring> &temp_ctr, 
		     bool /*replace_component*/)
{
   processMetFocusNode *procNode = (*procNode_arg);
   pd_process *proc = procNode->proc();
   bool dontInsertData = procNode->dontInsertData();
   
	/* Are normal constraints disallowed if the metric specifies "replacement" constraints? */
	if( repl_cons == NULL ) {
		for( unsigned int flag = 0; flag < flag_cons.size(); flag++ ) {
			pdstring constraintName( flag_cons[flag]->id() );
			
			/* We can share flags if and only if three conditions hold:
			   [1] They're for the same focus.
			   [2] They instrument the same points.
			   [3] The code they insert at those points is the same.
			   
			   We can check [1] by comparing focus names.
			   Because every constraint name is unique and invariant, we can check [2]
			     by comparing constraint names.
			   To check [3], we need a list of instPoints.  setup_constraint_code_node()
			     will generate a list of instReqNodes.
			     
			   Since newInstrCodeNode() checks [1] and [2] for us, we need to check [3].
			   We'll cheat.  (See more below.) */
			   
			instrCodeNode * cachedConstraintCodeNode =
				instrCodeNode::newInstrCodeNode( constraintName, no_thr_focus, proc, dontInsertData );
				
			if( cachedConstraintCodeNode->numDataNodes() <= 0 ) {
				/* Then we haven't inserted instrumentation for this constraint/focus pair before. */
				bool okay = setup_constraint_code_node(	cachedConstraintCodeNode, proc, 
														flag_cons[flag], * flags_focus_data[flag],
														dontInsertData );
				if( ! okay ) {
					delete cachedConstraintCodeNode;
					return false;
					}
								
				/* Since we haven't seen it before, just add it and move on. */
				// /* DEBUG */ fprintf( stderr, "%s[%d]: uncached constraint/focus pair (%s, %s) detected.\n", __FILE__, __LINE__, constraintName.c_str(), no_thr_focus.getName().c_str() );
				procNode->addConstraintCodeNode( cachedConstraintCodeNode );
				continue;
				} /* end if the node was not cached. */
				
			/* We've instrumented this constraint/focus pair before.  See if we did it with the same
			   set of instrumentation (set of excluded functions). */
			instrCodeNode * uncachedConstraintCodeNode = 
				instrCodeNode::newInstrCodeNode( constraintName + "@" + name, no_thr_focus, proc, dontInsertData );
				
			if( uncachedConstraintCodeNode->numDataNodes() <= 0 ) {
				/* Then we haven't inserted instrumentation for this constraint/focus/metric trio before. */
				bool okay = setup_constraint_code_node(	uncachedConstraintCodeNode, proc, 
														flag_cons[flag], * flags_focus_data[flag],
														dontInsertData );
				if( ! okay ) {
					delete uncachedConstraintCodeNode;
					return false;
					}
				} /* end if cachedConstraintCodeNode is incomplete. */
			else {
				/* Then we've inserted instrumentation for this constraint/focus/metric trio before.
				   I'm pretty sure this should be caught in the front-end. */
				// /* DEBUG */ fprintf( stderr, "%s[%d]: duplicate constraint/focus/metric (%s, %s, %s) detected.\n", __FILE__, __LINE__, constraintName.c_str(), no_thr_focus.getName().c_str(), name.c_str() );
				procNode->addConstraintCodeNode( uncachedConstraintCodeNode );
				continue;
				}
				
			/* Now we can compare the instrument request lists. */
			bool theSame = true;
			
			pdvector< instReqNode * > cachedInstrumentationRequests = cachedConstraintCodeNode->getInstRequests();
			pdvector< instReqNode * > uncachedInstrumentationRequests = uncachedConstraintCodeNode->getInstRequests();
			
			// /* DEBUG */ fprintf( stderr, "%s[%d]: %d cached addresses vs %d uncached addresses.\n", __FILE__, __LINE__, cachedInstrumentationRequests.size(), uncachedInstrumentationRequests.size() );
			if( cachedInstrumentationRequests.size() == uncachedInstrumentationRequests.size() ) {
				for( unsigned int i = 0; i < cachedInstrumentationRequests.size(); i++ ) {
					instReqNode * cachedIRN = cachedInstrumentationRequests[i];
					instReqNode * uncachedIRN = uncachedInstrumentationRequests[i];
					
					// /* DEBUG */ fprintf( stderr, "%s[%d]: address 0x%lx (cached) vs 0x%lx (uncached).\n", __FILE__, __LINE__, cachedIRN->Point()->getAddress(), uncachedIRN->Point()->getAddress() );
					if( cachedIRN->Point()->getAddress() != uncachedIRN->Point()->getAddress() ) {
						theSame = false;
						}
					} /* end iteration of the requests in the vector */
				} /* end if the size is the same */
			else { theSame = false; }
			
			if( theSame ) {
				// /* DEBUG */ fprintf( stderr, "%s[%d]: decided that metric %s can share constraint/focus pair (%s, %s).\n", __FILE__, __LINE__, name.c_str(), constraintName.c_str(), no_thr_focus.getName().c_str() );			
				procNode->addConstraintCodeNode( cachedConstraintCodeNode );
				delete uncachedConstraintCodeNode; 
				} else {
				// /* DEBUG */ fprintf( stderr, "%s[%d]: decided that metric %s could NOT share constraint/focus pair (%s, %s).\n", __FILE__, __LINE__, name.c_str(), constraintName.c_str(), no_thr_focus.getName().c_str() );			
				procNode->addConstraintCodeNode( uncachedConstraintCodeNode );
				}				
			} /* end iteration over normal constraints. */
		} /* end there are no replacement-type constraints. */
               
   instrCodeNode *metCodeNode = 
      instrCodeNode::newInstrCodeNode(name, no_thr_focus, proc,
                                      dontInsertData, hw_cntr_str);
   
   /* if hw_cntr_str is no good, metCodeNode is NULL */
   if (metCodeNode == NULL) {
      return false;
   }
    
   //cerr << "createCodeAndDataNodes: created newInstCodeNode " << 
   // hw_cntr_str << endl;
    
   bool metCodeNodeComplete = (metCodeNode->numDataNodes() > 0);

   if(! metCodeNodeComplete) {
       //cerr << "createCodeAndDataNodes: setup_sampled_code_node" << endl;
       
       // Create the data objects (timers/counters) and create the
       // astNodes which will be used to generate the instrumentation
       if(! setup_sampled_code_node(procNode, metCodeNode, proc, id, type, 
                                    repl_cons, stmts, temp_ctr,
                                    repl_focus_data, dontInsertData)) {
           delete metCodeNode;
           return false;
       }
       
   } else {
       //XXX sampled data node already set assert breaks
       //cerr << "Met code node already there, reuse it." << endl;
   }
   
   procNode->setMetricVarCodeNode(metCodeNode);

   return true;
}

bool createThreadNodes(processMetFocusNode **procNode_arg,
		       const pdstring &metname, 
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
            pdstring start_func_name;
            start_func_name = thr->get_start_func() ? 
               thr->get_start_func()->prettyName()
               : pdstring("no start func!");
            
            pdstring thrName = pdstring("thr_") + pdstring(thr->get_tid()) + "{" + 
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
apply_to_process(pd_process *proc, pdstring& id, pdstring& name,
                 const Focus &focus, unsigned agg_op, unsigned type,
                 pdstring& hw_cntr_str,
                 pdvector<T_dyninstRPC::mdl_constraint*>& flag_cons,
                 T_dyninstRPC::mdl_constraint *repl_cons,
                 pdvector<T_dyninstRPC::mdl_stmt*> *stmts,
                 pdvector<const Hierarchy *> &flags_focus_data,
                 const Hierarchy &repl_focus_data,
                 const pdvector<pdstring> &temp_ctr,
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
   no_thr_focus.set_thread(pdstring(""));
   // no_thr_focus, has the thr info stripped


   processMetFocusNode *procNode = 
      processMetFocusNode::newProcessMetFocusNode(proc, name, full_focus, 
					  aggregateOp(agg_op), dontInsertData);

   //cerr << "apply_to_process: createCodeAndDataNodes START" << endl;

   bool ret = createCodeAndDataNodes(&procNode, id, name, no_thr_focus, 
                                     type, hw_cntr_str, flag_cons, repl_cons,
                                     stmts, flags_focus_data, repl_focus_data,
                                     temp_ctr, replace_component);


   //cerr << "apply_to_process: createCodeAndDataNodes END" << endl;

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
				  pdstring& id, pdstring& name,
				  const Focus& focus,
				  unsigned& agg_op,
				  unsigned& type,
                                  pdstring& hw_cntr_str, 
			      pdvector<T_dyninstRPC::mdl_constraint*>& flag_cons,
				  T_dyninstRPC::mdl_constraint *repl_cons,
				  pdvector<T_dyninstRPC::mdl_stmt*> *stmts,
				  pdvector<const Hierarchy *> &flags_focus_data,
				  const Hierarchy &repl_focus_data,
				  const pdvector<pdstring> &temp_ctr,
				  bool replace_components_if_present,
				  bool dontInsertData) {
   for(unsigned p=0; p<instProcess.size(); p++) {
      pd_process *proc = instProcess[p];
      assert(proc);
      global_proc = proc;     // TODO -- global
      
     // skip exited processes 
     if (proc->isTerminated()) 
       continue;
      
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



static bool do_trailing_resources(const pdvector<pdstring>& resource_,
				  pd_process *proc)
{
    pdvector<pdstring>  resPath;

   for (unsigned i = 0; i < resource_.size(); i++) {
      pdstring consvar = pdstring("$constraint") + 
          pdstring(resource_.size()-i-1);

      pdstring trailingRes = resource_[i];
      resPath += resource_[i];
      assert(resPath.size() == (i+1));
      
      resource *r = resource::findResource(resPath);
      if (!r) assert(0);
      
      switch (r->mdlType()) {
        case MDL_T_INT: {
           const char* p = trailingRes.c_str();
           char*       q = NULL;
           int         val = (int) strtol(p, &q, 0);
           
           if (p == q) {
              pdstring msg = pdstring("unable to convert resource '") +
                           trailingRes + pdstring("' to integer.");
              mdl_data::cur_mdl_data->env->appendErrorString( msg );
              return(false);
           }
           mdl_data::cur_mdl_data->env->add(consvar, false, MDL_T_INT);
           mdl_data::cur_mdl_data->env->set(val, consvar);
           break;
        }
        case MDL_T_STRING:
           mdl_data::cur_mdl_data->env->add(consvar, false, MDL_T_STRING);
           mdl_data::cur_mdl_data->env->set(trailingRes, consvar);
           break;
        case MDL_T_PROCEDURE: {
           // find the resource corresponding to this function's module 
           pdvector<pdstring> m_vec;
           for(u_int j=0; j < resPath.size()-1; j++){
              m_vec += resPath[j];
           }
           assert(m_vec.size());
           assert(m_vec.size() == (resPath.size()-1));
           resource *m_resource = resource::findResource(m_vec);
           if (!m_resource) {
              return false;
           }
           
           BPatch_Vector<BPatch_function *> *func_buf = 
               new BPatch_Vector<BPatch_function*>;

           if ( !proc->findAllFuncsByName(r, m_resource, *func_buf) ) {
              const pdvector<pdstring> &f_names = r->names();
              const pdvector<pdstring> &m_names = m_resource->names();
              pdstring func_name = f_names[f_names.size() -1]; 
              pdstring mod_name = m_names[m_names.size() -1]; 

// 	      cerr << "Missing function, " << func_buf->size() << endl;
// 	      for (int i = 0; i < m_names.size(); i++)
// 		  cerr << m_names[i] << " ";
// 	      cerr << endl;
// 	      for (int i = 0; i < f_names.size(); i++)
// 		  cerr << f_names[i] << " ";
// 	      cerr << endl;
              
              pdstring msg = pdstring("For requested metric-focus, ") +
                           pdstring("unable to find  function ") +
                           func_name;
              mdl_data::cur_mdl_data->env->appendErrorString( msg );
              return false;
           }
           
           mdl_data::cur_mdl_data->env->add(consvar, false, MDL_T_PROCEDURE);
           mdl_data::cur_mdl_data->env->set(func_buf, consvar);
           break;
        }
        case MDL_T_LOOP: {
           // find the resource corresponding to the loop's function
           pdvector<pdstring> rvec;
           for (unsigned k=0; k < resPath.size()-1; k++)
              rvec += resPath[k];

           resource *fresource = resource::findResource(rvec);

           if (!fresource) {
               return false;
           }

           // find the resource corresponding to the loop's module
           rvec.pop_back();
           resource *mresource = resource::findResource(rvec);

           if (!mresource) {
               return false;
           }
           
           // find func by func and mod resource
           BPatch_Vector<BPatch_function *> *func_buf = 
               new BPatch_Vector<BPatch_function*>;

           if (!proc->findAllFuncsByName(fresource, mresource, *func_buf)) {
              return false;
           }

           //fprintf(stderr," mod %s\n",mresource->full_name().c_str());
           //fprintf(stderr,"func %s\n",fresource->full_name().c_str());

           //XXX find all funcs? should we only find a single function
           // and a single loop here, i.e. we don't need a buf
           pdstring loop_name = resPath[resPath.size()-1];
           BPatch_basicBlockLoop *loop = NULL;

           //fprintf(stderr,"loop %s\n",loop_name.c_str());
           for (unsigned j=0; j<(*func_buf).size(); j++) {
               BPatch_function *f = (*func_buf)[j];
               //char tmp[80];
               //f->getName(tmp,80);
               //fprintf(stderr,"func %s\n",tmp);
               BPatch_flowGraph *fg = f->getCFG();
               loop = fg->findLoop(loop_name.c_str());
               if (loop) break;
           }

           assert(loop != NULL);

           pdvector<BPatch_basicBlockLoop *> *loop_buf = 
               new pdvector<BPatch_basicBlockLoop*>;
           loop_buf->push_back(loop);

           mdl_data::cur_mdl_data->env->add(consvar, false, MDL_T_LOOP);
           mdl_data::cur_mdl_data->env->set(loop_buf, consvar);

           break;
        }
        case MDL_T_MODULE: {
           BPatch_module *mod = proc->findModule(trailingRes, true);
           if (!mod) {
              pdstring msg = pdstring("For requested metric-focus, ") + 
                 pdstring("unable to find module ") + trailingRes;
              mdl_data::cur_mdl_data->env->appendErrorString( msg );
              
              /*
                pd_image *img = proc->getImage();
                pdvector<pdmodule *> mods = img->getExcludedModules();
                for(unsigned j=0; j<mods.size(); j++) {
                cerr << "  j: " << j << ", filenm: " << mods[j]->fileName()
                << ", fullnm: " << mods[j]->fullName() << "\n";
                }
              */
              return(false);
           }
           mdl_data::cur_mdl_data->env->add(consvar, false, MDL_T_MODULE);
           mdl_data::cur_mdl_data->env->set(mod, consvar);
           break;
        }
        case MDL_T_MEMORY:
           break ;
        default:
           assert(0);
           break;
      }
   }
   return true;
}


bool mdl_can_do(const pdstring &met_name) {
  // NOTE: We can do better if there's a dictionary of <metric-name> to <anything>
  unsigned size = mdl_data::cur_mdl_data->all_metrics.size();
  for (unsigned u=0; u<size; u++)
    if (mdl_data::cur_mdl_data->all_metrics[u]->name_ == met_name)
      return true;

  /*
  metric_cerr << endl << " all metric names: " << endl;
  for (unsigned u=0; u<size; u++)
    metric_cerr << "  metric name [" << u << "] = [" 
		<< mdl_data::cur_mdl_data->all_metrics[u]->name_ << "] " << endl;
  */

  return false;
}

bool mdl_do(pdvector<processMetFocusNode *> *createdProcNodes, 
	    const Focus& focus, const pdstring &met_name,
	    const pdvector<pd_process *> &procs,
	    bool replace_components_if_present, bool enable, 
	    aggregateOp *aggOpToUse) {

   currentMetric = met_name;
   unsigned size = mdl_data::cur_mdl_data->all_metrics.size();
   // NOTE: We can do better if there's a dictionary of <metric-name> to
   // <metric>!
   
   for (unsigned u=0; u<size; u++) {
      T_dyninstRPC::mdl_metric *curMetric = mdl_data::cur_mdl_data->all_metrics[u];
      if (curMetric->name_ == met_name) {
         // calls mdl_metric::apply_be()

          //cerr << "\nmdl_do calling apply_be" << endl;

         bool ret = ((mdld_metric*)curMetric)->apply_be(createdProcNodes,
                                                        focus,
                                                        procs,
                                                replace_components_if_present,
                                                enable);

         (*aggOpToUse) = aggregateOp(curMetric->agg_op_);

         return ret;
      }
   }

   return false;
}

machineMetFocusNode *makeMachineMetFocusNode(int mid, const Focus& focus, 
			    const pdstring &met_name, 
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
			    const pdstring &met_name, pd_process *proc,
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

bool mdl_init_be(pdstring& flavor) { 

  daemon_flavor = flavor;

  mdl_data::cur_mdl_data->env->push();
  // These are pushed on per-process
  // mdl_data::cur_mdl_data->env->add("$procedures", false, MDL_T_LIST_PROCEDURE);
  // mdl_data::cur_mdl_data->env->add("$modules", false, MDL_T_LIST_MODULE);

  pdstring vname = "$machine";
  mdl_data::cur_mdl_data->env->add(vname, false, MDL_T_STRING);
  pdstring nodename = getNetworkName();
  mdl_data::cur_mdl_data->env->set(nodename, vname);

  /* Are these entered by hand at the new scope ? */
  /* $arg, $return */

  pdvector<mdl_type_desc> field_list;
  mdl_type_desc desc;
  desc.end_allowed = false;     // random initialization

  desc.name = "name"; desc.type = MDL_T_STRING; field_list += desc;
  desc.name = "calls"; desc.type = MDL_T_LIST_POINT; field_list += desc;
  desc.name = "entry"; desc.type = MDL_T_POINT; field_list += desc;
  desc.name = "return"; desc.type = MDL_T_POINT; field_list += desc;
  mdl_data::cur_mdl_data->fields[MDL_T_PROCEDURE] = field_list;
  field_list.resize(0);

  desc.name = "enter"; desc.type = MDL_T_POINT; field_list += desc;
  desc.name = "exit"; desc.type = MDL_T_POINT; field_list += desc;
  desc.name = "start_iter"; desc.type = MDL_T_POINT; field_list += desc;
  desc.name = "end_iter"; desc.type = MDL_T_POINT; field_list += desc;
  mdl_data::cur_mdl_data->fields[MDL_T_LOOP] = field_list;
  field_list.resize(0);
  
  desc.name = "name"; desc.type = MDL_T_STRING; field_list += desc;
  desc.name = "funcs"; desc.type = MDL_T_LIST_PROCEDURE; field_list += desc;
  mdl_data::cur_mdl_data->fields[MDL_T_MODULE] = field_list;
  field_list.resize(0);

  return true;
}

void pdRPC::send_metrics(pdvector<T_dyninstRPC::mdl_metric*>* var_0) {
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
	unsigned size=mdl_data::cur_mdl_data->all_metrics.size();
	for (unsigned u=0; u<size; u++) 
	  if (mdl_data::cur_mdl_data->all_metrics[u]->id_ == (*var_0)[v]->id_) {
	    delete mdl_data::cur_mdl_data->all_metrics[u];
	    mdl_data::cur_mdl_data->all_metrics[u] = (*var_0)[v];
	    found = true;
	  }
	if (!found) {
	  T_dyninstRPC::mdl_metric *m = (*var_0)[v];
	  mdl_data::cur_mdl_data->all_metrics += m;
	}
      }
    }
  } else {
     fprintf(stderr, "no metric defined\n");
     fflush(stderr);
  }
}

void pdRPC::send_constraints(pdvector<T_dyninstRPC::mdl_constraint*> *cv) {

  mdl_cons = true;
  if (cv) {
      unsigned var_size = cv->size();
      for (unsigned v=0; v<var_size; v++) {
          bool found = false;
          for (unsigned u=0; u<mdl_data::cur_mdl_data->all_constraints.size(); u++) 
              if (mdl_data::cur_mdl_data->all_constraints[u]->id_ == (*cv)[v]->id_) {
                  delete mdl_data::cur_mdl_data->all_constraints[u];
                  mdl_data::cur_mdl_data->all_constraints[u] = (*cv)[v];
                  found = true;
              }
          if (!found) {
              mdl_data::cur_mdl_data->all_constraints += (*cv)[v];
              // cout << *(*cv)[v] << endl;
          }
      }
  }
}


// TODO -- are these executed immediately ?
void pdRPC::send_stmts(pdvector<T_dyninstRPC::mdl_stmt*> *vs) {
  mdl_stmt = true;
  if (vs) {
    // ofstream of("other_out", (been_here ? ios::app : std::ios::out));
    // been_here = true;
    // of << "SEND_STMTS\n";
    // unsigned size = vs->size();
    // for (unsigned u=0; u<size; u++) 
    // (*vs)[u]->print(of);
    mdl_data::cur_mdl_data->stmts += *vs;

    // TODO -- handle errors here
    // TODO -- apply these statements without a metric definition node ?
    unsigned s_size = vs->size();
    pdvector<const instrDataNode*> flags;

    // Application may fail if the list flavor is different than the flavor
    // of this daemon

    for (unsigned s=0; s<s_size; s++) {
      mdld_stmt* curStmt = dynamic_cast<mdld_stmt*>((*vs)[s]);
      assert( curStmt != NULL );

      if( !curStmt->apply_be( NULL, flags ) )
      {
          // (*vs)[s]->print(cout);
          // cout << endl;
      }
    }
  }
}

// recieves the list of shared libraries to exclude 
void pdRPC::send_libs(pdvector<pdstring> *libs) {

    mdl_libs = true;
    //metric_cerr << "void pdRPC::send_libs(pdvector<pdstring> *libs) called" << endl;
    for(u_int i=0; i < libs->size(); i++){
	mdl_data::cur_mdl_data->lib_constraints += (*libs)[i]; 
	//metric_cerr << " send_libs : adding " << (*libs)[i] << " to paradynd set of mdl_data::cur_mdl_data->lib_constraints" << endl;
    }

}

// recieves notification that there are no excluded libraries 
void pdRPC::send_no_libs() {
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
        pdstring v1, v2;
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
   unsigned index = 0;
   unsigned max = types.size();

   while (index < max) 
   {
      unsigned current_type = types[index++];
      unsigned next_field = types[index++];
      switch (current_type) 
      {
        case MDL_T_LOOP: {
            pdvector<BPatch_basicBlockLoop *> *loop_buf_ptr;
            if (!ret.get(loop_buf_ptr)) {
                //XXX
                fprintf(stderr,"no loop buf\n");
                return false;
            }
            
           pdvector<BPatch_point *> *inst_point_buf = 
               new pdvector<BPatch_point*>;            

            for (unsigned i=0; i<(*loop_buf_ptr).size(); i++) {
                BPatch_basicBlockLoop *loop = (*loop_buf_ptr)[i];
                
                BPatch_flowGraph *fg = loop->getFlowGraph();
                BPatch_Vector<BPatch_point*> *pts;
                
                switch (next_field) {
                case 0: { // .enter
                    pts = fg->findLoopInstPoints(BPatch_locLoopEntry,loop);

		    //cerr << (fg->getFunction())->prettyName().c_str() 
		    // << " loop enter: ";
		    //for (unsigned i = 0; i < pts->size(); i++)
			//fprintf(stderr,"0x%x ",(*pts)[i]->getAddress());
		    //cerr << endl;

                    break;
                }
                case 1: {  // .exit
                    pts = fg->findLoopInstPoints(BPatch_locLoopExit,loop);

		    //cerr << (fg->getFunction())->prettyName().c_str() 
		    //			 << " loop exit: ";
		    //for (unsigned i = 0; i < pts->size(); i++)
		    //	fprintf(stderr,"0x%x ",(*pts)[i]->getAddress());

		    //cerr << endl;

                    break;
                }
                case 2: {  // .start_iter
                    pts = fg->findLoopInstPoints(BPatch_locLoopStartIter,loop);
                    break;
                }
                case 3: { // .end_iter
                    pts = fg->findLoopInstPoints(BPatch_locLoopEndIter,loop);
                    break;
                }
                default: {
                    assert(0);
                    break;
                }
                }
                
                for (unsigned j = 0; j < pts->size(); j++) {
                    (*inst_point_buf).push_back(pts->operator[](j));
                }
            }
            
            if (!ret.set(inst_point_buf)) 
                return false;
            break;
        }
        case MDL_T_PROCEDURE_NAME:
        case MDL_T_PROCEDURE: {
           BPatch_Vector<BPatch_function *> *func_buf_ptr;
           if (!ret.get(func_buf_ptr)) return false;
           pdvector<BPatch_point *> *inst_point_buf = new pdvector<BPatch_point*>;
                          
           switch (next_field) {
             case 0:  // .name
             {
                pdvector<pdstring> *nameBuf = new pdvector<pdstring>;
                for(unsigned i=0; i<(*func_buf_ptr).size(); i++) {
                   BPatch_function *f = (*func_buf_ptr)[i];
                   char fn[2048];
                   f->getName(fn, 2048);
                   pdstring *prettyName = new pdstring(fn); /// mem leak? sssssssss
                   (*nameBuf).push_back(*prettyName);
                }
                if (!ret.set(nameBuf)) return false;
                break;
                // TODO: should these be passed a process?  
                // yes, they definitely should!
             }
             case 1:  // .calls
             {
                //  here we should check the calls and exclude the calls to
                //  fns in the global_excluded_funcs list.
                //
                // ARI -- This is probably the spot!
                for(unsigned i=0; i<(*func_buf_ptr).size(); i++) {
                   BPatch_function *bpf = (*func_buf_ptr)[i];
                   BPatch_Vector<BPatch_point*> *calls = bpf->findPoint(BPatch_locSubroutine);
                   if (!calls) {
                      char fn[1024];
                      bpf->getName(fn, 1024);
                      fprintf(stderr, "%s[%d]:  no call points for %s\n", __FILE__, __LINE__, fn);
                      continue;
                   }

                   

                   // makes a copy of the return value (on purpose), since we 
                   // may delete some items that shouldn't be a call site for 
                   // this metric.
                   bool anythingRemoved = false; // so far
                   
                   // metric_cerr << "global_excluded_funcs size is: "
                   // << global_excluded_funcs.size() << endl;
                   
                   // metric_cerr << "pdf->funcCalls() returned the following call sites:" 
                   // << endl;
                                                 
                   unsigned oldSize;
                   for (unsigned u = 0; u < (oldSize = calls->size());
                        (oldSize == calls->size()) ? u++ : u) 
                   {  // calls->size() can change!
                   
                      // If this callee is in global_excluded_funcs for this
                      // metric (a global vrble...sorry for that), then it's not
                      // really a callee (for this metric, at least), and thus,
                      // it should be removed from whatever we eventually pass
                      // to "ret.set()" below.
                                                         
                      BPatch_point *point = (*calls)[u];
                      BPatch_function *callee = point->getCalledFunction();
                      if( callee == NULL ) { 
                      	/* We can't do anything useful with this one. */
                      	continue;
                      	}
                      
                      /* Assuming, of course, that the MDL doesn't specify mangled names. */
                      pdvector< pdstring > calleeNames = callee->func->prettyNameVector();
                      for( unsigned int i = 0; i < calleeNames.size(); i++ ) {
                      	pdstring calleeName = calleeNames[i];
                      	// /* DEBUG */ fprintf( stderr, "%s[%d]: calleeName[%d] = %s\n", __FILE__, __LINE__, i, calleeName.c_str() );
                      	for( unsigned int j = 0; j < global_excluded_funcs.size(); j++ ) {
                      		/* Assumes we overload comparison. */
                      		if( global_excluded_funcs[j] == calleeName ) {
                      		   // /* DEBUG */ fprintf( stderr, "%s[%d]: removed callee %s from list of calls.\n", __FILE__, __LINE__, calleeName.c_str() );
                               anythingRemoved = true;
                               
                               // remove calls[u] from calls.  To do this, swap
                               // calls[u] with calls[maxndx], and resize-1.
                               const unsigned maxndx = calls->size()-1;
                               (*calls)[u] = (*calls)[maxndx];
                               calls->resize(maxndx);
                               
                               break;
                               }
                            }
                         }
                   }
                   
                   if (!anythingRemoved) 
                   {
                      // metric_cerr << "nothing was removed -- doing set() now" << endl;
                      const BPatch_Vector<BPatch_point*> *pt_hold = 
                         bpf->findPoint(BPatch_subroutine);
                      if (!pt_hold) {
                        char fn[1024];
                        bpf->getName(fn, 1024);
                        fprintf(stderr, "%s[%d]:  no call points for %s\n", __FILE__, __LINE__, fn);
                        continue;
                      }

                      for(unsigned i=0; i<(*pt_hold).size(); i++)
                         (*inst_point_buf).push_back((*pt_hold)[i]);
                   }
                   else 
                   {
                      // metric_cerr << "something was removed! -- doing set() now" << endl;
                      BPatch_Vector<BPatch_point*> pt_hold(*calls);
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
                   BPatch_function *bpf = (*func_buf_ptr)[i];
                   const BPatch_Vector<BPatch_point*> *entry_pt_hold =
                       bpf->findPoint(BPatch_entry);
                   if (!entry_pt_hold) {
                     char fn[1024];
                     bpf->getName(fn, 1024);
                     fprintf(stderr, "%s[%d]:  no entry points for %s\n", __FILE__, __LINE__, fn);
                     continue;
                   }
                   assert(entry_pt_hold->size());
                   (*inst_point_buf).push_back((*entry_pt_hold)[0]);

		   //cerr << bpf->func->prettyName().c_str() << " entry: ";
		   //fprintf(stderr,"0x%x ",(*entry_pt_hold)[0]->getAddress());
		   //cerr << endl;
                }

                if(! ret.set(inst_point_buf))
                   return false;
                break;
             }
             case 3:   // .return
             {
                 //cerr << "\treturn mdl variable " << ret.get_name() << endl;

                for(unsigned i=0; i<(*func_buf_ptr).size(); i++) {
                   BPatch_function *bpf = (*func_buf_ptr)[i];
                   const BPatch_Vector<BPatch_point *> *func_exit_pts = 
                     bpf->findPoint(BPatch_exit);
                   if (!func_exit_pts) {
                     char fn[1024];
                     bpf->getName(fn, 1024);
                     fprintf(stderr, "%s[%d]:  no exit points for %s\n", __FILE__, __LINE__, fn);
                     continue;
                   }
                   for(unsigned j=0; j<func_exit_pts->size(); j++)
                      (*inst_point_buf).push_back((*func_exit_pts)[j]);
                }
                if(! ret.set(const_cast<pdvector<BPatch_point*>*>(inst_point_buf)))
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
           BPatch_module *mod;
           if (!ret.get(mod)) return false;
           switch (next_field) 
           {
             case 0: 
             {
                char modname[512];
                pdstring fileName = pdstring(mod->getName(modname, 512));
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
                   if (!ret.set(mod->getProcedures())) return false; 
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


bool mdl_get_initial(pdstring flavor, pdRPC *connection) {

   mdl_data::cur_mdl_data = new mdld_data();
   mdl_init();

   // receive the MDL files
   while( !saw_mdl )
   {
      switch (connection->waitLoop()) {
        case T_dyninstRPC::error:
           metric_cerr << "mdl_get_initial flavor: = " << flavor
                       << " connection = " << connection
                       << "  error in connection->waitLoop()" << endl;
           return false;
        default:
           break;
      }
      while (connection->buffered_requests()) {
         switch (connection->process_buffered()) {
           case T_dyninstRPC::error:
              metric_cerr << "mdl_get_initial flavor: = " << flavor
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

bool mdl_get_lib_constraints(pdvector<pdstring> &lc){
   for(u_int i=0; i < mdl_data::cur_mdl_data->lib_constraints.size(); i++){
      lc += mdl_data::cur_mdl_data->lib_constraints[i];
   }
   return (lc.size()>0);
}

void mdl_get_info(pdvector<T_dyninstRPC::metricInfo>& metInfo) {
  unsigned size = mdl_data::cur_mdl_data->all_metrics.size();
  T_dyninstRPC::metricInfo element;
  for (unsigned u=0; u<size; u++) {
    element.name = mdl_data::cur_mdl_data->all_metrics[u]->name_;
    element.style = mdl_data::cur_mdl_data->all_metrics[u]->style_;
    element.aggregate = mdl_data::cur_mdl_data->all_metrics[u]->agg_op_;
    element.units = mdl_data::cur_mdl_data->all_metrics[u]->units_;
    element.developerMode = mdl_data::cur_mdl_data->all_metrics[u]->developerMode_;
    element.unitstype = mdl_data::cur_mdl_data->all_metrics[u]->unitstype_;
    element.handle = 0; // ignored by paradynd for now
    metInfo += element;
  }
}

bool mdl_metric_data(const pdstring& met_name, mdl_inst_data& md) {
  unsigned size = mdl_data::cur_mdl_data->all_metrics.size();
  for (unsigned u=0; u<size; u++)
    if (mdl_data::cur_mdl_data->all_metrics[u]->name_ == met_name) {
      md.aggregate = mdl_data::cur_mdl_data->all_metrics[u]->agg_op_;
      md.style = (metricStyle) mdl_data::cur_mdl_data->all_metrics[u]->style_;
      return true;
    }
  return false;
}


