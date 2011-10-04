

#include "QuietOutputDriver.h"
#include <sstream>
#include <iostream>
#include <stdio.h>

void QuietOutputDriver::logResult(test_results_t result, int stage)
{
  const char* outfn = streams[HUMAN].c_str();
  FILE* out;
  if(strcmp(outfn, "-") == 0) 
  {
    out = stdout;
  }
  else
  {
    out = fopen(outfn, "a");
    if(NULL == out) {
      out = stdout;
    }
  }
  
  switch(result)
  {
  case PASSED:
    fprintf(out, ".");
    successCount++;
    break;
  case FAILED:
    {
      fprintf(out, "F");
      std::stringstream failureMessage;
      failureMessage << last_test->name << std::endl;
      failureMessage << "\tRun mode: " << (*attributes)["run_mode"] << std::endl;
      failures.push_back(failureMessage.str());
    }
    break;
  case CRASHED:
    {
      fprintf(out, "C");
      std::stringstream failureMessage;
      failureMessage << last_test->name << std::endl;
      failureMessage << "\tRun mode: " << (*attributes)["run_mode"] << std::endl;
      crashes.push_back(failureMessage.str());
    }
    break;
  case SKIPPED:
    fprintf(out, "S");
    skipCount++;
    break;
  default:
    fprintf(out, "U");
    break;
  }
}
