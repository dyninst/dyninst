/*
 * Copyright (c) 1996-2008 Barton P. Miller
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

#if !defined(symtab_comp_h_)
#define symtab_comp_h_

#include "test_lib.h"
#include "TestMutator.h"
#include "comptester.h"
#include "ParameterDict.h"

#include "Symtab.h"

class COMPLIB_DLL_EXPORT SymtabMutator : public TestMutator {
 public:
   Dyninst::SymtabAPI::Symtab *symtab;
   SymtabMutator();
   virtual test_results_t setup(ParameterDict &param);
   virtual ~SymtabMutator();
};

extern "C" {
   TEST_DLL_EXPORT TestMutator *TestMutator_factory();
}

class SymtabComponent : public ComponentTester
{
 private:
   ParamPtr symtab_ptr;
 public:
   Dyninst::SymtabAPI::Symtab *symtab;

   SymtabComponent();
   virtual test_results_t program_setup(ParameterDict &params);
   virtual test_results_t program_teardown(ParameterDict &params);
   virtual test_results_t group_setup(RunGroup *group, ParameterDict &params);
   virtual test_results_t group_teardown(RunGroup *group, ParameterDict &params);
   virtual test_results_t test_setup(TestInfo *test, ParameterDict &parms);
   virtual test_results_t test_teardown(TestInfo *test, ParameterDict &parms);

   virtual std::string getLastErrorMsg();

   virtual ~SymtabComponent();
};

extern "C"  {
   TEST_DLL_EXPORT ComponentTester *componentTesterFactory();
}


#endif
