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

// $Id: test_lib_dllExecution.C,v 1.2 2008/06/18 19:58:28 carl Exp $
#include "test_lib.h"
#include "ParameterDict.h"
#include "TestOutputDriver.h"
#include <windows.h>
#include <stdlib.h>
#include <string.h>

typedef int (*mutatorM_t)(ParameterDict &);
const char* ext = ".dll";

TESTLIB_DLL_EXPORT TestOutputDriver *loadOutputDriver(char *odname, void * data) {
	std::string fname (odname);
	fname += ext;

//	void *odhandle = dlopen(fname.str().c_str(), RTLD_NOW);
	HINSTANCE odhandle = LoadLibrary(fname.c_str());
	if (NULL == odhandle) {
		fprintf(stderr, "[%s:%u] - Error loading output driver: '%s'\n", __FILE__, __LINE__, GetLastError());
		return NULL;
	}

	typedef TestOutputDriver * (*odfactory_t)(void *);
//TODO ?	dlerror();
//	factory = (TestOutputDriver *(*)(void *)) dlsym(odhandle, "outputDriver_factory");
	odfactory_t factory = (odfactory_t) GetProcAddress(odhandle, "outputDriver_factory");
//	char *errmsg = dlerror();
	if (factory == NULL) {
		// TODO Handle error
		fprintf(stderr, "[%s:%u] - Error loading output driver: '%s'\n", __FILE__, __LINE__, GetLastError());
		return NULL;
	}

	TestOutputDriver *retval = factory(data);

	return retval;
}

int loadLibRunTest(test_data_t &testLib, ParameterDict &param)
{
   //printf("Loading test: %s\n", testLib.soname);
   char *dllname;
   HINSTANCE handle;

   // Build correct dll name
   dllname = (char *) malloc(sizeof(char)*(strlen(testLib.name)+strlen(ext)));
   strcpy(dllname,testLib.name);
   strcat(dllname,ext);

   handle = LoadLibrary(dllname);

   if (!handle)
   {
      printf("Error opening lib: %s\n", testLib.name);
      free(dllname);
      return -1;
   }

   char mutator_name[256];
   _snprintf(mutator_name, 256, "%s_mutatorMAIN", testLib.name);
   mutatorM_t mutTest = (mutatorM_t)GetProcAddress(handle, mutator_name);
   if ( !mutTest )
   {
      printf("Error finding function: %s, in %s\n", mutator_name, testLib.name);
      free(dllname);
      FreeLibrary(handle);
      return -1;
   }

   // Call function
   int result = mutTest(param);

   FreeLibrary(handle);
   free(dllname);

   //printf("Ran test: %s\n", testLib.soname);
   return result;
}

bool getMutatorsForRunGroup(RunGroup * group,
				std::vector<TestInfo *> &group_tests)
{
	for (int i = 0; i < group->tests.size(); i++) {
		if (false == group->tests[i]->enabled) {
			// Skip disabled tests
			continue;
		}

//		char *fullSoPath;

		TestInfo *test = group->tests[i];
		std::string dllname = std::string(test->name) + std::string(ext);
//		const char *soname = test->soname;
//		fullSoPath = searchPath(getenv("LD_LIBRARY_PATH"), soname);

//		if (!fullSoPath) {
//			fprintf(stderr, "Error finding lib %s in LD_LIBRARY_PATH/LIBPATH\n",
//			soname);
//			return true; // Error
//		}
//		void *handle = dlopen(fullSoPath, RTLD_NOW);
		HINSTANCE handle = LoadLibrary(dllname.c_str());
//		::free(fullSoPath);
		if (!handle) {
			fprintf(stderr, "Error opening lib: %s\n", dllname.c_str());
//			fprintf(stderr, "Error opening lib: %s\n", group->tests[i]->soname);
//			fprintf(stderr, "%s\n", dlerror());
			fprintf(stderr, "%s\n", GetLastError());
			return true; // Error
		}

		typedef TestMutator *(*mutator_factory_t)();
		char mutator_name[256];
		const char *testname = test->mutator_name;
		_snprintf(mutator_name, 256, "%s_factory", testname);
//		mutator_factory_t factory = (mutator_factory_t) dlsym(handle,
//		mutator_name);
		mutator_factory_t factory = (mutator_factory_t)GetProcAddress(handle, mutator_name);
		if (NULL == factory) {
			fprintf(stderr, "Error funding function: %s, in %s\n", mutator_name,
				dllname);
			//			soname);
//			fprintf(stderr, "%s\n", dlerror());
			fprintf(stderr, "%s\n", GetLastError());
//			dlclose(handle);
			FreeLibrary(handle);
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
}

