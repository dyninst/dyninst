//
// Created by bill on 7/6/16.
//

#ifndef DYNINST_TESTSUITE_JUNITOUTPUTDRIVER_H
#define DYNINST_TESTSUITE_JUNITOUTPUTDRIVER_H

#include "StdOutputDriver.h"
#include <sstream>

class JUnitOutputDriver : public StdOutputDriver
{
public:
    TESTLIB_DLL_EXPORT JUnitOutputDriver(void* data);
    TESTLIB_DLL_EXPORT virtual ~JUnitOutputDriver();

    // Informs the output driver that any log messages or results should be
    // associated with the test passed in through the attributes parameter
    virtual void startNewTest(std::map<std::string, std::string> &attributes, TestInfo *test, RunGroup *group);


    // Before calling any of the log* methods or finalizeOutput(), the user
    // must have initialized the test output driver with a call to startNewTest()

    virtual void logResult(test_results_t result, int stage=-1);
    virtual void finalizeOutput();
    virtual void vlog(TestOutputStream stream, const char *fmt, va_list args);

private:
    int group_failures;
    int group_skips;
    int group_errors;
    int group_tests;
    std::stringstream group_output;
    std::stringstream failure_log;
};


#endif //DYNINST_TESTSUITE_JUNITOUTPUTDRIVER_H
