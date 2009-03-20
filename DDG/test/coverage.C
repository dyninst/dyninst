//#define LOTSOFDATA

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
#include "intraFunctionCreator.h"

BPatch bpatch;

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

  clock_t loopIterTime = 0;
  clock_t totalTime = 0;
  struct tms time1, time2, time3, time4;
  
  fprintf(stderr, "%d functions\n", function.size());

  times(&time1);
  times(&time3);
  for (unsigned i = 0; i < function.size(); i++) {
      times(&time2);
      loopIterTime += time2.tms_utime - time1.tms_utime;
      intraFunctionDDGCreator creator = intraFunctionDDGCreator::create(function[i]);      
      Dyninst::DDG::Graph::Ptr g = creator.getDDG();
      times(&time1);
  }
  times(&time4);

  fprintf(stderr, "Loop iteration time: %ld\n",
          loopIterTime);
  fprintf(stderr, "init: %ld, inter: %ld, intra: %ld, total: %ld\n",
          intraFunctionDDGCreator::initTime,
          intraFunctionDDGCreator::interTime,
          intraFunctionDDGCreator::intraTime,
          intraFunctionDDGCreator::initTime +
          intraFunctionDDGCreator::interTime + 
          intraFunctionDDGCreator::intraTime);
  fprintf(stderr, "INIT: getInsn %ld, getDefAbsloc %ld, updateDefKill %ld, total %ld\n",
          intraFunctionDDGCreator::initGetInsnTime,
          intraFunctionDDGCreator::initGetDefAbslocTime,
          intraFunctionDDGCreator::initUpdateDefKillTime,
          intraFunctionDDGCreator::initGetInsnTime +
          intraFunctionDDGCreator::initGetDefAbslocTime +
          intraFunctionDDGCreator::initUpdateDefKillTime);

  fprintf(stderr, "DEF SET: alias %ld, precise %ld, else %ld, alias %ld\n",
          intraFunctionDDGCreator::defSetGetAliasTime,
          intraFunctionDDGCreator::defSetPreciseTime,
          intraFunctionDDGCreator::defSetElseTime,
          intraFunctionDDGCreator::defSetAliasTime);


  fprintf(stderr, "INTER: %ld merge, %ld calcOut, %ld out\n",
          intraFunctionDDGCreator::interMergeTime,
          intraFunctionDDGCreator::interCalcOutTime,
          intraFunctionDDGCreator::interOutTime);
          
  fprintf(stderr, "INTRA: %ld getUseDef, %ld createNodes, %ld updateDefSet\n",
          intraFunctionDDGCreator::intraGetUseDef,
          intraFunctionDDGCreator::intraCreateNodes,
          intraFunctionDDGCreator::intraUpdateDefSet);


  fprintf(stderr, "Total time: %ld\n",
          time4.tms_utime - time3.tms_utime);

  return EXIT_SUCCESS;
}
