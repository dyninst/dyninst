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

#include "symtab_comp.h"
#include "test_lib.h"

#include "Symtab.h"
#include "Symbol.h"
#include <iostream>

using namespace Dyninst;
using namespace SymtabAPI;

class test_relocations_Mutator : public SymtabMutator {
   std::vector<relocationEntry> relocs;
public:
   test_relocations_Mutator() { };
   virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* test_relocations_factory()
{
   return new test_relocations_Mutator();
}

test_results_t test_relocations_Mutator::executeTest()
{
   bool result = symtab->getFuncBindingTable(relocs);

   if (!result || !relocs.size() )
   {
      logerror("%s[%d]: - Unable to find relocations\n", 
               FILE__, __LINE__);
      fprintf(stderr, "%s[%d]: - Unable to find relocations\n", 
               FILE__, __LINE__);
      return FAILED;
   }

   fprintf(stderr, "%s[%d]:  have relocs:\n", FILE__, __LINE__);
   for (unsigned int i = 0; i < relocs.size(); ++i)
   {
	   Symbol *s = relocs[i].getDynSym();
	   std::cerr << "      " <<  relocs[i];
	   if (s)
		  std::cerr << "  symname:  " << s->getName() << "  symaddr: " <<s->getAddr() << " symtype = "<< Symbol::symbolType2Str(s->getType()) << "symlinkage = " <<Symbol::symbolLinkage2Str(s->getLinkage()) << " vis = " <<Symbol::symbolVisibility2Str(s->getVisibility()); 
	   std::cerr << std::endl;
   }
#if 0
   if (relocs.size() != 3)
   {
      logerror("%s[%d]: - wrong number of relocs??: %d\n", 
               FILE__, __LINE__, relocs.size());
      fprintf(stderr, "%s[%d]: - wrong number of relocs??: %d\n", 
               FILE__, __LINE__, relocs.size());
      return FAILED;
   }
#endif

   return PASSED;
}

