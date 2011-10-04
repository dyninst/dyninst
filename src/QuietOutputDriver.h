#include "StdOutputDriver.h"
#include <string.h>

class QuietOutputDriver : public StdOutputDriver
{
 public:
 QuietOutputDriver(void* data) : StdOutputDriver(data), successCount(0),
  skipCount(0)
  {
  }
  virtual ~QuietOutputDriver() 
  {
    FILE* out = getOutFile();
    fprintf(out, "\nResults: %d passed, %d skipped, %d failed, %d crashed (%d total)\n",
	    successCount, skipCount, failures.size(), crashes.size());
    if(!crashes.empty())
    {
      fprintf(out, "*** CRASH LIST ***\n");
      for(std::vector<std::string>::iterator c = crashes.begin(); c != crashes.end(); ++c)
      {
	fprintf(out, "%s\n", c->c_str());
      }
    }
    if(!failures.empty())
    {
      fprintf(out, "*** FAILURE LIST ***\n");
      for(std::vector<std::string>::iterator f = failures.begin(); f != failures.end(); ++f)
      {
	fprintf(out, "%s\n", f->c_str());
      }
    }
  }
  
  FILE* getOutFile() 
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
    return out;
  }
  
  
  virtual void logResult(test_results_t result, int stage=-1);
 private:
  std::vector<std::string> failures;
  std::vector<std::string> crashes;
  int successCount;
  int skipCount;
  
};

