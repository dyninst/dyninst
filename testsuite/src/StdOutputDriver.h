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
// StdOutputDriver.h
// Implements the standard test_driver output system

#include "TestOutputDriver.h"

#include <map>
#include <string>

class StdOutputDriver : public TestOutputDriver {
private:
  std::map<TestOutputStream, std::string> streams;
  std::map<std::string, std::string> *attributes;
  TestInfo *last_test;
  RunGroup *last_group;

  //Column widths
  static const int name_len = 26;
  static const int compiler_len = 6;
  static const int opt_len = 4;
  static const int abi_len = 3;
  static const int mode_len = 8;
  static const int thread_len = 7;
  static const int link_len = 7;
  static const int pic_len = 7;
  static const int pmode_len = 5;

  bool printed_header;
  void printHeader(FILE *out);
  public:
  TESTLIB_DLL_EXPORT StdOutputDriver(void * data);
  ~StdOutputDriver();

  virtual void startNewTest(std::map<std::string, std::string> &attributes, TestInfo *test, RunGroup *group);

  virtual void redirectStream(TestOutputStream stream, const char * filename);
  virtual void logResult(test_results_t result, int stage=-1);
  virtual void logCrash(std::string testname);
  virtual void log(TestOutputStream stream, const char *fmt, ...);
  virtual void vlog(TestOutputStream stream, const char *fmt, va_list args);
  virtual void finalizeOutput();
};
