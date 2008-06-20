#include <sstream>
#include <string>
#include <vector>

#include "TestOutputDriver.h"

#include "test_results.h"

// FIXME Magic numbers..  These should be changed to test_driver parameters
#define DB_HOSTNAME "bryce.cs.umd.edu"
#define DB_USERNAME "editdata"
#define DB_PASSWD "editdata"
#define DB_DBNAME "testDB"
#define DB_PORT 0
#define DB_UXSOCKET NULL
#define DB_CLIENTFLAG 0


class DatabaseOutputDriver : public TestOutputDriver {
private:
  std::string dblogFilename;
  std::string sqlLogFilename;
  std::map<std::string, std::string> *attributes;
  bool wroteLogHeader;
  bool submittedResults;
  test_results_t result;

  // Stores any output before startNewTest is first called
  std::stringstream pretestLog;

  void writeSQLLog();

public:
  DatabaseOutputDriver(void * data);
  ~DatabaseOutputDriver();

  virtual void startNewTest(std::map<std::string, std::string> &attrs);
  virtual void redirectStream(TestOutputStream stream, const char * filename);
  virtual void logResult(test_results_t result);
  virtual void logCrash(std::string testname);
  virtual void log(TestOutputStream stream, const char *fmt, ...);
  virtual void vlog(TestOutputStream stream, const char *fmt, va_list args);
  virtual void finalizeOutput();
  virtual void getMutateeArgs(std::vector<std::string> &args);
};

extern "C" {
extern TEST_DLL_EXPORT TestOutputDriver *outputDriver_factory(void * data);
}
