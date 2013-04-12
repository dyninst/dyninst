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

// $Id: test_lib_soExecution.C,v 1.5 2008/10/30 19:16:58 legendre Exp $

#include <sstream>

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#include "test_lib.h"
#include "ParameterDict.h"

#include "TestOutputDriver.h"
#include "TestMutator.h"
#include "test_info_new.h"
#include "comptester.h"
#include "module.h"

TESTLIB_DLL_EXPORT TestOutputDriver *loadOutputDriver(char *odname, void * data) {
  std::stringstream fname;
  fname  <<  odname << ".so";

  void *odhandle = dlopen(fname.str().c_str(), RTLD_NOW);
  if (NULL == odhandle) {
     odhandle = dlopen(("./" + fname.str()).c_str(), RTLD_NOW);
  }
  if (NULL == odhandle) {
    fprintf(stderr, "[%s:%u] - Error loading output driver: '%s'\n", __FILE__, __LINE__, dlerror());
    return NULL;
  }

  TestOutputDriver *(*factory)(void *);
  dlerror();
  factory = (TestOutputDriver *(*)(void *)) dlsym(odhandle, "outputDriver_factory");
  const char *errmsg = const_cast<const char *>(dlerror());
  if (errmsg != NULL) {
    // TODO Handle error
    fprintf(stderr, "[%s:%u] - Error loading output driver: '%s'\n", __FILE__, __LINE__, errmsg);
    return NULL;
  }

  TestOutputDriver *retval = factory(data);

  return retval;
}

#include <link.h>

static void* openSO(const char *soname, bool local)
{
   char *fullSoPath = NULL;
#if defined(os_aix_test)
   fullSoPath = searchPath(getenv("LIBPATH"), soname);
#else
   fullSoPath = searchPath(getenv("LD_LIBRARY_PATH"), soname);
#endif
   if (getDebugLog()) {
      fprintf(getDebugLog(), "openSO: search path is %s\n", fullSoPath ? fullSoPath : "NULL");
   }
   
   if (!fullSoPath) {
      fullSoPath = strdup(soname);
   }
   unsigned int dl_options = RTLD_NOW | (local ? RTLD_LOCAL : RTLD_GLOBAL);
   void *handle = dlopen(fullSoPath, dl_options);
   if (!handle) {
      fprintf(stderr, "Error opening lib: %s\n", soname);
      const char *errmsg = dlerror();
      fprintf(stderr, "%s\n", errmsg);
      std::string str = std::string("./") + std::string(soname);
      fprintf(stderr, "Error loading library: %s\n", dlerror());
      handle = dlopen(str.c_str(), dl_options);
   }
   ::free(fullSoPath);
   if (!handle) {
      fprintf(stderr, "Error opening lib: %s\n", soname);
      const char *errmsg = dlerror();
      fprintf(stderr, "%s\n", errmsg);
      return NULL; //Error
   }

   return handle;
}

int setupMutatorsForRunGroup(RunGroup *group)
{
  int tests_found = 0;
  for (int i = 0; i < group->tests.size(); i++) {
    TestInfo *test = group->tests[i];
    if (test->disabled)
       continue;
    if (test->mutator)
       continue;
    
    std::string soname = "lib";
    soname += test->soname;
    
    void *handle = openSO(soname.c_str(), true);
    if (!handle) {
       getOutput()->log(STDERR, "Couldn't open %s\n", soname.c_str());
       return -1;
    }

    typedef TestMutator *(*mutator_factory_t)();
    char mutator_name[256];
    const char *testname = test->mutator_name;
    snprintf(mutator_name, 256, "%s_factory", testname);
    mutator_factory_t factory = (mutator_factory_t) dlsym(handle,
							  mutator_name);
    if (NULL == factory) {
      fprintf(stderr, "Error finding function: %s, in %s\n", mutator_name,
	      soname.c_str());
      fprintf(stderr, "%s\n", dlerror());
      dlclose(handle);
      return -1; //Error
    }

    TestMutator *mutator = factory();
    if (NULL == mutator) {
      fprintf(stderr, "Error creating new TestMutator for test %s\n",
	      test->name);
    } else {
      test->mutator = mutator;
      tests_found++;
    }
  }
  return tests_found; // No error
} // setupMutatorsForRunGroup

typedef ComponentTester* (*comptester_factory_t)();
ComponentTester *Module::loadModuleLibrary()
{
   libhandle = NULL;
   char libname[256];
#if defined(os_aix_test)
   snprintf(libname, 256, "libtest%s.a", name.c_str());
#else   
   snprintf(libname, 256, "libtest%s.so", name.c_str());
#endif
   libhandle = openSO(libname, false);
   if (!libhandle) {
      fprintf(stderr, "Error loading library: %s\n", dlerror());
      return NULL;
   }

   comptester_factory_t factory;
   factory = (comptester_factory_t) dlsym(libhandle, "componentTesterFactory");
   if (!factory)
   {
      fprintf(stderr, "Error finding componentTesterFactory\n");
      return NULL;
   }

   return factory();
}
