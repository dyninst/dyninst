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

#include <sstream>
#include <string>
#include <vector>

#include "TestOutputDriver.h"

#include "test_results.h"
#include "test_lib.h"

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
  TestInfo *currTest;
  test_results_t result;
  // Stores any output before startNewTest is first called
  std::stringstream pretestLog;

  void writeSQLLog();

public:
  DatabaseOutputDriver(void * data);
  ~DatabaseOutputDriver();

  virtual void startNewTest(std::map<std::string, std::string> &attrs, TestInfo *test, RunGroup *group);
  virtual void redirectStream(TestOutputStream stream, const char * filename);
  virtual void logResult(test_results_t result, int stage=-1);
  virtual void logCrash(std::string testname);
  virtual void log(TestOutputStream stream, const char *fmt, ...);
  virtual void vlog(TestOutputStream stream, const char *fmt, va_list args);
  virtual void finalizeOutput();
  virtual void getMutateeArgs(std::vector<std::string> &args);
};

extern "C" {
	DLLEXPORT TestOutputDriver *outputDriver_factory(void * data);
}
