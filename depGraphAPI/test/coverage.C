
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
      DDG::Ptr ddg = DDG::analyze(function[i]);
  }

  return EXIT_SUCCESS;
}
