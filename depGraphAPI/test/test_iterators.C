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

#include "Node.h"
#include "Edge.h"

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

  // Find an in node
  NodeIterator nBegin, nEnd;
  ddg->entryNodes(nBegin, nEnd);
  if (nBegin == nEnd) return 0;
  Node::Ptr entryNode = *nBegin;

  fprintf(stderr, "Testing in set of edges\n");
  EdgeIterator eBegin, eEnd;
  entryNode->ins(eBegin, eEnd);
  for (; eBegin != eEnd; eBegin++) {
      fprintf(stderr, "\tsource: %s\n", (*eBegin)->source()->format().c_str());
  }

  fprintf(stderr, "Testing out set of edges\n");
  entryNode->outs(eBegin, eEnd);
  for (; eBegin != eEnd; eBegin++) {
      fprintf(stderr, "\ttarget: %s\n", (*eBegin)->target()->format().c_str());
  }

  fprintf(stderr, "Testing in nodes\n");
  entryNode->ins(nBegin, nEnd);
  for (; nBegin != nEnd; nBegin++) {
      fprintf(stderr, "\tsource:%s\n", (*nBegin)->format().c_str());
  }

  fprintf(stderr, "Testing out nodes\n");
  entryNode->outs(nBegin, nEnd);
  for (; nBegin != nEnd; nBegin++) {
      fprintf(stderr, "\ttarget:%s\n", (*nBegin)->format().c_str());
  }
  
  fprintf(stderr, "Testing backwards closure\n");
  entryNode->backwardClosure(nBegin, nEnd);
  for (; nBegin != nEnd; nBegin++) {
      fprintf(stderr, "\tback node:%s\n", (*nBegin)->format().c_str());
  }

  fprintf(stderr, "Testing forwards closure\n");
  entryNode->forwardClosure(nBegin, nEnd);
  for (; nBegin != nEnd; nBegin++) {
      fprintf(stderr, "\tforward node:%s\n", (*nBegin)->format().c_str());
  }


  return EXIT_SUCCESS;
}
