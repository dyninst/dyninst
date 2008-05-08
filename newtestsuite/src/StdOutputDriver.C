// StdOutputDriver.C
// Implements the standard test_driver output system

#include "TestOutputDriver.h"
#include "StdOutputDriver.h"

#include <map>
#include <string>

#include <stdarg.h>
#include <stdio.h>

StdOutputDriver::StdOutputDriver() : attributes(NULL), streams() {
  streams[STDOUT] = std::string("-");
  streams[STDERR] = std::string("-");
  streams[LOGINFO] = std::string("-");
  streams[LOGERR] = std::string("-");
  streams[HUMAN] = std::string("-");
}

StdOutputDriver::~StdOutputDriver() {
  if (attributes != NULL) {
    delete attributes;
  }
}

void StdOutputDriver::startNewTest(std::map<std::string, std::string> &attrs) {
  if (attributes != NULL) {
    delete attributes;
    attributes = NULL;
  }

  attributes = new std::map<std::string, std::string>(attrs);
}

void StdOutputDriver::redirectStream(TestOutputStream stream, const char *filename) {
  if (streams.find(stream) == streams.end()) {
    fprintf(stderr, "[%s:%u] - StdOutputDriver::redirectStream called with unexpected stream value %d\n", __FILE__, __LINE__, stream);
  } else {
    streams[stream] = std::string(filename);
  }
}

void StdOutputDriver::logResult(test_results_t result) {
  // TODO Finish me.  I can probably copy the final output stuff from
  // test_driver.C into here.

  // I think this just has to print out the human log results
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
  fprintf(out, "%s:\tmutatee: %s.mutatee_solo_%s_%s_%s\tcreate_mode: %s\tresult: ",
	  (*attributes)["test"].c_str(), (*attributes)["mutatee"].c_str(),
	  (*attributes)["compiler"].c_str(),
	  (*attributes)["mutatee_abi"].c_str(),
	  (*attributes)["optimization"].c_str(),
	  (*attributes)["run_mode"].c_str());
  switch(result) {
  case PASSED:
    fprintf(out, "PASSED");
    break;

  case FAILED:
    fprintf(out, "FAILED");
    break;

  case SKIPPED:
    fprintf(out, "SKIPPED");
    break;

  case CRASHED:
    fprintf(out, "CRASHED");
    break;

  default:
    fprintf(out, "UNKNOWN");
  }
  fprintf(out, "\n");

  if ((out != stdout) && (out != stderr)) {
    fclose(out);
  } else {
    fflush(out);
  }
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
