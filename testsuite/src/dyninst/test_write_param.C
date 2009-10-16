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

// $Id: test1_1.C,v 1.1 2008/10/30 19:17:22 legendre Exp $
/*
 *
 * #Name: test_write_param
 * #Desc: Write to a parameter and return value
 * #Arch: all
 * #Dep: 
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

#include "dyninst_comp.h"

class test_write_param_Mutator : public DyninstMutator {
public:
   virtual test_results_t executeTest(); // override
   BPatch_function *getFunc(const char *name);
   bool insertParamExpr(BPatch_point *point, long v0, long v1, long v2, long v3, 
                        long v4, long v5, long v6, long v7);
   bool insertRetExpr(BPatch_point *point, long value);
};

// Factory function.
extern "C" DLLEXPORT TestMutator* test_write_param_factory()
{
  return new test_write_param_Mutator();
}

BPatch_function *test_write_param_Mutator::getFunc(const char *name)
{
  BPatch_Vector<BPatch_function *> found_funcs;
  appImage->findFunction(name, found_funcs);
  if (found_funcs.size() == 0) {
     logerror("  Unable to find function %s\n", name);
     return NULL;
  }
  if (found_funcs.size() > 1) {
     logerror("  Found too many copies of %s\n", name);
     return NULL;
  }
  return found_funcs[0];
}

bool test_write_param_Mutator::insertParamExpr(BPatch_point *point, long v0, long v1, 
                                               long v2, long v3, long v4, long v5, 
                                               long v6, long v7)
{
   BPatch_arithExpr p0(BPatch_assign, BPatch_paramExpr(0), BPatch_constExpr(v0));
   BPatch_arithExpr p1(BPatch_assign, BPatch_paramExpr(1), BPatch_constExpr(v1));
   BPatch_arithExpr p2(BPatch_assign, BPatch_paramExpr(2), BPatch_constExpr(v2));
   BPatch_arithExpr p3(BPatch_assign, BPatch_paramExpr(3), BPatch_constExpr(v3));
   BPatch_arithExpr p4(BPatch_assign, BPatch_paramExpr(4), BPatch_constExpr(v4));
   BPatch_arithExpr p5(BPatch_assign, BPatch_paramExpr(5), BPatch_constExpr(v5));
   BPatch_arithExpr p6(BPatch_assign, BPatch_paramExpr(6), BPatch_constExpr(v6));
   BPatch_arithExpr p7(BPatch_assign, BPatch_paramExpr(7), BPatch_constExpr(v7));
   BPatch_Vector<BPatch_snippet *> seq;
   seq.push_back(&p0);
   seq.push_back(&p1);
   seq.push_back(&p2);
   seq.push_back(&p3);
   seq.push_back(&p4);
   seq.push_back(&p5);
   seq.push_back(&p6);
   seq.push_back(&p7);
   BPatch_sequence sequence(seq);
   appAddrSpace->insertSnippet(sequence, *point);
   return true;
}

bool test_write_param_Mutator::insertRetExpr(BPatch_point *point, long value)
{
   BPatch_arithExpr p(BPatch_assign, BPatch_retExpr(), BPatch_constExpr(value));
   appAddrSpace->insertSnippet(p, *point, BPatch_callAfter);
   return true;
}


//
// Start Test Case #1 - (zero arg function call)
//
test_results_t test_write_param_Mutator::executeTest() {
  const char *funcName = "test_write_param_func";
  const char *call1Name = "test_write_param_call1";
  const char *call2Name = "test_write_param_call2";
  const char *call3Name = "test_write_param_call3";
  const char *call4Name = "test_write_param_call4";

  // Find the entry point to the procedure "func1_1"
 
  BPatch_function *func = getFunc(funcName);
  BPatch_function *call1 = getFunc(call1Name);
  BPatch_function *call2 = getFunc(call2Name);
  BPatch_function *call3 = getFunc(call3Name);
  BPatch_function *call4 = getFunc(call4Name);

  if (!func || !call1 || !call2 || !call3 || !call4)
     return FAILED;
  
  
  BPatch_Vector<BPatch_point *> *calls = func->findPoint(BPatch_locSubroutine);
  if (!calls || calls->size() < 4) {
     logerror("Didn't find correct number of calls in %s (found %d)\n", 
              funcName, calls ? calls->size() : 0);
     return FAILED;
  }

  for (unsigned i=0; i<calls->size(); i++) {
     BPatch_function *callee = (*calls)[i]->getCalledFunction();
     if (callee == call1) {
        //Insert snippet at callsite
        insertParamExpr((*calls)[i], 1, 2, 3, 4, 5, 6, 7, 8);
        continue;
     }
     if (callee == call2) {
        //Insert snippet at call2 entry
        BPatch_Vector<BPatch_point *> *entries = callee->findPoint(BPatch_entry);
        BPatch_point *entry = (*entries)[0];
        insertParamExpr(entry, 11, 12, 13, 14, 15, 16, 17, 18);
        continue;
     }
     if (callee == call3) {
        //Insert snippet at callsite
        insertRetExpr((*calls)[i], 20);
        continue;
     }
     if (callee == call4) {
        BPatch_Vector<BPatch_point *> *exits = callee->findPoint(BPatch_exit);
        BPatch_point *exit = (*exits)[0];
        insertRetExpr(exit, 30);
        continue;
     }
  }

  return PASSED;
} // test_write_param_Mutator::executeTest()
