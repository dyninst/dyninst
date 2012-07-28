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
#ifndef RUNTESTS_UTILS_H
#define RUNTESTS_UTILS_H

/* Windows Specific Includes and Macros */
#if defined(i386_unknown_nt4_0_test)
// For getpid
#include <process.h>
#define popen _popen
#define pclose _pclose

#define S_ISREG(x) ((x & S_IFMT) == S_IFREG)
#define S_ISDIR(x) ((x & S_IFMT) == S_IFDIR)
#else
/* Unix Specific includes */
#include <unistd.h>

#endif

#if !defined(os_aix_test) && !defined(os_windows_test) && !defined(os_freebsd_test)
#include <wait.h>
#elif defined(os_freebsd_test)
#include <sys/wait.h>
#endif

#if defined(i386_unknown_nt4_0_test)
#define WEXITSTATUS(x) x
#endif

#include <string>
#include <vector>

using namespace std;

extern char* scriptname;

#if defined(os_windows_test)
typedef HANDLE test_pid_t;
#else
typedef int test_pid_t;
#endif

class test_driver_t {
  public:

  test_driver_t() :
         pid(0),
         last_result(0),
         unique(0),
         useLog(false),
         staticTests(false),
         testLimit(0)
         {
         }

   test_pid_t pid;
   int last_result;
   int unique;
   bool useLog;
   bool staticTests;
   string logfile;
   int testLimit;
   vector<char *> child_argv;
   std::string pidFilename;
   std::string hostname;
   std::string outputlog;
};

// fills the buffer with the name of a file to use for PID
// registration for mutatee cleanup
void initPIDFilename(char *buffer, size_t len);

// Kills any remaining mutatee processes that are listed in the PID file
void cleanupMutatees(char *pidFilename);

test_pid_t RunTest(unsigned int iteration, bool useLog, bool staticTests,
                   std::string logfile, int testLimit, vector<char *> child_argv,
                   const char *pidFilename, std::string hostname);

int CollectTestResults(vector<test_driver_t> &test_drivers, int parallel_copies);

bool isRegFile(const string& filename);

bool isDir(const string& filename); 

void getInput(const char *filename, string& output);

void generateTestString(bool resume, bool useLog, bool staticTests,
			string &logfile, int testLimit,
			vector<char *>& child_argv, string& shellString,
			char *pidFilename);

char *setResumeEnv();

char *setLibPath();

void setupVars(bool useLog, string &logfile);

char *createParallelScript();

#endif /* RUNTESTS_UTILS_H */
