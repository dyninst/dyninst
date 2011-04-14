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

#include <sys/times.h>

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
  
  if( argc < 1 )
    {
      std::cerr << "Usage: " << argv[0] 
		<< "coverage <test_program>" 
		<< std::endl;
      exit( EXIT_FAILURE );
    }

  BPatch_addressSpace *app = bpatch.openBinary(argv[1], true);

  BPatch_image* appImage = app->getImage();

  BPatch_Vector <BPatch_function *> function;
  appImage->getProcedures(function);

  for (unsigned i = 0; i < function.size(); i++) {
      char buffer[256];
      char libbuf[256];
      function[i]->getName(buffer, 256);
      function[i]->getModule()->getName(libbuf, 256);
      fprintf(stderr, "analyzing %s (%s)\n", buffer, libbuf);
      DDG::Ptr ddg = DDG::analyze(function[i]);
      fprintf(stderr, "... done\n");
      ddg->removeAnnotation();
  }

  return EXIT_SUCCESS;
}
