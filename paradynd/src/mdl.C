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

// $Id: mdl.C,v 1.160 2004/07/28 07:24:47 jaw Exp $

#include <iostream>
#include <stdio.h>
#include "dyninstRPC.xdr.SRVR.h"
#include "pdutil/h/mdl_data.h"
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
#include "paradynd/src/mdld_data.h"

// for REG_MT_POS
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)
#include "dyninstAPI/src/inst-sparc.h"
#elif defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1)
#include "dyninstAPI/src/inst-power.h"
#elif defined(i386_unknown_solaris2_5) || defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0)
#include "dyninstAPI/src/inst-x86.h"
#endif

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


/*
 * Simple: return base + (POS * size). ST: POS==0, so return base
 */
 
AstNode *getTimerAddressSlow(void *base, unsigned struct_size,
                             bool for_multithreaded)
{
   if(! for_multithreaded)
      return new AstNode(AstNode::DataPtr, base);

   // Return base + struct_size*POS. Problem is, POS is unknown
   // until we are running the instrumentation

   pdvector<AstNode *> ast_args;
   AstNode *get_thr    = new AstNode("pthread_self", ast_args);
   AstNode *get_index  = new AstNode("DYNINSTthreadIndexSLOW", get_thr);
   AstNode *increment  = new AstNode(AstNode::Constant, (void *)struct_size);
   AstNode *var_base   = new AstNode(AstNode::DataPtr, base);
   
   AstNode *offset     = new AstNode(timesOp, get_index, increment);
   AstNode *var        = new AstNode(plusOp, var_base, offset);

   removeAst(get_thr);
   removeAst(get_index);
   removeAst(increment);
   removeAst(var_base);
   removeAst(offset);
   return var;
}

AstNode *getTimerAddressFast(void *base, unsigned struct_size,
                             bool for_multithreaded)
{
   if(! for_multithreaded)
      return new AstNode(AstNode::DataPtr, base);

   // Return base + struct_size*POS. Problem is, POS is unknown
   // until we are running the instrumentation
   
   AstNode *pos        = new AstNode(AstNode::DataReg, (void *)REG_MT_POS);

   AstNode *increment  = new AstNode(AstNode::Constant, (void *)struct_size);
   AstNode *var_base   = new AstNode(AstNode::DataPtr, base);
   
   AstNode *offset     = new AstNode(timesOp, pos, increment);

   AstNode *var        = new AstNode(plusOp, var_base, offset);

   removeAst(pos);


   removeAst(increment);
   removeAst(var_base);
   removeAst(offset);
   return var;
}

AstNode *getTimerAddress(void *base, unsigned struct_size,
                         bool for_multithreaded)
{
#if defined(i386_unknown_linux2_0)
   return getTimerAddressSlow(base, struct_size, for_multithreaded);
#else
   return getTimerAddressFast(base, struct_size, for_multithreaded);
#endif
}

AstNode *createTimer(const pdstring &func, void *dataPtr, 
                     pdvector<AstNode *> &ast_args, bool for_multithreaded)
{
   AstNode *var_base=NULL,*timer=NULL;

   // t0 = new AstNode(AstNode::Constant, (void *) dataPtr);  // This was AstNode::DataPtr

   var_base = getTimerAddress(dataPtr, sizeof(tTimer), for_multithreaded);
   ast_args.push_back(assignAst(var_base));
   removeAst(var_base);
   timer = new AstNode(func, ast_args);

   for(unsigned i=0; i<ast_args.size(); i++)
      removeAst(ast_args[i]);  

   return timer;
}


AstNode *createHwTimer(const pdstring &func, void *dataPtr, 
                     pdvector<AstNode *> &ast_args, int hwCntrIndex)
{
  AstNode *var_base=NULL,*timer=NULL;
  AstNode *hw_var=NULL;

  var_base = new AstNode(AstNode::DataPtr, dataPtr);
  hw_var = new AstNode(AstNode::Constant, (void*)hwCntrIndex);

  ast_args += assignAst(var_base);
  ast_args += assignAst(hw_var);
   
  removeAst(var_base);
  removeAst(hw_var);

  timer = new AstNode(func, ast_args);
  for (unsigned i=0;i<ast_args.size();i++) removeAst(ast_args[i]);  
  return(timer);
}

AstNode *getCounterAddressSlow(void *base, unsigned struct_size,
                               bool for_multithreaded)
{
   if(! for_multithreaded)
      return new AstNode(AstNode::DataAddr, base);

   pdvector<AstNode *> ast_args;
   AstNode *get_thr    = new AstNode("pthread_self", ast_args);
   AstNode *get_index  = new AstNode("DYNINSTthreadIndexSLOW", get_thr);   
   AstNode *increment  = new AstNode(AstNode::Constant, (void *)struct_size);
   AstNode *var_base   = new AstNode(AstNode::DataPtr, base);
   
   AstNode *offset     = new AstNode(timesOp, get_index, increment);
   AstNode *var        = new AstNode(plusOp, var_base, offset);

   // Hrm... this gives us the base address just fine, but we need a "load"
   // to actually make it work. 

   removeAst(get_thr);
   removeAst(get_index);
   removeAst(increment);
   removeAst(var_base);
   removeAst(offset);
   return var;
}

AstNode *getCounterAddressFast(void *base, unsigned struct_size,
                               bool for_multithreaded)
{
   if(! for_multithreaded)
      return new AstNode(AstNode::DataAddr, base);
   
   AstNode *pos        = new AstNode(AstNode::DataReg, (void *)REG_MT_POS);
   AstNode *increment  = new AstNode(AstNode::Constant, (void *)struct_size);
   AstNode *var_base   = new AstNode(AstNode::DataPtr, base);
   
   AstNode *offset     = new AstNode(timesOp, pos, increment);
   AstNode *var        = new AstNode(plusOp, var_base, offset);

   // Hrm... this gives us the base address just fine, but we need a "load"
   // to actually make it work. 

   removeAst(pos);
   removeAst(increment);
   removeAst(var_base);
   removeAst(offset);
   return var;
}

AstNode *getCounterAddress(void *base, unsigned struct_size,
                           bool for_multithreaded)
{
#if defined(i386_unknown_linux2_0)
   return getCounterAddressSlow(base, struct_size, for_multithreaded);
#else
   return getCounterAddressFast(base, struct_size, for_multithreaded);
#endif
}


AstNode *createCounter(const pdstring &func, void *dataPtr, 
                       AstNode *ast, bool for_multithreaded) 
{
   AstNode *load=NULL, *calc=NULL, *store=NULL;

   // We keep the different MT__THREAD code, because otherwise we really
   // de-optimize the singlethread case
   AstNode *counter_base = getCounterAddress(dataPtr, sizeof(intCounter),
                                             for_multithreaded);
   if (func=="addCounter") {
      if(for_multithreaded) {
         load = new AstNode(AstNode::DataIndir,counter_base);
         calc = new AstNode(plusOp,load,ast);
      } else {
         calc = new AstNode(plusOp,counter_base, ast);
      }

      store = new AstNode(storeOp,counter_base,calc);
   } else if (func=="subCounter") {
      if(for_multithreaded) {
         load = new AstNode(AstNode::DataIndir,counter_base);
         calc = new AstNode(minusOp,load,ast);
      } else {
         calc = new AstNode(minusOp,counter_base,ast);
      }

      store = new AstNode(storeOp,counter_base,calc);
   } else if (func=="setCounter") {
      store = new AstNode(storeOp,counter_base,ast);
   } else abort();

   if (counter_base) removeAst(counter_base);
   if (calc) removeAst(calc);
   if (load) removeAst(load);

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
mdld_v_expr::apply_be(AstNode*& ast)
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
           // create another pdstring here and pass it to AstNode(), instead
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
           if (!dynamic_cast<mdld_expr*>(left_)->apply_be(index_var))
              return false;
           int index_value;
           if (!index_var.get(index_value))
              return false;

           if (var_ == pdstring ("$arg"))
              ast = new AstNode (AstNode::Param, (void*)index_value);
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
              ast = new AstNode(AstNode::Constant, (void*)value);
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
                      ast = new AstNode(AstNode::Constant, (void*)value);
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
           if (!dynamic_cast<mdld_expr*>(left_)->apply_be (lnode)) return false;
           if (!dynamic_cast<mdld_expr*>(right_)->apply_be (rnode)) return false;
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

              pdvector<AstNode *> ast_args;
              ast = createTimer(timer_func, (void*)(dn->getInferiorPtr()),
                                ast_args, global_proc->multithread_capable());
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
                 Symbol info;
                 Address baseAddr;
                 if (global_proc->getSymbolInfo(symbol_name, info, baseAddr)) 
                 {
                    Address adr = info.addr();
                    ast = new AstNode(AstNode::DataAddr, (void*)adr);
                 } 
                 else 
                 {
                    pdstring msg = pdstring("In metric '") + currentMetric + pdstring("': ")
                       + pdstring("unable to find symbol '") + symbol_name + pdstring("'");
                    mdl_data::cur_mdl_data->env->appendErrorString( msg );
                    return false;
                 }
              }
           }
           else if (var_ == "readAddress")
           {
              mdl_var addr_var;
              if (!dynamic_cast<mdld_expr*>((*args_)[0])->apply_be (addr_var))
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
                 if (!dynamic_cast<mdld_expr*>((*args_)[u])->apply_be(tmparg)) 
                 {
                    removeAst(tmparg);
                    return false;
                 }
                 astargs += assignAst(tmparg);
                 removeAst(tmparg);
              }

              BPatch_Vector<BPatch_function *> bpfv;
              BPatch_function *bpf = NULL;

              if (!global_proc->findAllFuncsByName(var_, bpfv)) {
                 pdstring msg = pdstring("In metric '") + currentMetric + pdstring("': ")
                    + pdstring("unable to find procedure '") + var_ + pdstring("'");
                 mdl_data::cur_mdl_data->env->appendErrorString( msg );
                 return false;
              }
              else {
                 if (bpfv.size() > 1) {
                    pdstring msg = 
                       pdstring("WARNING:  found ") + pdstring(bpfv.size()) +
                       pdstring(" records of function '") + var_ + pdstring("'") +
                       pdstring(".  Using the first.(3)");
                 mdl_data::cur_mdl_data->env->appendErrorString( msg );
                 }
                 bpf = bpfv[0];
              }

              if (!bpf) 
              {
                 pdstring msg = pdstring("In metric '") + currentMetric + pdstring("': ")
                    + pdstring("unable to find procedure '") + var_ + pdstring("'");
                 mdl_data::cur_mdl_data->env->appendErrorString( msg );
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
        AstNode* ast_arg;
        if (!dynamic_cast<mdld_expr*>(left_)->apply_be(ast_arg)) return false;

        pdstring func_str;
        switch (bin_op_)
        {
          case MDL_ASSIGN: func_str = "setCounter"; break;
          case MDL_PLUSASSIGN: func_str = "addCounter"; break;
          case MDL_MINUSASSIGN: func_str = "subCounter"; break;
          default: return false;
        }
        ast = createCounter(func_str, (void*)(dn->getInferiorPtr()), ast_arg,
                            global_proc->multithread_capable());
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

                if(global_proc->multithread_capable()) {
                   AstNode *tmp_ast;
                   tmp_ast = getCounterAddress((void *)(dn->getInferiorPtr()),
                                               dn->getSize(),
                                           global_proc->multithread_capable());
                   // First we get the address, and now we get the value...
                   ast = new AstNode(AstNode::DataIndir,tmp_ast);
                   removeAst(tmp_ast);
                } else {
                   // ast = new AstNode(AstNode::DataValue,  // restore AstNode::DataValue
                   ast = new AstNode(AstNode::DataAddr,
                                     (void*)(dn->getInferiorPtr()));
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
                if(global_proc->multithread_capable()) {
                   ast = getTimerAddress((void *)(dn->getInferiorPtr()),
                                         dn->getSize(),
                                         global_proc->multithread_capable());
                } else {
                   //ast =new AstNode(AstNode::Constant, //was AstNode::DataPtr
                   // restore AstNode::DataPtr
                   ast = new AstNode(AstNode::DataPtr,  
                                     (void*)(dn->getInferiorPtr()));
                }

                break;
             }
          case MDL_MINUS:
             {
                mdl_var tmp;
                if (!dynamic_cast<mdld_expr*>(left_)->apply_be (tmp)) return false;
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
                if (!dynamic_cast<mdld_expr*>(left_)->apply_be(dn_var)) return false;
                if (!dn_var.get(dn)) return false;

                int value = 1;
                AstNode* ast_arg = new AstNode(AstNode::Constant, (void*)value);

                ast = createCounter("addCounter",(void*)(dn->getInferiorPtr()),
                                    ast_arg,
                                    global_proc->multithread_capable());
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

bool
mdld_v_expr::apply_be(mdl_var& ret) 
{
   switch (type_) 
   {
     case MDL_EXPR_INT: 
        return (ret.set(int_literal_));
     case MDL_EXPR_STRING:
        return (ret.set(str_literal_));
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
              if (!dynamic_cast<mdld_expr*>(left_)->apply_be(ndx)) return false;
              int x;
              if (!ndx.get(x)) return false;
              return (mdl_data::cur_mdl_data->env->get(ret, var_+pdstring(x)));
           }
           mdl_var array(false);
           if (!mdl_data::cur_mdl_data->env->get(array, var_)) return false;
           if (!array.is_list()) return false;  
           mdl_var index_var;
           if (!dynamic_cast<mdld_expr*>(left_)->apply_be(index_var)) return false;
           int index_value;
           if (!index_var.get(index_value)) return false;
           if (index_value >= (int)array.list_size()) return false;
           return (array.get_ith_element(ret, index_value));
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
              // TODO -- what if the function is not found ?
              BPatch_Vector<BPatch_function *> bpfv;
              BPatch_function *bpf;
              
              if (!global_proc->findAllFuncsByName(func_name, bpfv)) {
                 pdstring msg = pdstring("In metric '") + currentMetric +
                              pdstring("': ") + 
                              pdstring("unable to find procedure '") +
                              func_name + pdstring("'");
                 mdl_data::cur_mdl_data->env->appendErrorString( msg );
                 assert(0);
              }
              else {
                 if (bpfv.size() > 1) {
                    pdstring msg = pdstring("WARNING:  found ") +pdstring(bpfv.size())
                               + pdstring(" records of function '") + func_name +
                                 pdstring("'") + pdstring(".  Using the first.");
                     mdl_data::cur_mdl_data->env->appendErrorString( msg );
                 }
                 bpf = bpfv[0];
              }
              if (!bpf) { assert(0); return false; }
              return (ret.set(bpf));
           }
           else { assert(0); return false; }
        }
     }
  case MDL_EXPR_DOT:
     {
        if (!dynamic_cast<mdld_expr*>(left_)->apply_be(ret)) return false;

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
     return false;
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
mdld_icode::apply_be(AstNode *&mn, bool mn_initialized,
				    pd_process *proc) 
{
  // a return value of true implies that "mn" has been written to

  if (!expr_)
     return false;

  AstNode* pred = NULL;
  AstNode* ast = NULL;

  if (if_expr_) {
    if (!dynamic_cast<mdld_expr*>(if_expr_)->apply_be(pred)) {
       return false;
    }
  }

  if (!dynamic_cast<mdld_expr*>(expr_)->apply_be(ast)) {
    return false;
  }

  AstNode* code = NULL;
  if (pred) 
  {
    // Note: we don't use assignAst on purpose here
    code = createIf(pred, ast, proc->get_dyn_process()->lowlevel_process());
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
  //  pdvector<function_base*> *vp;
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
  if (!dynamic_cast<mdld_expr*>(point_expr_)->apply_be(pointsVar)) { // process the 'point(s)' e.g. "$start.entry"
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
  AstNode *code = NULL;
  unsigned size = icode_reqs_->size();
  for (unsigned u=0; u<size; u++) {
    if (!((mdld_icode*)(*icode_reqs_)[u])->apply_be(code, u>0, mn->proc())) {
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
        AstNode *temp1;
        if(global_proc->multithread_capable()) {
           AstNode *tmp_ast = getCounterAddress(
                                  (void *)((inFlags[fi])->getInferiorPtr()),
                                  inFlags[fi]->getSize(),
                                  global_proc->multithread_capable());
           temp1 = new AstNode(AstNode::DataIndir, tmp_ast);
           removeAst(tmp_ast);
        } else {
           // Note: getInferiorPtr could return a NULL pointer here if we are
           // just computing cost - naim 2/18/97
                      
           // AstNode *temp1 = new AstNode(AstNode::DataValue,
           temp1 = new AstNode(AstNode::DataAddr, 
                               (void*)((inFlags[fi])->getInferiorPtr()));
        }
        // Note: we don't use assignAst on purpose here
        AstNode *temp2 = code;
        code = createIf(temp1, temp2,
                        mn->proc()->get_dyn_process()->lowlevel_process());
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
     mn->addInst(points[i]->PDSEP_instPoint(), code, cwhen, corder);
     // appends an instReqNode to mn's instRequests; actual 
     // instrumentation only
     // takes place when mn->loadInstrIntoApp() is later called.
  }

  removeAst(code); 
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
   if(!do_trailing_resources(resource.tokenized(), proc)) {
      mdl_data::cur_mdl_data->env->pop();
      return(false);
   }
   
   // Now evaluate the constraint statements
   unsigned size = stmts_->size();
   pdvector<const instrDataNode*> flags;

   for (unsigned u=0; u<size; u++) {
      if (!dynamic_cast<mdld_stmt*>((*stmts_)[u])->apply_be(codeNode, flags)) {
         return(false);
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
     ((mdld_constraint*)repl_cons)->mk_list(global_excluded_funcs);
  } else {
     for (unsigned u1 = 0; u1 < flag_cons.size(); u1++) {
        ((mdld_constraint*)flag_cons[u1])->mk_list(global_excluded_funcs);
     }
     for (unsigned u2 = 0; u2 < stmts_->size(); u2++) {
        dynamic_cast<mdld_stmt*>((*stmts_)[u2])->mk_list(global_excluded_funcs);
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
   // create the instrCodeNodes and instrDataNodes for the flag constraints
   
   if(repl_cons == NULL) {
      unsigned flag_size = flag_cons.size(); // could be zero
      
      for(unsigned fs=0; fs<flag_size; fs++) {
         pdstring cons_name(flag_cons[fs]->id());
         
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



static bool do_trailing_resources(const pdvector<pdstring>& resource_,
				  pd_process *proc)
{
   pdvector<pdstring>  resPath;

   for(unsigned pLen = 0; pLen < resource_.size(); pLen++) {
      pdstring   caStr = pdstring("$constraint") + 
                       pdstring(resource_.size()-pLen-1);
      pdstring   trailingRes = resource_[pLen];
      resPath += resource_[pLen];
      assert(resPath.size() == (pLen+1));
      
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
           mdl_data::cur_mdl_data->env->add(caStr, false, MDL_T_INT);
           mdl_data::cur_mdl_data->env->set(val, caStr);
           break;
        }
        case MDL_T_STRING:
           mdl_data::cur_mdl_data->env->add(caStr, false, MDL_T_STRING);
           mdl_data::cur_mdl_data->env->set(trailingRes, caStr);
           break;
        case MDL_T_PROCEDURE: {
           // find the resource corresponding to this function's module 
           pdvector<pdstring> m_vec;
           for(u_int i=0; i < resPath.size()-1; i++){
              m_vec += resPath[i];
           }
           assert(m_vec.size());
           assert(m_vec.size() == (resPath.size()-1));
           resource *m_resource = resource::findResource(m_vec);
           if(!m_resource) {
              return(false);
           }
           
           BPatch_Vector<BPatch_function *> *func_buf = new BPatch_Vector<BPatch_function*>;
           if ( !proc->findAllFuncsByName(r, m_resource, *func_buf) ) {
              const pdvector<pdstring> &f_names = r->names();
              const pdvector<pdstring> &m_names = m_resource->names();
              pdstring func_name = f_names[f_names.size() -1]; 
              pdstring mod_name = m_names[m_names.size() -1]; 
              
              pdstring msg = pdstring("For requested metric-focus, ") +
                           pdstring("unable to find  function ") +
                           func_name;
              mdl_data::cur_mdl_data->env->appendErrorString( msg );
              return false;
           }

           mdl_data::cur_mdl_data->env->add(caStr, false, MDL_T_PROCEDURE);
           mdl_data::cur_mdl_data->env->set(func_buf, caStr);
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
                for(unsigned i=0; i<mods.size(); i++) {
                cerr << "  i: " << i << ", filenm: " << mods[i]->fileName()
                << ", fullnm: " << mods[i]->fullName() << "\n";
                }
              */
              return(false);
           }
           mdl_data::cur_mdl_data->env->add(caStr, false, MDL_T_MODULE);
           mdl_data::cur_mdl_data->env->set(mod, caStr);
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
                // TODO: should these be passed a process?  yes, they definitely should!
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
                      // metric_cerr << u << ") ";
                                                         
                      BPatch_point *point = (*calls)[u];
                      BPatch_function *callee = point->getCalledFunction();
                         //dynamic_cast<function_base*>(point->getCallee());
                                                         
                      const char *callee_name=NULL;
                      char fnamebuf[2048];                                   
                      if (callee == NULL) 
                      {
                         // an unanalyzable function call; sorry.
                         callee_name = NULL;
                         // metric_cerr << "-unanalyzable-" << endl;
                      }
                      else 
                      {
                         callee_name = callee->getName(fnamebuf, 2048);
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
                               const unsigned maxndx = calls->size()-1;
                               (*calls)[u] = (*calls)[maxndx];
                               calls->resize(maxndx);
                               
                               // metric_cerr << "removed something! -- " << callee_name << endl;
                               
                               break;
                            }
                         }
                   }
                   
                   if (!anythingRemoved) 
                   {
                      // metric_cerr << "nothing was removed -- doing set() now" << endl;
                      const BPatch_Vector<BPatch_point*> *pt_hold = 
                         bpf->findPoint(BPatch_subroutine);
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
                   assert(entry_pt_hold->size());
                   (*inst_point_buf).push_back((*entry_pt_hold)[0]);
                }
                if(! ret.set(inst_point_buf))
                   return false;
                break;
             }
             case 3:   // .return
             {
                for(unsigned i=0; i<(*func_buf_ptr).size(); i++) {
                   BPatch_function *bpf = (*func_buf_ptr)[i];
                   const BPatch_Vector<BPatch_point *> *func_exit_pts = 
                     bpf->findPoint(BPatch_exit);
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


