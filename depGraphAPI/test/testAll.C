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
#include "CDG.h"
#include "PDG.h"
#include "FDG.h"
#include "xPDG.h"

BPatch bpatch;

using namespace Dyninst;
using namespace DepGraphAPI;

#define DDG_ID 1
#define CDG_ID 2
#define FDG_ID 4
#define PDG_ID 8
#define XPDG_ID 16

int main(int argc, const char** argv)
{
  
  if( argc < 3 )
    {
      std::cerr << "Usage: " << argv[0] 
		<< " <test_program> <func_to_analyze> [graph type: any subset of ddg, cdg, fdg, pdg, xpdg]" 
		<< std::endl;
      exit( EXIT_FAILURE );
    }
  
  unsigned process = 0;
  if (argc == 3) {
      process = 31;
  }
  else {
      for (int i = 3; i < argc; i++) {
          if (strcmp(argv[i], "ddg") == 0) {
              process += DDG_ID;
          }
          else if (strcmp(argv[i], "cdg") == 0) {
              process += CDG_ID;
          }
          else if (strcmp(argv[i], "fdg") == 0) {
              process += FDG_ID;
          }
          else if (strcmp(argv[i], "pdg") == 0) {
              process += PDG_ID;
          }
          else if (strcmp(argv[i], "xpdg") == 0) {
              process += XPDG_ID;
          }
          else {
              std::cerr << "Unrecognized argument: " << argv[i] << ". Exiting!" << std::endl;
              exit( EXIT_FAILURE );
          }
      }
  }

  BPatch_addressSpace *app = bpatch.openBinary(argv[1], true);

  BPatch_image* appImage = app->getImage();

  BPatch_Vector <BPatch_function *> function;
  appImage->findFunction(argv[2], 
			 function);

  std::string str;
  
  if (process & DDG_ID) {
      DDG::Ptr ddg = DDG::analyze(function[0]);
      std::string str(argv[2]);
      str += ".ddg.dot";
      ddg->printDOT(str);
  }

  if (process & CDG_ID) {
      CDG::Ptr cdg = CDG::analyze(function[0]);
      str = std::string(argv[2]);
      str += ".cdg.dot";
      cdg->printDOT(str);
  }

  if (process & FDG_ID) {
      FDG::Ptr fdg = FDG::analyze(function[0]);
      str = std::string(argv[2]);
      str += ".fdg.dot";
      fdg->printDOT(str);
  }

  if (process & PDG_ID) {
      PDG::Ptr pdg = PDG::analyze(function[0]);
      str = std::string(argv[2]);
      str += ".pdg.dot";
      pdg->printDOT(str);
  }

  if (process & XPDG_ID) {
      xPDG::Ptr xpdg = xPDG::analyze(function[0]);
      str = std::string(argv[2]);
      str += ".xpdg.dot";
      xpdg->printDOT(str);
  }
  return EXIT_SUCCESS;
}
