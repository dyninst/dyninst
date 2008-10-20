/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: test_lib_soExecution.C,v 1.4 2008/10/20 20:37:27 legendre Exp $

#include <sstream>

#include <dlfcn.h>
#include <stdio.h>

#include "test_lib.h"
#include "ParameterDict.h"

#include "TestOutputDriver.h"
#include "TestMutator.h"
#include "test_info_new.h"

TESTLIB_DLL_EXPORT TestOutputDriver *loadOutputDriver(char *odname, void * data) {
  std::stringstream fname;
  fname << odname << ".so";

  void *odhandle = dlopen(fname.str().c_str(), RTLD_NOW);
  if (NULL == odhandle) {
    fprintf(stderr, "[%s:%u] - Error loading output driver: '%s'\n", __FILE__, __LINE__, dlerror());
    return NULL;
  }

  TestOutputDriver *(*factory)(void *);
  dlerror();
  factory = (TestOutputDriver *(*)(void *)) dlsym(odhandle, "outputDriver_factory");
  char *errmsg = dlerror();
  if (errmsg != NULL) {
    // TODO Handle error
    fprintf(stderr, "[%s:%u] - Error loading output driver: '%s'\n", __FILE__, __LINE__, errmsg);
    return NULL;
  }

  TestOutputDriver *retval = factory(data);

  return retval;
}

int setupMutatorsForRunGroup(RunGroup *group)
{
  int tests_found = 0;
  for (int i = 0; i < group->tests.size(); i++) {
    if (group->tests[i]->disabled)
       continue;

    char *fullSoPath;
#if defined(os_aix)
    TestInfo *test = group->tests[i];
    const char *soname = test->soname;
    fullSoPath = searchPath(getenv("LIBPATH"), group->tests[i]->soname);
#else
    TestInfo *test = group->tests[i];
    const char *soname = test->soname;
    fullSoPath = searchPath(getenv("LD_LIBRARY_PATH"), soname);
#endif
    if (!fullSoPath) {
      fprintf(stderr, "Error finding lib %s in LD_LIBRARY_PATH/LIBPATH\n",
	      soname);
      return -1; // Error
    }
    void *handle = dlopen(fullSoPath, RTLD_NOW);
    ::free(fullSoPath);
    if (!handle) {
      fprintf(stderr, "Error opening lib: %s\n", group->tests[i]->soname);
      fprintf(stderr, "%s\n", dlerror());
      return -1; //Error
    }

    typedef TestMutator *(*mutator_factory_t)();
    char mutator_name[256];
    const char *testname = test->mutator_name;
    snprintf(mutator_name, 256, "%s_factory", testname);
    mutator_factory_t factory = (mutator_factory_t) dlsym(handle,
							  mutator_name);
    if (NULL == factory) {
      fprintf(stderr, "Error funding function: %s, in %s\n", mutator_name,
	      soname);
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

