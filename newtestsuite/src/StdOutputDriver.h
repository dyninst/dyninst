// StdOutputDriver.h
// Implements the standard test_driver output system

#include "TestOutputDriver.h"

#include <map>
#include <string>

class StdOutputDriver : public TestOutputDriver {
private:
  std::map<TestOutputStream, std::string> streams;
  std::map<std::string, std::string> *attributes;

public:
  StdOutputDriver();
  ~StdOutputDriver();

  virtual void startNewTest(std::map<std::string, std::string> &attributes);

  virtual void redirectStream(TestOutputStream stream, const char * filename);
  virtual void logResult(test_results_t result);
  virtual void logCrash(std::string testname);
  virtual void log(TestOutputStream stream, const char *fmt, ...);
  virtual void vlog(TestOutputStream stream, const char *fmt, va_list args);
  virtual void finalizeOutput();
};
