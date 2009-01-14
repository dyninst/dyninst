// StdOutputDriver.C
// Implements the standard test_driver output system

#include "TestOutputDriver.h"
#include "StdOutputDriver.h"

#include <map>
#include <string>

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

StdOutputDriver::StdOutputDriver(void * data) : attributes(NULL), streams() {
  streams[STDOUT] = std::string("-");
  streams[STDERR] = std::string("-");
  streams[LOGINFO] = std::string("-");
  streams[LOGERR] = std::string("-");
  streams[HUMAN] = std::string("-");
  last_test = NULL;
  last_group = NULL;
}

StdOutputDriver::~StdOutputDriver() {
  if (attributes != NULL) {
    delete attributes;
  }
}

void StdOutputDriver::startNewTest(std::map<std::string, std::string> &attrs, TestInfo *test, RunGroup *group) {
  if (attributes != NULL) {
    delete attributes;
    attributes = NULL;
  }
  last_test = test;
  last_group = group;

  attributes = new std::map<std::string, std::string>(attrs);
}

void StdOutputDriver::redirectStream(TestOutputStream stream, const char *filename) {
  if (streams.find(stream) == streams.end()) {
    fprintf(stderr, "[%s:%u] - StdOutputDriver::redirectStream called with unexpected stream value %d\n", __FILE__, __LINE__, stream);
  } else {
    streams[stream] = std::string(filename);
  }
}

void StdOutputDriver::logResult(test_results_t result, int stage) {
   // TODO Finish me.  I can probably copy the final output stuff from
   // test_driver.C into here.
   
   // I think this just has to print out the human log results
   bool print_stage = false;
   const char *outfn = streams[HUMAN].c_str();
   FILE *out;
   if (strcmp(outfn, "-") == 0) {
      out = stdout;
   } else {
      out = fopen(outfn, "a");
      if (NULL == out) {
         out = stdout;
      }
   }
   
   // Now print out a summary results line
   const char *run_mode_str;
   const char *orig_run_mode_str = (*attributes)["run_mode"].c_str();
   if (strcmp(orig_run_mode_str, "createProcess") == 0)
      run_mode_str = "create";
   else if (strcmp(orig_run_mode_str, "useAttach") == 0)
      run_mode_str = "attach";
   else
      run_mode_str = orig_run_mode_str;
   assert(last_test && last_group);
#if defined(cap_32_64_test)
   fprintf(out, "%s:\tcompiler: %s\tabi: %s\tmode: %s\tresult: ",
           last_test->name, last_group->compiler, last_group->abi, run_mode_str);
#else
   fprintf(out, "%s:\tcompiler: %s\tmode: %s\tresult: ",
           last_test->name, last_group->compiler, run_mode_str);
#endif
   switch(result) {
      case PASSED:
         fprintf(out, "PASSED");
         break;
         
      case FAILED:
         fprintf(out, "FAILED");
	 print_stage = true;
         break;
         
      case SKIPPED:
         fprintf(out, "SKIPPED");
         break;
         
      case CRASHED:
         fprintf(out, "CRASHED");
	 print_stage = true;
         break;

      default:
         fprintf(out, "UNKNOWN");
   }
   
   if (print_stage && stage != -1)
   {
     switch ( (test_runstate_t) stage)
     {
     case program_setup_rs:
       fprintf(out, " (Module Setup)");
       break;
     case group_setup_rs:
       fprintf(out, " (Group Setup)");
       break;
     case group_teardown_rs:
       fprintf(out, " (Group Teardown)");
       break;
     case test_init_rs:
       fprintf(out, " (Test Init)");
       break;
     case test_setup_rs:
       fprintf(out, " (Test Setup)");
       break;
     case test_execute_rs:
       fprintf(out, " (Running Test)");
       break;
     case test_teardown_rs:
       fprintf(out, " (Test Teardown)");
       break;
     default:
       fprintf(out, "\nUnknown test state: %d\n", stage);
       assert(0);
       break;
     }
   }

   fprintf(out, "\n");
   
   if ((out != stdout) && (out != stderr)) {
      fclose(out);
   } else {
      fflush(out);
   }
   last_group = NULL;
   last_test = NULL;
}

void StdOutputDriver::logCrash(std::string testname) {
  // TODO Do something here
}

void StdOutputDriver::log(TestOutputStream stream, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vlog(stream, fmt, args);
  va_end(args);
}

void StdOutputDriver::vlog(TestOutputStream stream, const char *fmt, va_list args) {
  // If stream is an invalid stream, return
  if (streams.find(stream) == streams.end()) {
    fprintf(stderr, "[%s:%u] - StdOutputDriver::log called with unexpected stream value %d\n", __FILE__, __LINE__, stream);
    return;
  }

  // If the stream has been redirected to NULL, ignore this output
  if (NULL == streams[stream].c_str()) {
    return;
  }

  const char *fn = streams[stream].c_str();
  FILE *out;
  if (strcmp(fn, "-") == 0) {
    // We're printing to the default file
    switch(stream) {
    case STDOUT:
    case LOGINFO:
    case HUMAN:
      out = stdout;
      break;

    case STDERR:
    case LOGERR:
      out = stderr;
      break;
    }
  } else {
    // Open the file
    out = fopen(fn, "a");
    if (NULL == out) {
      // TODO Handle this error
      return;
    }
  }

  vfprintf(out, fmt, args);

  if ((out != stdout) && (out != stderr)) {
    fclose(out);
  }
}

void StdOutputDriver::finalizeOutput() {
  // I don't think this method needs to do anything for StdOutputDriver
}
