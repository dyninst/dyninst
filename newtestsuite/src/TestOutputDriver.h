// TestOutputDriver.h
// This file defines an object that produces output for the test driver.

#ifndef TEST_OUTPUT_DRIVER_H
#define TEST_OUTPUT_DRIVER_H

#include <map>
#include <string>
#include <vector>

#include <stdarg.h>

#include "test_info_new.h"
#include "test_results.h"

typedef enum {
  STDOUT,
  STDERR,
  LOGINFO,
  LOGERR,
  HUMAN
} TestOutputStream;

class TestOutputDriver {
public:
  // FIXME Should the attributes parameter be a reference or a pointer?
  // Heck, can I get everything I need from the RunGroup and TestInfo
  // structures?  Then I wouldn't need to create a map here.  Maybe a
  // convenience method to transform those structures into a map, because
  // I'm pretty sure a map will be easier to produce output from.
  TESTLIB_DLL_EXPORT static std::map<std::string, std::string> *getAttributesMap(TestInfo *test, RunGroup *group);

  // Informs the output driver that any log messages or results should be
  // associated with the test passed in through the attributes parameter
  virtual void startNewTest(std::map<std::string, std::string> &attributes) = 0;

  // Specifies a file to redirect one of the output streams to.  The default
  // file can be specified with a filename of "-".  Defaults are as follows:
  // STDOUT, LOGINFO, HUMAN -> stdout
  // STDERR, LOGERR -> stderr
  virtual void redirectStream(TestOutputStream stream, const char * filename) = 0;

  // Before calling any of the log* methods or finalizeOutput(), the user
  // must have initialized the test output driver with a call to startNewTest()

  virtual void logResult(test_results_t result) = 0;
  // Log that the last test run by a test driver with pid crashedPID crashed
  virtual void logCrash(std::string testname) = 0;
  virtual void log(TestOutputStream stream, const char *fmt, ...) = 0;
  // Like the vprintf() family, vlog() does not call the va_end() macro, so
  // its caller should do so after the call to vlog().
  virtual void vlog(TestOutputStream stream, const char *fmt, va_list args) = 0;
  virtual void finalizeOutput() = 0;

  // Returns arguments to pass to the mutatee driver that cause it to invoke
  // its support for this output driver
  TESTLIB_DLL_EXPORT virtual void getMutateeArgs(std::vector<std::string> &args);
};

#endif // TEST_OUTPUT_DRIVER_H
