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

// $Id: test_lib_soExecution.C,v 1.3 2008/06/18 19:58:50 carl Exp $

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

bool getMutatorsForRunGroup(RunGroup *group,
			    std::vector<TestInfo *> &group_tests)
{
  for (int i = 0; i < group->tests.size(); i++) {
    if (false == group->tests[i]->enabled) {
      // Skip disabled tests
      continue;
    }

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
      return true; // Error
    }
    void *handle = dlopen(fullSoPath, RTLD_NOW);
    ::free(fullSoPath);
    if (!handle) {
      fprintf(stderr, "Error opening lib: %s\n", group->tests[i]->soname);
      fprintf(stderr, "%s\n", dlerror());
      return true; // Error
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
      return true; // Error
    }

    TestMutator *mutator = factory();
    if (NULL == mutator) {
      fprintf(stderr, "Error creating new TestMutator for test %s\n",
	      test->name);
    } else {
      test->mutator = mutator;
      group_tests.push_back(test);
    }
  }
  return false; // No error
} // getMutatorsForRunGroup

int loadLibRunTest(test_data_t &testLib, ParameterDict &param)
{
   //printf("Loading test: %s\n", testLib.soname);
   char *fullSoPath;
#if defined(os_aix)
   fullSoPath = searchPath(getenv("LIBPATH"), testLib.soname);
#else
   fullSoPath = searchPath(getenv("LD_LIBRARY_PATH"), testLib.soname);
#endif
   if (!fullSoPath) {
      printf("Error finding lib %s in LD_LIBRARY_PATH/LIBPATH\n",
             testLib.soname);
      return -1;
   }
   void *handle = dlopen(fullSoPath, RTLD_NOW);
   ::free(fullSoPath);

   if (!handle)
   {
      printf("Error opening lib: %s\n", testLib.soname);
      printf("%s\n", dlerror());
      return -1;
   }

   // Parse parameters here, to keep tests simple
//    bool useAttach = param["useAttach"]->getInt();
//    BPatch *bpatch = (BPatch *)(param["bpatch"]->getPtr());
//    BPatch_thread *appThread = (BPatch_thread *)(param["appThread"]->getPtr());
   // Read the program's image and get an associated image object
//    BPatch_image *appImage = appThread->getImage();


//    typedef int (*mutatorM_t)(BPatch_thread *, BPatch_image *, int);
   typedef int (*mutatorM_t)(ParameterDict &);

   typedef TestMutator *(*mutator_factory_t)();

   char mutator_name[256];
//    snprintf(mutator_name, 256, "%s_mutatorTest", testLib.name); 
//    snprintf(mutator_name, 256, "%s_mutatorMAIN", testLib.name);
//    mutatorM_t mutTest = (mutatorM_t) dlsym(handle, mutator_name);
//    if ( !mutTest )
//    {
//       printf("Error finding function: mutatorMAIN, in %s\n", testLib.soname);
//       printf("%s\n", dlerror());
//       dlclose(handle);
//       return -1;
//    }

//    int mutateeFortran = isMutateeFortran(appImage);

//    if ( useAttach ) {
//      if ( ! signalAttached(appThread, appImage) )
//        return -1;
//    }

   snprintf(mutator_name, 256, "%s_factory", testLib.name);
   mutator_factory_t factory = (mutator_factory_t) dlsym(handle, mutator_name);
   if (NULL == factory) {
     printf("Error finding function: %s, in %s\n", mutator_name,
	    testLib.soname);
     printf("%s\n", dlerror());
     dlclose(handle);
     return -1;
   }
     
   // Call function
//    int result = mutTest(appThread, appImage, mutateeFortran);
//    int result = mutTest(param);
   TestMutator *mutator = factory();
   test_results_t result = mutator->setup(param);
   if (PASSED == result) {
     if (mutator->hasCustomExecutionPath()) {
       result = mutator->execute();
     } else {
       result = mutator->preExecution();
       // Do something with inExecution?
       if (PASSED == result) {
	 result = mutator->postExecution();
       }
       if (PASSED == result) {
	 result = mutator->teardown();
       }
     }
   }

   dlclose(handle);

   //printf("Ran test: %s\n", testLib.soname);
   int retval;
   if (FAILED == result) {
     retval = -1;
   } else {
     retval = 0;
   }
   return retval;
}
