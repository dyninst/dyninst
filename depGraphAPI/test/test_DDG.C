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

#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "BPatch.h"
#include "BPatch_function.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "Graph.h"

#include "DDG.h"

BPatch bpatch;

using namespace Dyninst;
using namespace DepGraphAPI;

int main(int argc, const char** argv)
{
  
  if( argc < 3 )
    {
      std::cerr << "Usage: " << argv[0] 
		<< "test_DD <test_program> <func_to_analyze>" 
		<< std::endl;
      exit( EXIT_FAILURE );
    }

  BPatch_addressSpace *app = bpatch.openBinary(argv[1], true);

  BPatch_image* appImage = app->getImage();

  BPatch_Vector <BPatch_function *> function;
  appImage->findFunction(argv[2], 
			 function);

  DDG::Ptr ddg = DDG::analyze(function[0]);

  std::string str(argv[2]);
  str += ".dot";
  ddg->printDOT(str);
  
  fprintf(stderr, "Prepping pruned DDG\n");
  DDG::Ptr pruned = ddg->removeDeadNodes();
  std::string str2(argv[2]);
  str2 += ".pruned.dot";
  pruned->printDOT(str2);


  return EXIT_SUCCESS;
}
