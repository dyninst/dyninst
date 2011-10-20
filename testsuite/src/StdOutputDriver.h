/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

#if !defined(STD_OUTPUT_DRIVER_H)
#define STD_OUTPUT_DRIVER_H

#include "TestOutputDriver.h"

#include <map>
#include <string>

class TESTLIB_DLL_EXPORT StdOutputDriver : public TestOutputDriver {
protected:
  std::map<TestOutputStream, std::string> streams;
  std::map<std::string, std::string> *attributes;
  TestInfo *last_test;
  RunGroup *last_group;

public:
  StdOutputDriver(void * data);
  ~StdOutputDriver();

  virtual void startNewTest(std::map<std::string, std::string> &attributes, TestInfo *test, RunGroup *group);

  virtual void redirectStream(TestOutputStream stream, const char * filename);
  virtual void logResult(test_results_t result, int stage=-1);
  virtual void logCrash(std::string testname);
  virtual void log(TestOutputStream stream, const char *fmt, ...);
  virtual void vlog(TestOutputStream stream, const char *fmt, va_list args);
  virtual void finalizeOutput();
};

#endif // !defined(STD_OUTPUT_DRIVER_H)
