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

#include "symtab_comp.h"
#include "test_lib.h"
#include <iostream>

#include "Symtab.h"
#include "Symbol.h"

using namespace Dyninst;
using namespace SymtabAPI;

class test_exception_Mutator : public SymtabMutator {
   std::vector<ExceptionBlock *> excps;
public:
   test_exception_Mutator() { };
   virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* test_exception_factory()
{
   return new test_exception_Mutator();
}

#define NUM_EXPECTED_EXCPS 3

test_results_t test_exception_Mutator::executeTest()
{
	if (createmode == DESERIALIZE)
		return SKIPPED;
#if defined (os_windows_test)
	return SKIPPED;
#endif

#if defined (os_aix_test)
	return SKIPPED;
#endif

   bool result = symtab->getAllExceptions(excps);

   if (!result || !excps.size() )
   {
      logerror("%s[%d]: - Failed to get exceptions\n", 
               FILE__, __LINE__);
      fprintf(stderr, "%s[%u]: - Failed to get exceptions\n", 
               FILE__, __LINE__);
      return FAILED;
   }

#if  0
   fprintf(stderr, "%s[%d]:  have %d exception blocks:\n", FILE__, __LINE__, excps.size());

   for (unsigned int i = 0; i < excps.size(); ++i)
   {
	   std::cerr << "     " << *(excps[i]) << std::endl;
   }
#endif

#if 0
   if (excps.size() != NUM_EXPECTED_EXCPS)
   {
      logerror("%s[%u]: - wrong number of exception blocks found: %d\n", 
               FILE__, __LINE__, excps.size());
      return FAILED;
   }
#endif


   return PASSED;
}

