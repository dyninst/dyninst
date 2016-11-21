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
#include <errno.h>
#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sstream>
#include <string>
#include <cstring>
#include <cassert>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "runTests-utils.h"

extern string pdscrdir;

int timeout = 1200; /* seconds */

void initPIDFilename(char *buffer, size_t len) {
  snprintf(buffer, len, "pids.%d", getpid());
}

void cleanupMutatees(char *pidFilename) {
   if (!pidFilename)
      return;
   FILE *pidFile = fopen(pidFilename, "r");
   if (NULL == pidFile) 
      return;
   for (;;) {
      int pid, count;
      count = fscanf(pidFile, "%d\n", &pid);
      if (count != 1) 
         break;
      
      if (pid < 1) {
         fprintf(stderr, "[%s:%u] - Read a negative PID (%d).  Something's weird.\n", __FILE__, __LINE__, pid);
         continue;
      } 
      
      int status = kill(pid, SIGKILL);
      if ((status != 0) && (errno != ESRCH)) {
         // FIXME the call to strerror() is not thread-safe
         // We don't care if the process is already dead, so we're ignoring
         // ESRCH
         fprintf(stderr, "[%s:%u] - INFO: %s\n", __FILE__, __LINE__, strerror(errno));
      }
   }
   fclose(pidFile);
   unlink(pidFilename);
}

static bool timed_out;
static void sigalrm_action(int sig, siginfo_t *siginfo, void *context) {
  // Note that the child has timed out and return
  timed_out = true;
}

static bool interrupted;
static void sigint_action(int sig, siginfo_t *siginfo, void *context) {
  interrupted = true;
}

void generateTestArgs(char **exec_args[], bool resume, bool useLog,
                      bool staticTests, string &logfile, int testLimit,
                      vector<char *> &child_argv, const char *pidFilename,
                      std::string hostname);

int CollectTestResults(vector<test_driver_t> &test_drivers, int parallel_copies)
{
   // Install signal handler for a timer
   struct sigaction sigalrm_a;
   struct sigaction old_sigalrm_a;
   struct sigaction sigint_a;
   struct sigaction old_sigint_a;
   int retval;

   timed_out = false;
   sigalrm_a.sa_sigaction = sigalrm_action;
   sigemptyset(&(sigalrm_a.sa_mask));
   sigalrm_a.sa_flags = SA_SIGINFO;
   sigaction(SIGALRM, &sigalrm_a, &old_sigalrm_a);
   alarm(timeout);

   interrupted = false;
   sigint_a.sa_sigaction = sigint_action;
   sigemptyset(&(sigint_a.sa_mask));
   sigint_a.sa_flags = SA_SIGINFO;
   sigaction(SIGINT, &sigint_a, &old_sigint_a);

   // I think I want to use wait() here instead of using a sleep loop
   // - There are issues with using sleep() and alarm()..
   // - I'll wait() and if the alarm goes off it will interrupt the waiting,
   //   so the end result should be the same

   // Wait for one of the signals to fire
   for (;;) 
   {
      // Wait for child to exit
      pid_t waiting_pid;
      int child_status;
      //fprintf(stderr, "%s[%d]:  before waitpid(%d)\n", __FILE__, __LINE__, child_pid);
      if (!timed_out && !interrupted) {
         // BUG I fear there's a race condition here, where I may not catch a
         // timeout if it occurs between the timed_out check and the call to
         // waitpid().  I'm not sure what to do about that.
         waiting_pid = wait(&child_status);
      }

      if (timed_out || interrupted) {
         // Timed out..  timer.pl sets the return value to -1 for this case
         if (timed_out)
            fprintf(stderr, "*** Process exceeded time limit.  Reaping children.\n");
         else
            fprintf(stderr, "*** SIGINT received.  Reaping children.\n");

         for (unsigned i = 0; i < parallel_copies; i++) {
            if (!test_drivers[i].pid)
               continue;
            kill(test_drivers[i].pid, SIGKILL);
            if (waitpid(test_drivers[i].pid, NULL, 0) > 0) {
                // Enable the possibility of restarting this test driver
                test_drivers[i].pid = 0;
            }
         }
         retval = interrupted ? -3 : -1;
         break;

      } else if (waiting_pid < 0) {
          // We have no children.  Probably because we reaped them.
          // Let the outer calling function decide if we should exit
          // or restart test_drivers.
          //
          // This behavior has been seen before, but may not be a
          // possibility after changes in runTests.C.
          retval = 0;
          break;
      }

      if (WIFSIGNALED(child_status) || WIFEXITED(child_status))
      {
         int child_ret = 0;
         if (WIFSIGNALED(child_status)) {
            fprintf(stderr, "*** Child terminated abnormally via signal %d.\n", WTERMSIG(child_status));
            child_ret = -2;
         } else {
            child_ret = (signed char) WEXITSTATUS(child_status);
         }
         for (unsigned i=0; i<parallel_copies; i++)
         {
            if (test_drivers[i].pid == waiting_pid) {
               test_drivers[i].pid = 0;
               test_drivers[i].last_result = child_ret;
               retval = i;
               break;
            }
         }
         break;
      }
   }
   alarm(0); // Cancel alarm
   
   // Uninstall handlers
   sigaction(SIGALRM, &old_sigalrm_a, NULL);
   sigaction(SIGINT, &old_sigint_a, NULL);

   return retval;
}

test_pid_t RunTest(unsigned int iteration, bool useLog, bool staticTests,
                   string logfile, int testLimit, vector<char *> child_argv,
                   const char *pidFilename, std::string hostname)
{
   int retval = -1;

   char **exec_args = NULL;

   generateTestArgs(&exec_args, iteration > 0, useLog, staticTests, logfile,
                    testLimit, child_argv, pidFilename, hostname);

   if (hostname.length())
      sleep(1);

   // Fork and execute test_driver in the child process
   int child_pid = fork();
   if (-1 == child_pid) {
      return -4;
   } else if (0 == child_pid) {
      // Child
      execvp(exec_args[0], exec_args);
      std::string newexec = std::string("./") + exec_args[0];
      execvp(newexec.c_str(), exec_args);
      exit(-4);
   }

   return child_pid;
}

string ReplaceAllWith(const string &in, const string &replace, const string &with);

void generateTestArgs(char **exec_args[], bool resume, bool useLog,
                      bool staticTests, string &logfile, int testLimit,
                      vector<char *> &child_argv, const char *pidFilename,
                      std::string hostname)
{
  vector<const char *> args;

  if (hostname.size())
  {
     args.push_back("ssh");
     args.push_back(hostname.c_str());
     assert(scriptname);
     args.push_back(scriptname);
  }
  else
  {
     args.push_back("test_driver");
  }

  args.push_back("-under-runtests");
  args.push_back("-enable-resume");
  args.push_back("-group-limit");
  char *limit_str = new char[12];
  snprintf(limit_str, 12, "%d", testLimit);
  args.push_back(limit_str);
  if (pidFilename != NULL && strlen(pidFilename)) {
    args.push_back("-pidfile");
    args.push_back(pidFilename);
  }
  if (resume) {
    args.push_back("-use-resume");
  }
  if (useLog) {
    args.push_back("-log");
    args.push_back("-logfile");
    args.push_back(const_cast<char *>(logfile.c_str()));
  }
  static bool first_run = true;
  if (!first_run) {
     args.push_back("-no-header");
  }
  first_run = false;

  for (unsigned int i = 0; i < child_argv.size(); i++) {
      args.push_back(child_argv[i]);
  }

  // Copy the arguments from the vector to a new array
  // BUG limit_str leaks here.  I'm not sure how to clean up this leak
  int exec_argc = args.size();
  *exec_args = new char *[exec_argc + 1];
  for (unsigned int i = 0; i < args.size(); i++) {
     (*exec_args)[i] = const_cast<char *>(args[i]);
  }
  (*exec_args)[exec_argc] = NULL;
}

void generateTestString(bool resume, bool useLog, bool staticTests,
			string &logfile, int testLimit,
			vector<char *>& child_argv, string& shellString,
			char *pidFilename)
{
   stringstream testString;
   if (staticTests) {
     testString << "test_driver_static";
   } else {
     testString << "test_driver";
   }
   testString << " -under-runtests -enable-resume -limit " << testLimit;

   if ( resume )
   {
      testString << " -use-resume";
   }
   if ( useLog )
   {
      testString << " -log";
   }
   if (useLog) {
     testString << " -logfile " << logfile;
   }

   // Add child's arguments
   for ( unsigned int i = 0; i < child_argv.size(); i++ )
   {
      testString << " " << child_argv[i];
   }
   
   stringstream timerString;
   timerString << pdscrdir << "/timer.pl -t " << timeout;
   shellString = timerString.str() + " " + testString.str();
   
}

char *setResumeEnv()
{
   if ( getenv("RESUMELOG") != NULL )
      return NULL;

   stringstream tmp;
   tmp << "RESUMELOG=";
   if ( getenv("TMP") )
   {
      tmp << getenv("TMP") << "/";
   }
   else
   {
      tmp << "/tmp/";
   }

   tmp << "test_driver.resumelog." << getpid();

   char *r_tmp = strdup(tmp.str().c_str());
   putenv(r_tmp);

   return r_tmp;
}

char *setLibPath()
{
   stringstream tmp;
   tmp << "LD_LIBRARY_PATH=.:";

   if ( getenv("LD_LIBRARY_PATH") )
   {
      tmp << getenv("LD_LIBRARY_PATH");
   }

   char *l_tmp = strdup(tmp.str().c_str());
   putenv(l_tmp);

   // And add LIBPATH for AIX if necessary
   stringstream AIXtmp;
   AIXtmp << "LIBPATH=.:";
   if ( getenv("LIBPATH") ) {
       AIXtmp << getenv("LIBPATH");
   }
   char *a_tmp = strdup(AIXtmp.str().c_str());
   putenv(a_tmp);

   return l_tmp;
}

void setupVars(bool useLog, string &logfile)
{
   string base_dir, tlog_dir;

#if defined(m_abi)
   if ( getenv("DYNINSTAPI_RT_LIB") )
   {
      char *rtlib = getenv("DYNINSTAPI_RT_LIB");

      string temp = ReplaceAllWith(rtlib, "libdyninstAPI_RT.so", "libdyninstAPI_RT_m32.so");
      char *rtlib_mabi = strdup(temp.c_str());
      setenv("DYNINSTAPI_RT_LIB_MABI", rtlib_mabi, 0);
   }
#endif
  
   // Determine Base Dir
   if ( getenv("PARADYN_BASE") == NULL )
   {
      if ( getenv("DYNINST_ROOT") == NULL )
      {
	base_dir = string("../../..");
      }
      else
      {
         base_dir = getenv("DYNINST_ROOT");
      }
   } else
   {
      base_dir = getenv("PARADYN_BASE");
   }

   pdscrdir = base_dir + "/dyninst/scripts";
   bool have_scripts = true;
   if ( ! isDir(pdscrdir) )
   {
      have_scripts = false;
   }

   // Determine Test log dir
   char *pdtst = getenv("PDTST");
   if ( pdtst == NULL )
   {
      tlog_dir = base_dir + "/log/tests";
   } else
   {
      tlog_dir = pdtst;
   }
   tlog_dir += "2";

   // Determin Test log file
   string buildnum, build_id;
   if (have_scripts) {
      buildnum = pdscrdir + "/buildnum";
      if ( getenv("PARADYN_BASE") != NULL && isRegFile(buildnum) )
      {
         getInput(buildnum.c_str(), build_id); 
      } else
      {
         getInput("date '+%Y-%m-%d'", build_id);
      }
   }
   else {
      build_id = string("0");
   }
      

   if ( useLog ) 
   {

      if ( logfile == "" )
      {
         logfile = tlog_dir + "/" + getenv("PLATFORM") + "/" + build_id;
      }

      cout << "   ... output to " << logfile << endl;

      string testslogdir, cmd;
      cmd = "dirname " + logfile;
      getInput(cmd.c_str(), testslogdir);
   
      if ( ! isDir(testslogdir) )
      {
         cout << testslogdir << "does not exist (yet)!" << endl;
         cmd = "mkdir -p " + testslogdir;
         system(cmd.c_str());
         if ( ! isDir(testslogdir) )
         {
            cout << testslogdir << " creation failed - aborting!" << endl;
            exit(1);
         } else {
            cout << testslogdir << " create for test logs!" << endl;
         }
      }

      if ( isRegFile(logfile) )
      {
         cout << "File exists" << endl;
      }
      else
      {
         cmd = "touch " + logfile;
         system(cmd.c_str());
      }
   
      cmd = logfile + ".gz";
      if ( isRegFile(cmd) )
      {
         cout << "File.gz exists" << endl;
      }
   }
}

#define SCRIPT_NAME "parallel_testdriver"

char *createParallelScript()
{
   char cwd[4096];
   memset(cwd, 0, 4096);
   char *result = getcwd(cwd, 4096);
   if (!result) {
      perror("Could not getcwd for parallel script");
      exit(-1);
   }

   FILE *f = fopen(SCRIPT_NAME, "w");
   fprintf(f, "#!/bin/sh\n");
   fprintf(f, "cd %s\n", cwd);
   fprintf(f, "./test_driver $*\n");
   fclose(f);
   
   chmod(SCRIPT_NAME, 0750);
   
   char *fullname = (char *) malloc(strlen(cwd) + strlen(SCRIPT_NAME) + 3);
   strcpy(fullname, cwd);
   strcat(fullname, "/");
   strcat(fullname, SCRIPT_NAME);

   return fullname;
}
