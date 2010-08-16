/*
 * Copyright (c) 1996-2009 Barton P. Miller
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
#ifndef TEST_INFO_NEW_H
#define TEST_INFO_NEW_H

#include <vector>
#include <string>
#include <map>

#include "TestData.h"
#include "test_results.h"

// Avoid stupid circular dependency issue..
class TestMutator;
class ComponentTester;

#define NUM_RUNSTATES 8
typedef enum {
   program_setup_rs = 0,
   test_init_rs,
   test_setup_rs,
   test_execute_rs,
   test_teardown_rs,
   group_setup_rs,
   group_teardown_rs,
   program_teardown_rs
} test_runstate_t;

typedef enum {
   PNone = 0,
   SingleProcess,
   MultiProcess
} test_procstate_t;

typedef enum {
   TNone = 0,
   SingleThreaded,
   MultiThreaded
} test_threadstate_t;

typedef enum {
    StaticLink = 0,
    DynamicLink
} test_linktype_t;

typedef enum
{
        nonPIC = 0,
        PIC
} test_pictype_t;

class TestInfo {
private:
	static int global_max_test_name_length;
public:
  const char *name;
  const char *mutator_name;
  const char *soname;
  const char *label;
  TestMutator *mutator;
  bool serialize_enable;
  // This test has been explicitly disabled, probably by the resumelog system
  bool disabled;
  bool enabled;
  unsigned int index;
  
  test_results_t results[NUM_RUNSTATES];
  bool result_reported;
  
  TESTLIB_DLL_EXPORT static int getMaxTestNameLength();
  TESTLIB_DLL_EXPORT static void setMaxTestNameLength(int newlen);
  TESTLIB_DLL_EXPORT TestInfo(unsigned int i, const char *iname, const char *mrname,
	   const char *isoname, bool _serialize_enable, const char *ilabel);
  TESTLIB_DLL_EXPORT ~TestInfo();
};

class Module;

class RunGroup {
public:
  const char *mutatee;
  start_state_t state;
  create_mode_t useAttach;
  bool customExecution;
  bool selfStart;
  unsigned int index;
  std::vector<TestInfo *> tests;
  bool disabled;
  Module *mod;
  test_threadstate_t threadmode;
  test_procstate_t procmode;
  test_linktype_t linktype;
  test_pictype_t pic;
  const char *compiler;
  const char *optlevel;
  const char *abi;
  
  TESTLIB_DLL_EXPORT RunGroup(const char *mutatee_name, start_state_t state_init,
                              create_mode_t attach_init, 
                              test_threadstate_t threads_, test_procstate_t procs_, 
                              test_linktype_t linktype_,
                              bool ex,
                              test_pictype_t pic_,
                              TestInfo *test_init,
                              const char *modname_, const char *compiler_, const char *optlevel_, 
                              const char *abi_);
  TESTLIB_DLL_EXPORT RunGroup(const char *mutatee_name, start_state_t state_init,
                              create_mode_t attach_init,
                              bool ex,
                              test_pictype_t pic_,
                              const char *modname_,
                              const char *compiler_, const char *optlevel_,
							  const char *abi);
  TESTLIB_DLL_EXPORT RunGroup(const char *mutatee_name, start_state_t state_init,
                              create_mode_t attach_init, 
                              test_threadstate_t threads_, test_procstate_t procs_, 
                              test_linktype_t linktype_,
                              bool ex,
                              test_pictype_t pic_,
                              const char *modname_,
                              const char *compiler_, const char *optlevel_, 
                              const char *abi_);
  TESTLIB_DLL_EXPORT ~RunGroup();
};

class Module {
   bool creation_error;
   bool initialized;
   bool setup_run;
   static std::map<std::string, Module *> allmods;

   Module(std::string name_);
   ComponentTester *loadModuleLibrary();
   void *libhandle;
public:
   std::string name;
   ComponentTester *tester;
   std::vector<RunGroup *> groups;

   TESTLIB_DLL_EXPORT bool isInitialized();
   TESTLIB_DLL_EXPORT void setInitialized(bool result);

   TESTLIB_DLL_EXPORT bool setupRun();
   TESTLIB_DLL_EXPORT void setSetupRun(bool result);

   TESTLIB_DLL_EXPORT static bool registerGroupInModule(std::string modname, RunGroup *group);
   TESTLIB_DLL_EXPORT static void getAllModules(std::vector<Module *> &mods);
};

//extern std::vector<RunGroup *> tests;

extern void initialize_mutatees(std::vector<RunGroup *> &tests);

#endif // !defined(TEST_INFO_NEW_H)
