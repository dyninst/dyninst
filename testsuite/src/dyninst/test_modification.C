/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

/*
 *
 * #Name: test_modification
 * #Desc: CFG Modification via Dyninst code generator
 * #Arch: all
 * #Dep: 
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "BPatch_point.h"
#include "BPatch_function.h"
#include "PatchCFG.h"
#include "PatchModifier.h"

#include "test_lib.h"

#include "dyninst_comp.h"


using namespace Dyninst;
using namespace PatchAPI;
using namespace std;

char modification_buffer[] = {
   0xc7, 0x05, 0x40, 0x9c, 0x05, 0x08, 0x2a, 0x00, 0x00, 0x00 
};
   
   
unsigned modification_buffer_size = 10;

class test_modification_Mutator : public DyninstMutator {
public:
  virtual test_results_t executeTest(); // override
   bool test_modify();
   BPatch_function *findFunc(std::string);
   PatchBlock *splitBlock(PatchBlock *);
   PatchEdge *getFallthrough(PatchBlock *);
   PatchEdge *getSink(PatchBlock *);
};

// Factory function.
extern "C" DLLEXPORT TestMutator* test_modification_factory()
{
  return new test_modification_Mutator();
}

BPatch_function *test_modification_Mutator::findFunc(std::string funcname) {
   BPatch_Vector<BPatch_function *> found;
   appImage->findFunction(funcname.c_str(), found);
   if (found.empty()) return NULL;
   return found[0];
}

PatchBlock *test_modification_Mutator::splitBlock(PatchBlock *block) {
   // We want to split ... somewhere in the middle, eh?
   // Try to figure out a way to make this non-compiler-specific...

   return PatchModifier::split(block, 0x805160a);
}   

PatchEdge *test_modification_Mutator::getFallthrough(PatchBlock *block) {
   return block->findTarget(ParseAPI::FALLTHROUGH);
}

PatchEdge *test_modification_Mutator::getSink(PatchBlock *block) {
   // Look for the sink edge. It'll have a type of DIRECT (by construction)
   // and a sink label
   PatchEdge *e = block->findTarget(ParseAPI::DIRECT);
   if (!e) return NULL;
   if (!e->sinkEdge()) return NULL;
   return e;
}

bool test_modification_Mutator::test_modify() {
   
   const std::string funcname = "test_modification";
   BPatch_function *bpfunc = findFunc(funcname); 
   if (!bpfunc) {
      cerr << "Error: failed to find " << funcname << endl;
      return false;
   }
   PatchFunction *func = bpfunc->getPatchAPIFunc(); 
   if (!func) {
      cerr << "Error: failed to find PatchAPI function" << endl;
      return false;
   }

   PatchBlock *entry = func->entry(); 
   if (!entry) {
      cerr << "Error: failed to find entry block" << endl;
      return false;
   }
   PatchBlock *split = splitBlock(entry); 
   if (!split) { 
      cerr << "Error: failed to split block at magic address" << endl;
      return false;
   }
   
   PatchBlock *inserted = PatchModifier::insert(entry->obj(),
                                                modification_buffer,
                                                modification_buffer_size); 
   if (!inserted) {
      cerr << "Error: failed to insert buffer" << endl;
      return false;
   }
   cerr << "Added new block at address " << hex << inserted->start() 
        << " -> " << inserted->end() << dec << endl;
   PatchBlock::Insns insns;
   inserted->getInsns(insns);
   for (PatchBlock::Insns::iterator iter = insns.begin(); iter != insns.end(); ++iter) {
      cerr << "\t " << hex << iter->first << ": " << iter->second->format() 
           << " /w/ first byte " << (unsigned) iter->second->rawByte(0) << dec << endl;
   }

   PatchEdge *first = getFallthrough(entry); 
   if (!first) {
      cerr << "Error: failed to get fallthrough edge for split blocks" << endl;
      return false;
   }
   PatchEdge *second = getSink(inserted); 
   if (!second) {
      cerr << "Error: failed to find sink edge on new block" << endl;
      return false;
   }
   if (!PatchModifier::redirect(first, inserted)) {
      cerr << "Error: failed to redirect first edge" << endl;
      return false;
   }
   if (!PatchModifier::redirect(second, split)) {
      cerr << "Error: failed to redirect second edge" << endl;
      return false;
   }
   // Generate the new binary
   bpfunc->relocateFunction(); 
   return true;
}

test_results_t test_modification_Mutator::executeTest() {
   // This is really a PatchAPI test, but since I need the
   // Dyninst code generator it's under the Dyninst ones. 
   // Yay!

   // General outline:
   // Take a 1-block function, split that block, and insert
   // new code into the previously split block. Then generate
   // out the new code and examine it to see if things work. 
   //
   // Tests: block splitting, code insertion, edge redirection.
   if (!test_modify()) {
      cerr << "Error: test failed!" << endl;
      return ::FAILED;
   }
   
   return PASSED;
} // test_modification_Mutator::executeTest()
