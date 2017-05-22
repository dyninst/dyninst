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

// $Id: test_lib_dllExecution.C,v 1.4 2008/10/30 19:16:57 legendre Exp $
#include "test_lib.h"
#include "ParameterDict.h"
#include "TestOutputDriver.h"
#include "comptester.h"
#include "module.h"
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef int (*mutatorM_t)(ParameterDict &);
const char* ext = ".dll";

void printErrorString()
{
	LPTSTR errMessage;
	DWORD lastError = GetLastError();
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		lastError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&errMessage,
		0, NULL );
	fprintf(stderr, "(%d): %s\n", lastError, errMessage);
	LocalFree(errMessage);
}

TESTLIB_DLL_EXPORT TestOutputDriver *loadOutputDriver(char *odname, void * data) {
	std::string fname (odname);
	fname += ext;

//	void *odhandle = dlopen(fname.str().c_str(), RTLD_NOW);
	HINSTANCE odhandle = LoadLibrary(fname.c_str());
	if (NULL == odhandle) {
		fprintf(stderr, "[%s:%u] - Error loading output driver: ", __FILE__, __LINE__);
		printErrorString();
		return NULL;
	}

	typedef TestOutputDriver * (*odfactory_t)(void *);
//TODO ?	dlerror();
//	factory = (TestOutputDriver *(*)(void *)) dlsym(odhandle, "outputDriver_factory");
	odfactory_t factory = (odfactory_t) GetProcAddress(odhandle, "outputDriver_factory");
//	char *errmsg = dlerror();
	if (factory == NULL) {
		// TODO Handle error
		fprintf(stderr, "[%s:%u] - Error loading output driver: ", __FILE__, __LINE__);
		printErrorString();
		return NULL;
	}

	TestOutputDriver *retval = factory(data);

	return retval;
}


int setupMutatorsForRunGroup(RunGroup * group)
{
   int tests_found = 0;
	for (unsigned i = 0; i < group->tests.size(); i++) {
		if (group->tests[i]->disabled)
			continue;

		TestInfo *test = group->tests[i];
		std::string dllname = std::string(test->name) + std::string(ext);
		HINSTANCE handle = LoadLibrary(dllname.c_str());
		if (!handle) {
			fprintf(stderr, "Error opening lib: %s\n", dllname.c_str());
			printErrorString();
			continue;
//			return -1;
		}

		typedef TestMutator *(*mutator_factory_t)();
		char mutator_name[256];
		const char *testname = test->mutator_name;
		snprintf(mutator_name, 256, "%s_factory", testname);
		mutator_factory_t factory = (mutator_factory_t)GetProcAddress(handle, mutator_name);
		if (NULL == factory) {
			fprintf(stderr, "Error funding function: %s, in %s\n", mutator_name,
				dllname.c_str());
			printErrorString();
//			FreeLibrary(handle);
//			return -1;
			continue;
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
	return tests_found;
}

typedef ComponentTester* (*comptester_factory_t)();
ComponentTester *Module::loadModuleLibrary()
{
   libhandle = NULL;
   std::string dllname = "test";
   dllname += name;
   dllname += ".dll";

   HINSTANCE handle = LoadLibrary(dllname.c_str());
   if (!handle) {
	   fprintf(stderr, "%s[%d]: Error opening library %s ", __FILE__, __LINE__, dllname.c_str());
		printErrorString();
	   return NULL;
   }
   libhandle = (void *) handle;
   comptester_factory_t factory;
   factory = (comptester_factory_t) GetProcAddress(handle, "componentTesterFactory");
   if (NULL == factory) {
	   fprintf(stderr, "Error finding componentTesterFactory ", dllname.c_str());
		printErrorString();
	   FreeLibrary(handle);
      return NULL;
   }
   
   return factory();
}
