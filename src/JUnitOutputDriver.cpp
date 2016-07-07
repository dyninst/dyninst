//
// Created by bill on 7/6/16.
//

#include "JUnitOutputDriver.h"

JUnitOutputDriver::JUnitOutputDriver(void *data) : StdOutputDriver(data),
                                                   group_failures(0),
                                                   group_skips(0),
                                                   group_errors(0),
                                                   group_tests(0) {
    streams[HUMAN] = "test_results.xml";

        log(HUMAN, "<testsuites>\n");
}

JUnitOutputDriver::~JUnitOutputDriver() {
    log(HUMAN, "</testsuites>");
    FILE* human = getHumanFile();
    fflush(human);
    if(human != stdout) fclose(human);
}

// Informs the output driver that any log messages or results should be
// associated with the test passed in through the attributes parameter
void JUnitOutputDriver::startNewTest(std::map <std::string, std::string> &attributes, TestInfo *test, RunGroup *group) {
    if (group != last_group) {
        group_failures = 0;
        group_skips = 0;
        group_errors= 0;
        group_tests = 0;

    }
    StdOutputDriver::startNewTest(attributes, test, group);
}


// Before calling any of the log* methods or finalizeOutput(), the user
// must have initialized the test output driver with a call to startNewTest()

void JUnitOutputDriver::logResult(test_results_t result, int stage)
{



    group_output << "<testcase classname=\"" << last_group->modname.c_str();
    if(last_group->mutatee != "") group_output << "." << last_group->mutatee;
    group_output << "\" name=\"" << last_test->name << "\"";

    if (last_test && last_test->usage.has_data()) {
        float cpu = last_test->usage.cpuUsage().tv_sec + last_test->usage.cpuUsage().tv_usec / 1000000;
        group_output << " time=\"" << cpu << "\"";
    }
    group_tests++;
    switch (result) {
        case PASSED:
            group_output << "/>\n";
            break;

        case FAILED:
            group_output << ">\n<failure>Test failed</failure>\n";
            group_failures++;
            group_output << "</testcase>";
            break;

        case SKIPPED:
            group_skips++;
            group_output << ">\n<skipped />\n";
            group_output << "</testcase>";
            break;

        case CRASHED:
            group_errors++;
            group_output << ">\n<error>Test crashed</error>\n";
            group_output << "</testcase>";
            break;

        default:
            group_errors++;
            group_output << ">\n<error>Testsuite internal error, unknown result</error>\n";
            group_output << "</testcase>\n";
            break;
            // do nothing
    }

}
void JUnitOutputDriver::finalizeOutput()
{
    std::stringstream suitename;
    suitename << last_group->modname;
    if(last_group->mutatee != '\0') suitename << "." << last_group->mutatee;
    log(HUMAN, "<testsuite name=\"%s\" errors=\"%d\" skipped=\"%d\" tests=\"%d\" failures=\"%d\">\n",
        suitename.str().c_str(), group_errors, group_skips, group_tests, group_failures);
    log(HUMAN, group_output.str().c_str());
    log(HUMAN, "</testsuite>\n");
    log(HUMAN, "</testsuites>");
    FILE* human = getHumanFile();
    fflush(human);
    if(human != stdout) fclose(human);
}


