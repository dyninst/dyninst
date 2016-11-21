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

//
// $Id: test_lib.C,v 1.6 2008/10/30 19:16:54 legendre Exp $
// Utility functions for use by the dyninst API test programs.
//

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <limits.h>

#include <string>
#include <vector>
using namespace std;

#if !defined(i386_unknown_nt4_0_test)
#include <fnmatch.h>
#endif

#if defined(i386_unknown_nt4_0_test) || defined(mips_unknown_ce2_11_test) //ccw 10 apr 2001 
#ifndef mips_unknown_ce2_11_test //ccw 10 apr 2001
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include <stdarg.h>

// Blind inclusion from test9.C
#if defined(i386_unknown_linux2_0_test) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */
#include <sys/types.h>
#include <sys/wait.h>
#endif

#if defined(i386_unknown_linux2_0_test) \
 || defined(x86_64_unknown_linux2_4)
#include <unistd.h>
#endif
// end inclusion from test9.C

#include "test_lib.h"
#include "ResumeLog.h"
#define BINEDIT_DIRNAME "" 


/* Control Debug printf statements */
static int debugPrint_ = 0;

// output logging
FILE *outlog = NULL;
FILE *errlog = NULL;
const char *outlogname = "-";
const char *errlogname = "-";

static const char *binedit_dir = BINEDIT_BASENAME;

const char *get_binedit_dir()
{
	return binedit_dir;
}

void set_binedit_dir(const char *d)
{
	binedit_dir = d;
}

LocErr::LocErr(const char *__file__, const int __line__, const std::string msg) :
	msg__(msg),
	file__(std::string(__file__)),
	line__(__line__)
{}

LocErr::~LocErr() THROW
{}

std::string LocErr::file() const
{
	return file__;
}

std::string LocErr::msg() const
{
	return msg__;
}
const char * LocErr::what() const
{
	return msg__.c_str();
}
int LocErr::line() const
{
	return line__;
}

void LocErr::print(FILE * /*stream*/) const
{
	logerror( "Error thrown from %s[%d]:\n\t\"%s\"\n",
			file__.c_str(), line__, what());
}

std::vector<std::string> Tempfile::all_open_files;

Tempfile::Tempfile()
{
#if defined (os_windows_test)
	fname = new char[1024];
	assert(fname);
	const char *dyninst_root = getenv("DYNINST_ROOT");
	char tmp_dir[1024];
	struct stat statbuf;

	if (!dyninst_root)
	{
      dyninst_root = "../..";
	}

	snprintf(tmp_dir, 1024, "%s\temp", dyninst_root);

	if (0 != stat(tmp_dir, &statbuf))
	{
		if (ENOENT == errno)
		{
			//  doesn't exist, make it
			if (0 != _mkdir(tmp_dir))
			{
				fprintf(stderr, "%s[%d]:  mkdir(%s): %s\n", __FILE__, __LINE__, tmp_dir, strerror(errno));
				abort();
			}
		}
		else
		{
			fprintf(stderr, "%s[%d]:  FIXME:  unexpected stat result: %s\n",
					__FILE__, __LINE__, strerror(errno));
			abort();
		}
	}

	if (0 != GetTempFileName(tmp_dir, "tempfile", 0, fname))
	{
		fprintf(stderr, "%s[%d]:  failed to create temp file name\n", __FILE__, __LINE__);
		assert(0);
	}

	fd = CreateFile(fname,
			GENERIC_READ | GENERIC_WRITE, // open r-w 
			0,                    // do not share 
			NULL,                 // default security 
			CREATE_ALWAYS,        // overwrite existing
			FILE_ATTRIBUTE_NORMAL,// normal file 
			NULL);                // no template 

	if (fd == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "%s[%d]:  failed to create temp file\n", __FILE__, __LINE__);
		assert(0);
	}
#else
	fname = strdup("/tmp/tmpfileXXXXXX");
	fd = mkstemp(fname);

	if (-1 == fd)
	{
		fprintf(stderr, "%s[%d]:  failed to make temp file\n", __FILE__, __LINE__);
		abort();
	}
#endif
	all_open_files.push_back(std::string(fname));
}

Tempfile::~Tempfile()
{
#if defined (os_windows_test)
	if (0 == DeleteFile(fname))
	{
		fprintf(stderr, "%s[%d]:  DeleteFile failed: %s\n",
				__FILE__, __LINE__, strerror(errno));
	}
	delete [] fname;
#else
	logerror( "%s[%d]:  unlinking %s\n", FILE__, __LINE__, fname);
	if (0 != unlink (fname))
	{
		fprintf(stderr, "%s[%d]:  unlink failed: %s\n",
				__FILE__, __LINE__, strerror(errno));
	}
	free (fname);
#endif
}

const char *Tempfile::getName()
{
	return fname;
}

void Tempfile::deleteAll()
{
	for (unsigned int i = (all_open_files.size() - 1); i > 0; --i)
	{
		const char *fn = all_open_files[i].c_str();
		assert(fn);
#if defined (os_windows_test)
		if (0 == DeleteFile(fn))
		{
			fprintf(stderr, "%s[%d]:  DeleteFile failed: %s\n",
					__FILE__, __LINE__, strerror(errno));
		}
#else
		fprintf(stderr, "%s[%d]:  unlinking %s\n", FILE__, __LINE__, fn);
		if (0 != unlink (fn))
		{
			fprintf(stderr, "%s[%d]:  unlink failed: %s\n",
					__FILE__, __LINE__, strerror(errno));
		}
#endif
	}
	all_open_files.clear();
}

TestOutputDriver * output = NULL;

// windows has strange support for sharing variables across
// a dll and a program, so here is a simple utility function to do that, since
// FUNCTION definitions are easily shared.
TestOutputDriver * getOutput() {
	return output;
}

	void setOutput(TestOutputDriver * new_output) {
		if (output != NULL)
			delete output;
		output = new_output;
	}

void setOutputLog(FILE *log_fp) {
	if (log_fp != NULL) {
		outlog = log_fp;
	} else {
		outlog = stdout;
	}
}

FILE *getOutputLog() {
	return outlog;
}

void setErrorLog(FILE *log_fp) {
	if (log_fp != NULL) {
		errlog = log_fp;
	} else {
		errlog = stderr;
	}
}

FILE *getErrorLog() {
	return errlog;
}

void setOutputLogFilename(char *log_fn) {
	if (log_fn != NULL) {
		outlogname = log_fn;
	}
}

void setErrorLogFilename(char *log_fn) {
	if (log_fn != NULL) {
		errlogname = log_fn;
	}
}

const char *getOutputLogFilename() {
	return outlogname;
}

const char *getErrorLogFilename() {
	return errlogname;
}

void logstatus(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	getOutput()->vlog(LOGINFO, fmt, args);
	va_end(args);
}

void logerror(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	getOutput()->vlog(LOGERR, fmt, args);
	va_end(args);
}

void flushOutputLog() {
	if (outlog != NULL) {
		fflush(outlog);
	}
}
void flushErrorLog() {
	if (errlog != NULL) {
		fflush(errlog);
	}
}

void setDebugPrint(int debug) {
   debugPrint_ = debug;
}
int debugPrint() 
{
  return debugPrint_;
}


bool inTestList(test_data_t &test, std::vector<char *> &test_list)
{
   for (unsigned int i = 0; i < test_list.size(); i++ )
   {
#if defined(i386_unknown_nt4_0_test)
      if ( strcmp(test_list[i], test.name) == 0 )
#else
      if ( fnmatch(test_list[i], test.name, 0) == 0 )
#endif
      {
         return true;
      }
   }

   return false;
}

// control debug printf statements
void dprintf(const char *fmt, ...) {
   va_list args;
   va_start(args, fmt);

   if(debugPrint())
      vfprintf(stderr, fmt, args);

   va_end(args);

   fflush(stderr);
}

// Build Architecture specific libname
// FIXME Is this used any more?  Is it necessary?
void addLibArchExt(char *dest, unsigned int dest_max_len, int psize, bool isStatic)
{
   int dest_len;

   dest_len = strlen(dest);

   // Patch up alternate ABI filenames
#if defined(rs6000_ibm_aix64_test)
   if(psize == 4) {
     strncat(dest, "_32", dest_max_len - dest_len);
     dest_len += 3;
   }
#endif

#if defined(arch_x86_64_test)
   if (psize == 4) {
      strncat(dest,"_m32", dest_max_len - dest_len);
      dest_len += 4;   
   }
#endif

#if defined(mips_sgi_irix6_4_test)
   strncat(dest,"_n32", dest_max_len - dest_len);
   dest_len += 4;
#endif

#if defined(os_windows_test)
   strncat(dest, ".dll", dest_max_len - dest_len);
   dest_len += 4;
#else
   if( isStatic ) {
       strncat(dest, ".a", dest_max_len - dest_len);
       dest_len += 2;
   }else{
       strncat(dest, ".so", dest_max_len - dest_len);
       dest_len += 3;
   }
#endif
}

#define TOLOWER(c) ((c >= 'A' && c <= 'Z') ? c - 'A' + 'a' : c)
int strcmpcase(char *s1, char *s2) {
    unsigned i;
    unsigned char s1_c, s2_c;
    for (i=0; s1[i] || s2[i]; i++) {
        s1_c = TOLOWER(s1[i]);
        s2_c = TOLOWER(s2[i]);
        if (s1_c < s2_c)
            return -1;
        if (s1_c > s2_c)
            return 1;
    }
    return 0;
}


#if !defined(os_windows_test)
char *searchPath(const char *path, const char *file) {
   assert(path);
   assert(file);

   char *pathcopy = strdup(path);
   char *fullpath;
   char *ptr = NULL; // Purify complained that this was read uninitialized
   char *token = strtok_r(pathcopy, ":", &ptr);

   while (token) {
      fullpath = (char *) ::malloc(strlen(token) + strlen(file) + 2);
      strcpy(fullpath, token);
      strcat(fullpath, "/");
      strcat(fullpath, file);

      struct stat statbuf;
      if (!stat(fullpath, &statbuf))
         break;

      ::free(fullpath);
      token = strtok_r(NULL, ":", &ptr);
   }
   ::free(pathcopy);
   if (token)
      return fullpath;
   return NULL;
}
#else
char *searchPath(const char *path, const char *file) {
  char *fullpath = (char *) ::malloc(strlen(path) + strlen(file) + 2);
  strcpy(fullpath, path);
  strcat(fullpath, "\\");
  strcat(fullpath, file);
  return fullpath;
}
#endif

/**
 * A test should be run if:
 *   1) It isn't disabled
 *   2) It hasn't reported a failure/crash/skip
 *   3) It hasn't reported final results already
 **/
bool shouldRunTest(RunGroup *group, TestInfo *test)
{
   if (group->disabled || test->disabled)
      return false;
   
   if (test->result_reported)
      return false;

   for (unsigned i=0; i<NUM_RUNSTATES; i++)
   {
      if (i == program_teardown_rs)
         continue;
      if (test->results[i] == FAILED ||
          test->results[i] == SKIPPED ||
          test->results[i] == CRASHED)
      {
         reportTestResult(group, test);
         return false;
      }
      assert(test->results[i] == UNKNOWN ||
             test->results[i] == PASSED);
   }
   return true;
}

void reportTestResult(RunGroup *group, TestInfo *test)
{
   if (test->result_reported || test->disabled)
      return;

   test_results_t result = UNKNOWN;
   bool has_unknown = false;
   int failed_state = -1;

   for (unsigned i=0; i<NUM_RUNSTATES; i++)
   {
      if (i == program_teardown_rs)
         continue;
      if (test->results[i] == FAILED ||
          test->results[i] == CRASHED || 
          test->results[i] == SKIPPED) {
         result = test->results[i];
         failed_state = i;
         break;
      }
      else if (test->results[i] == PASSED) {
         result = test->results[i];
      }
      else if (test->results[i] == UNKNOWN) {
         has_unknown = true;
      }
      else {
         assert(0 && "Unknown run state");
      }
   }

   if (result == PASSED && has_unknown)
      return;

   std::map<std::string, std::string> attrs;
   TestOutputDriver::getAttributesMap(test, group, attrs);
   getOutput()->startNewTest(attrs, test, group);
   getOutput()->logResult(result, failed_state);
   getOutput()->finalizeOutput();

   log_testreported(group->index, test->index);
   test->result_reported = true;
}

#if defined(cap_liberty_exec_test)
#if !defined(cap_gnu_demangler_test)
/**
 * Many linkers don't want to link the static libiberty.a unless
 * we have a reference to it.  It's really needed by libtestdyninst.so
 * and libtestsymtab.so, but we can't link into them because 
 * most systems only provide a static version of this library.
 * 
 * Thus we need libiberty.a linked in with test_driver.  We put a reference
 * to libiberty in libtestSuite.so here, which causes cplus_demangle to 
 * be exported for use by libraries in in test_driver.
 *
 * This is intentionally unreachable code
 **/
extern "C" char *cplus_demangle(char *, int);

void use_liberty()
{
   cplus_demangle("a", 0);
}
#endif
#endif

#if defined (os_windows_test)
//  solaris does not provide setenv, so we provide an ersatz replacement.
// yes it's leaky, but we don't plan on using it too much, so who cares?
int setenv(const char *envname, const char *envval, int)
{
	std::string *alloc_env = new std::string(std::string(envname) 
			+ std::string("=") + std::string(envval));
	return _putenv((char *)alloc_env->c_str());

}
#endif

int bg_maxThreadsPerProcess(const char *runmode) {
   if (strcmp(runmode, "SMP") == 0) {
      return 4;
   }
   else if (strcmp(runmode, "DUAL") == 0) {
      return 2;
   }
   else if (strcmp(runmode, "VN") == 0) {
      return 1;
   }
   assert(0);
   return -1;
}

int getNumProcs(const ParameterDict &dict)
{
   ParameterDict::const_iterator i = dict.find("mp");
   assert(i != dict.end());
   if (i->second->getInt() <= 1) {
      return 1;
   }
#if defined(os_bg_test)
   int base = 16;
#else
   int base = 8;
#endif
   char *e = getenv("DYNINST_MPTEST_WIDTH");
   if (e) {
      int result = atoi(e);
      if (result)
         base = result;
   }
   int mult = 1;
#if defined(os_bgp_test)
   i = dict.find("platmode");
   int max_threads = bg_maxThreadsPerProcess(i->second->getString());
   mult = 4 / max_threads;
#endif
   return base * mult;
}

int getNumThreads(const ParameterDict &dict)
{
   ParameterDict::const_iterator i = dict.find("mt");
   assert(i != dict.end());
   if (i->second->getInt() <= 1) {
      return 0;
   }
   char *e = getenv("DYNINST_MTTEST_WIDTH");
   if (e) {
      int result = atoi(e);
      if (result)
         return result;
   }
#if defined(os_bg_test)
   i = dict.find("platmode");
   return bg_maxThreadsPerProcess(i->second->getString()) - 1;
#else
   return 8;
#endif
}

static FILE *debug_log = NULL;
FILE *getDebugLog()
{
	return debug_log;
}

void setDebugLog(FILE *f)
{
	debug_log = f;
}
