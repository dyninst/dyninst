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

#ifndef _test_lib_h_
#define _test_lib_h_

#include <iostream>
#include <typeinfo>
#include <stdexcept>
#include "ParameterDict.h"
#include "TestData.h"
#include "test_info_new.h"
#include "test_lib_dll.h"
#include "util.h"
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#if !defined(P_sleep)
#if defined(os_windows_test)
#define P_sleep(sec) Sleep(1000*(sec))
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#define P_sleep(sec) sleep(sec)
#include <unistd.h>
#endif
#endif

#if defined(os_windows_test)
#include <direct.h>
#endif
#define DYNINST_NO_ERROR -1

#include "test_results.h"
#include "TestMutator.h"
#include "TestOutputDriver.h"
#include "comptester.h"

#define RETURNONFAIL(x) if ( x < 0 ) return FAILED;
#define RETURNONNULL(x) if ( x == NULL ) return FAILED;
#define PASS 0
#define FAIL -1

#define BINEDIT_BASENAME "./binaries"
TESTLIB_DLL_EXPORT const char *get_binedit_dir();
TESTLIB_DLL_EXPORT void set_binedit_dir(const char *s);

// New logging system
TESTLIB_DLL_EXPORT TestOutputDriver * getOutput();
TESTLIB_DLL_EXPORT void setOutput(TestOutputDriver * new_output);
// Set up the log files for test library output
TESTLIB_DLL_EXPORT void setOutputLog(FILE *log_fp);
TESTLIB_DLL_EXPORT void setErrorLog(FILE *log_fp);
TESTLIB_DLL_EXPORT FILE *getOutputLog();
TESTLIB_DLL_EXPORT FILE *getErrorLog();
TESTLIB_DLL_EXPORT void setOutputLogFilename(char *log_fn);
TESTLIB_DLL_EXPORT void setErrorLogFilename(char *log_fn);
TESTLIB_DLL_EXPORT const char *getOutputLogFilename();
TESTLIB_DLL_EXPORT const char *getErrorLogFilename();

// Functions to print messages to the log files
TESTLIB_DLL_EXPORT void logstatus(const char *fmt, ...);
TESTLIB_DLL_EXPORT void logerror(const char *fmt, ...);
TESTLIB_DLL_EXPORT void flushOutputLog();
TESTLIB_DLL_EXPORT void flushErrorLog();

// TODO Implement this function for Windows   
TESTLIB_DLL_EXPORT int setupMutatorsForRunGroup (RunGroup *group);

TESTLIB_DLL_EXPORT int getNumProcs(const ParameterDict &dict);
TESTLIB_DLL_EXPORT int getNumThreads(const ParameterDict &dict);

TESTLIB_DLL_EXPORT FILE *getDebugLog();
TESTLIB_DLL_EXPORT void setDebugLog(FILE *f);

// Mutatee PID registration, for cleaning up hung mutatees
// TODO Check if these make any sense on Windows.  I suspect I'll need to
// change them.
TESTLIB_DLL_EXPORT void setPIDFilename(char *pfn);
TESTLIB_DLL_EXPORT void registerPID(int pid);

TESTLIB_DLL_EXPORT void setDebugPrint(int debug);
TESTLIB_DLL_EXPORT int debugPrint();

TESTLIB_DLL_EXPORT bool inTestList(test_data_t &test, std::vector<char *> &test_list);

TESTLIB_DLL_EXPORT void dprintf(const char *fmt, ...);
TESTLIB_DLL_EXPORT void addLibArchExt(char *dest, unsigned int dest_max_len, int psize, bool isStatic = false);
TESTLIB_DLL_EXPORT int strcmpcase(char *s1, char *s2);
char *searchPath(const char *path, const char *file);

TESTLIB_DLL_EXPORT bool shouldRunTest(RunGroup *group, TestInfo *test);
TESTLIB_DLL_EXPORT void reportTestResult(RunGroup *group, TestInfo *test);

// loadOutputDriver loads an output driver plug-in and returns a pointer to
// the output driver implemented by it.
TESTLIB_DLL_EXPORT TestOutputDriver *loadOutputDriver(char *odname, void * data);

// Functions used for redirecting output e.g. to a temp file for entering into
// the database after running a test
TESTLIB_DLL_EXPORT int printout(char *fmt, ...);
TESTLIB_DLL_EXPORT int printerr(char *fmt, ...);
TESTLIB_DLL_EXPORT int printhuman(char *fmt, ...);

// Functions related to database output
TESTLIB_DLL_EXPORT void enableDBLog(TestInfo *test, RunGroup *runGroup);
TESTLIB_DLL_EXPORT void clearDBLog();

TESTLIB_DLL_EXPORT ComponentTester *getComponentTester();

#define EFAIL(cmsg) throw LocErr(__FILE__, __LINE__, std::string(cmsg))
#define REPORT_EFAIL catch(const LocErr &err) { \
	   err.print(stderr); \
	   return FAILED; }

class LocErr  
{
	std::string msg__;
	std::string file__;
	int line__;

	public:

	TESTLIB_DLL_EXPORT LocErr(const char * __file__,
			const int __line__,
			const std::string msg); 

	TESTLIB_DLL_EXPORT virtual ~LocErr() THROW; 

	TESTLIB_DLL_EXPORT std::string file() const;

	TESTLIB_DLL_EXPORT int line() const;
	TESTLIB_DLL_EXPORT std::string msg() const;
	TESTLIB_DLL_EXPORT const char * what() const;

	TESTLIB_DLL_EXPORT void print(FILE * stream)  const;
};

class Tempfile {

	//  file paths should be generalized to work on windows
	char *fname;
#if !defined (os_windows_test)
	typedef int fd_type;
#else
	typedef HANDLE fd_type;
#endif
	fd_type fd;
	static std::vector<std::string> all_open_files;

	public:

	TESTLIB_DLL_EXPORT Tempfile();
	TESTLIB_DLL_EXPORT ~Tempfile();
	TESTLIB_DLL_EXPORT const char *getName();
	TESTLIB_DLL_EXPORT static void deleteAll();
};

#if defined (os_windows_test)
TESTLIB_DLL_EXPORT int setenv(const char *envname, const char *envval, int);
#endif

#endif
