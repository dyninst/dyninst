/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#if !defined(symtab_comp_h_)
#define symtab_comp_h_

#include "test_lib.h"
#include "TestMutator.h"
#include "comptester.h"
#include "ParameterDict.h"

#include "Symtab.h"

class COMPLIB_DLL_EXPORT SymtabMutator : public TestMutator {
  protected:
   create_mode_t createmode;
   std::string mutatee;
   std::string compiler;
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
   ParamString mutatee_p;
   ParamString compiler_p;

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
