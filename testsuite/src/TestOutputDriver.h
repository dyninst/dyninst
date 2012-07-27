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
protected:
   bool needs_header;
public:
   TESTLIB_DLL_EXPORT virtual ~TestOutputDriver();
   TESTLIB_DLL_EXPORT static bool getAttributesMap(TestInfo *test, 
                        RunGroup *group, std::map<std::string, std::string> &attrs);

  // Informs the output driver that any log messages or results should be
  // associated with the test passed in through the attributes parameter
  virtual void startNewTest(std::map<std::string, std::string> &attributes, TestInfo *test, RunGroup *group) = 0;

  // Specifies a file to redirect one of the output streams to.  The default
  // file can be specified with a filename of "-".  Defaults are as follows:
  // STDOUT, LOGINFO, HUMAN -> stdout
  // STDERR, LOGERR -> stderr
  virtual void redirectStream(TestOutputStream stream, const char * filename) = 0;

  // Before calling any of the log* methods or finalizeOutput(), the user
  // must have initialized the test output driver with a call to startNewTest()

  virtual void logResult(test_results_t result, int stage=-1) = 0;

  // Log that the last test run by a test driver with pid crashedPID crashed
  virtual void logCrash(std::string testname) = 0;
  virtual void log(TestOutputStream stream, const char *fmt, ...) = 0;
  // Like the vprintf() family, vlog() does not call the va_end() macro, so
  // its caller should do so after the call to vlog().
  virtual void vlog(TestOutputStream stream, const char *fmt, va_list args) = 0;
  virtual void finalizeOutput() = 0;

  TESTLIB_DLL_EXPORT void setNeedsHeader(bool h);
  // Returns arguments to pass to the mutatee driver that cause it to invoke
  // its support for this output driver
  TESTLIB_DLL_EXPORT virtual void getMutateeArgs(std::vector<std::string> &args);
};

#endif // TEST_OUTPUT_DRIVER_H
