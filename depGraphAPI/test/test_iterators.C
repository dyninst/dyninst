
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
