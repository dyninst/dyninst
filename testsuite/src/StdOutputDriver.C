/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
// StdOutputDriver.C
// Implements the standard test_driver output system

#include "TestOutputDriver.h"
#include "StdOutputDriver.h"
#include "test_info_new.h"

#include <map>
#include <string>

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

StdOutputDriver::StdOutputDriver(void * data) : attributes(NULL), streams() {
  streams[STDOUT] = std::string("-");
  streams[STDERR] = std::string("-");
  streams[LOGINFO] = std::string("-");
  streams[LOGERR] = std::string("-");
  streams[HUMAN] = std::string("-");
  last_test = NULL;
  last_group = NULL;
  printed_header = false;
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

void StdOutputDriver::printHeader(FILE *out) {
   if (printed_header)
      return;
   printed_header = true;
#if defined (os_bgp_test)
   fprintf(out, "%-*s %-*s %-*s %-*s %-*s %-*s %-*s %-*s %s\n", 
           name_len, "TEST", 
           compiler_len, "COMP", 
           opt_len, "OPT", 
           mode_len, "MODE", 
           thread_len, "THREAD", 
           link_len, "LINK", 
           pic_len, "PIC",
           pmode_len, "PMODE",
           "RESULT");
#elif defined(cap_32_64_test)
   fprintf(out, "%-*s %-*s %-*s %-*s %-*s %-*s %-*s %-*s %s\n", 
           name_len, "TEST", 
           compiler_len, "COMP", 
           opt_len, "OPT", 
           abi_len, "ABI", 
           mode_len, "MODE", 
           thread_len, "THREAD", 
           link_len, "LINK", 
           pic_len, "PIC",
           "RESULT");
#else
   fprintf(out, "%-*s %-*s %-*s %-*s %-*s %-*s %-*s %s\n", 
           name_len, "TEST", 
           compiler_len, "COMP", 
           opt_len, "OPT", 
           mode_len, "MODE", 
           thread_len, "THREAD", 
           link_len, "LINK", 
           pic_len, "PIC",
           "RESULT");
#endif
}

#define MAX_PRINTED_TESTNAME_LEN 18
void StdOutputDriver::logResult(test_results_t result, int stage) {
   // This just has to print out the human log results
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
   else if (strcmp(orig_run_mode_str, "binary") == 0)
      run_mode_str = "rewriter";
   else
      run_mode_str = orig_run_mode_str;

   const char *linkage_str = NULL;
   if( (*attributes)["format"] == std::string("staticMutatee") )
       linkage_str = "static";
   else
       linkage_str = "dynamic";

   char thread_str[5];
   if (last_group->threadmode == TNone && last_group->procmode == PNone) {
      strncpy(thread_str, "NA", 5);
   }
   else {
      if (last_group->procmode == SingleProcess)
         thread_str[0] = 'S';
      else if (last_group->procmode == MultiProcess)
         thread_str[0] = 'M';
      else
         thread_str[0] = 'N';
      thread_str[1] = 'P';
      if (last_group->threadmode == SingleThreaded)
         thread_str[2] = 'S';
      else if (last_group->threadmode == MultiThreaded)
         thread_str[2] = 'M';
      else
         thread_str[2] = 'N';
      thread_str[3] = 'T';
      thread_str[4] = '\0';
   }
   const char* picStr = NULL;
   if(last_group->pic == nonPIC)
   {
       picStr = "nonPIC";
   } else
   {
       picStr = "PIC";
   }
   
   assert(last_test && last_group);

   char name_align_buffer[name_len+1];
   name_align_buffer[name_len] = '\0';
   strncpy(name_align_buffer, last_test->name, name_len);

   if (needs_header)
      printHeader(out);
#if defined(os_bgp_test)
   fprintf(out, "%-*s %-*s %-*s %-*s %-*s %-*s %-*s %-*s ",
           name_len, name_align_buffer, 
           compiler_len, last_group->compiler,
           opt_len, last_group->optlevel, 
           mode_len, run_mode_str, 
           thread_len, thread_str, 
           link_len, linkage_str, 
           pic_len, picStr,
           pmode_len, last_group->platmode);
#elif defined(cap_32_64_test)
   fprintf(out, "%-*s %-*s %-*s %-*s %-*s %-*s %-*s %-*s ", 
           name_len, name_align_buffer, 
           compiler_len, last_group->compiler,
           opt_len, last_group->optlevel, 
           abi_len, last_group->abi, 
           mode_len, run_mode_str, 
           thread_len, thread_str, 
           link_len, linkage_str, 
           pic_len, picStr);
#else
   fprintf(out, "%-*s %-*s %-*s %-*s %-*s %-*s %-*s ", 
           name_len, name_align_buffer, 
           compiler_len, last_group->compiler,
           opt_len, last_group->optlevel, 
           mode_len, run_mode_str, 
           thread_len, thread_str, 
           link_len, linkage_str, 
           pic_len, picStr);
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

   if (last_test && last_test->usage.has_data()) {
       fprintf(out, " (CPU: %ld.%06ld MEMORY: %ld)",
               last_test->usage.cpuUsage().tv_sec,
               last_test->usage.cpuUsage().tv_usec,
               last_test->usage.memUsage());
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
