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
// Inst2AST
// Convert a single instruction to an AST and print

#include "BPatch.h"
#include "BPatch_function.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "AbslocInterface.h"
#include "BPatch_flowGraph.h"
#include "BPatch_basicBlock.h"

#include "SymEval.h"

#include "InstructionDecoder.h"
using namespace Dyninst;
using namespace Dyninst::InstructionAPI;
using namespace Dyninst::DataflowAPI;

#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <cstdio>
using namespace std;

int main(int argc, char *argv[])
{
  // Symbolic evaluation of all instructions
  // in a provided binary/function

  BPatch bpatch;

  if( argc < 3 ) {
    cerr << "Usage: " << argv[0] 
	 << "inst2ast <test_program> <func_to_analyze>" 
	 << endl;
    exit(-1);
  }

  BPatch_addressSpace *app = bpatch.openBinary(argv[1], true);

  BPatch_image* appImage = app->getImage();

  BPatch_Vector <BPatch_function *> function;
  appImage->findFunction(argv[2], 
                         function);
  
  BPatch_flowGraph *cfg = function[0]->getCFG();
  std::set<BPatch_basicBlock *> blocks;
  cfg->getAllBasicBlocks(blocks);

  AssignmentConverter converter(true);

  for (std::set<BPatch_basicBlock *>::iterator b_iter = blocks.begin();
       b_iter != blocks.end(); ++b_iter) {
    std::vector<std::pair<Instruction::Ptr, Address> > insns;
    (*b_iter)->getInstructions(insns);
    for (unsigned i = 0; i < insns.size(); ++i) {
      std::vector<Assignment::Ptr> assigns;
      cerr << insns[i].first->format() << endl;

      converter.convert(insns[i].first,
			insns[i].second,
			function[0],
			assigns);

      // Build a map stating which assignments we're interested in
      Result_t res;
      for (std::vector<Assignment::Ptr>::iterator a_iter = assigns.begin();
	   a_iter != assigns.end(); ++a_iter) {
	//cerr << "Recording interest in " << (*a_iter)->format() << endl;
	res[*a_iter] = AST::Ptr();
      }

      // Feed the map into the symbolic evaluator
      SymEval<Arch_x86>::expand(res);

      // What do we have here?
      for (Result_t::const_iterator r_iter = res.begin();
	   r_iter != res.end(); ++r_iter) {
	cout << "-----------------" << endl;
	cout << r_iter->first->format();
	cout << " == ";
	cout << r_iter->second->format() << endl;
      }
      cout << endl << "====================" << endl;
    }
  }

  return 0;
}
