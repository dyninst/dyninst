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

// $Id: test_lib_mutateeStart.C,v 1.1 2008/10/30 19:21:46 legendre Exp $
// Functions Dealing with mutatee Startup

#define COMPLIB_DLL_BUILD

#include "dyninst_comp.h"
#include "test_lib.h"
#include "util.h"
#include "ResumeLog.h"
#include <stdlib.h>


BPatch_thread *startMutateeTestGeneric(BPatch *bpatch, const char *pathname, const char **child_argv, bool useAttach)
{
   BPatch_thread *appThread;
   if (useAttach) {
      // I should be able to remove the outlog and errlog parameters from
      // startNewProcessForAttach without harming anything.
      int pid = startNewProcessForAttach(pathname, child_argv,
                                         NULL, NULL, true);
      if (pid < 0) {
         fprintf(stderr, "*ERROR*: unable to start tests due to error creating mutatee process\n");
         return NULL;
      } else {
         dprintf("New mutatee process pid %d started; attaching...\n", pid);
         registerPID(pid); // Register PID for cleanup
      }
      dprintf("Attaching to process: %s, %d\n", pathname, pid);
      appThread = bpatch->attachProcess(pathname, pid);
      dprintf("Attached to process\n");
      dprintf("appThread == %lu\n", (unsigned long) appThread);
   } else {
      appThread = bpatch->createProcess(pathname, child_argv, NULL);
      if (appThread != NULL) {
         int pid = appThread->getProcess()->getPid();
         registerPID(pid); // Register PID for cleanup
      }
   }

   return appThread;
}

BPatch_thread *startMutateeTest(BPatch *bpatch, const char *mutatee, const char *testname,
                                bool useAttach, char *logfilename,
                                char *humanlogname)
{
   std::vector<std::string> mutateeArgs;
   getOutput()->getMutateeArgs(*&mutateeArgs); // mutateeArgs is an output parameter
   const char **child_argv = new const char *[8 + mutateeArgs.size()];
   if (NULL == child_argv) {
      return NULL;
   }

   // Start the mutatee
   dprintf("Starting \"%s\"\n", mutatee);

   int n = 0;
   child_argv[n++] = mutatee;
   if (logfilename != NULL) {
      child_argv[n++] = const_cast<char *>("-log");
      child_argv[n++] = logfilename;
   }
   if (humanlogname != NULL) {
      child_argv[n++] = const_cast<char *>("-humanlog");
      child_argv[n++] = humanlogname;
   }
   child_argv[n++] = const_cast<char *>("-run");
   child_argv[n++] = testname;
   for (int i = 0; i < mutateeArgs.size(); i++) {
      child_argv[n++] = mutateeArgs[i].c_str();
   }
   child_argv[n] = NULL;

   BPatch_thread *retval = startMutateeTestGeneric(bpatch, mutatee, child_argv,
                                                   useAttach);
   delete [] child_argv;
   return retval;
}

static const char** parseArgsIndividual(RunGroup *group, TestInfo *test,
                              char *logfilename, char *humanlogname,
                              bool verboseFormat, bool printLabels,
                              int debugPrint, const char *pidfilename)
{
   std::vector<std::string> mutateeArgs;
   getOutput()->getMutateeArgs(mutateeArgs); // mutateeArgs is an output parameter
   const char **child_argv = new const char *[12 + (4 * group->tests.size()) +
                                              mutateeArgs.size()];
   assert(child_argv);
   int n = 0;
   child_argv[n++] = group->mutatee;
   if (logfilename != NULL) {
      child_argv[n++] = const_cast<char *>("-log");
      child_argv[n++] = logfilename;
   }
   if (humanlogname != NULL) {
      child_argv[n++] = const_cast<char *>("-humanlog");
      child_argv[n++] = humanlogname;
   }
   if (false == verboseFormat) {
      child_argv[n++] = const_cast<char *>("-q");
      // TODO I'll also want to pass a parameter specifying a file to write
      // postponed messages to
   }
   if (debugPrint != 0) {
      child_argv[n++] = const_cast<char *>("-verbose");
   }
   if (pidfilename != NULL) {
      child_argv[n++] = const_cast<char *>("-pidfile");
      child_argv[n++] = pidfilename;
   }
   if (shouldRunTest(group, test)) {
         child_argv[n++] = const_cast<char*>("-run");
         child_argv[n++] = test->name;
      }
   if (printLabels) {
         child_argv[n++] = const_cast<char *>("-label");
         child_argv[n++] = test->label;
      child_argv[n++] = const_cast<char *>("-print-labels");
   }
   for (int i = 0; i < mutateeArgs.size(); i++) {
      child_argv[n++] = strdup(mutateeArgs[i].c_str());
   }
   child_argv[n] = NULL;   

   return child_argv;
}

static const char** parseArgs(RunGroup *group,
                              char *logfilename, char *humanlogname,
                              bool verboseFormat, bool printLabels,
                              int debugPrint, char *pidfilename)
{
   std::vector<std::string> mutateeArgs;
   getOutput()->getMutateeArgs(mutateeArgs); // mutateeArgs is an output parameter
   const char **child_argv = new const char *[12 + (4 * group->tests.size()) +
                                              mutateeArgs.size()];
   assert(child_argv);
   int n = 0;
   child_argv[n++] = group->mutatee;
   if (logfilename != NULL) {
      child_argv[n++] = const_cast<char *>("-log");
      child_argv[n++] = logfilename;
   }
   if (humanlogname != NULL) {
      child_argv[n++] = const_cast<char *>("-humanlog");
      child_argv[n++] = humanlogname;
 }
   if (false == verboseFormat) {
      child_argv[n++] = const_cast<char *>("-q");
      // TODO I'll also want to pass a parameter specifying a file to write
      // postponed messages to
   }
   if (debugPrint != 0) {
      child_argv[n++] = const_cast<char *>("-verbose");
   }
   if (pidfilename != NULL) {
      child_argv[n++] = const_cast<char *>("-pidfile");
      child_argv[n++] = pidfilename;
   }
   for (int i = 0; i < group->tests.size(); i++) {
      if (shouldRunTest(group, group->tests[i])) {
         child_argv[n++] = const_cast<char*>("-run");
         child_argv[n++] = group->tests[i]->name;
      }
   }
   if (printLabels) {
      for (int i = 0; i < group->tests.size(); i++) {
         child_argv[n++] = const_cast<char *>("-label");
         child_argv[n++] = group->tests[i]->label;
      }
      child_argv[n++] = const_cast<char *>("-print-labels");
   }
   for (int i = 0; i < mutateeArgs.size(); i++) {
      child_argv[n++] = strdup(mutateeArgs[i].c_str());
   }
   child_argv[n] = NULL;   

   return child_argv;
}

BPatch_thread *startMutateeTest(BPatch *bpatch, RunGroup *group,
                                char *logfilename, char *humanlogname,
                                bool verboseFormat, bool printLabels,
                                int debugPrint, char *pidfilename)
{
   const char **child_argv = parseArgs(group, logfilename, humanlogname,
                                       verboseFormat, printLabels, 
                                       debugPrint, pidfilename);
   
   // Start the mutatee
   dprintf("Starting \"%s\"\n", group->mutatee);
   BPatch_thread *retval = startMutateeTestGeneric(bpatch, group->mutatee,
                                                   child_argv,
                                                   group->useAttach);
   delete [] child_argv;
   return retval;
}

BPatch_binaryEdit *startBinaryTest(BPatch *bpatch, RunGroup *group)
{
   BPatch_binaryEdit *binEdit = bpatch->openBinary(group->mutatee, true);
   return binEdit;
}


#if defined(os_linux_test)
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

static void clearBinEditFiles()
{
   struct dirent **files;
   int result = scandir(BINEDIT_DIR, &files, NULL, NULL);
   if (result == -1) {
      return;
   }

   int num_files = result;
   for (unsigned i=0; i<num_files; i++) {
      if ((strcmp(files[i]->d_name, ".") == 0) || 
          (strcmp(files[i]->d_name, "..") == 0)) 
      {
         free(files[i]);
         continue;
      }
      std::string full_file = std::string(BINEDIT_DIR) + std::string("/") +
         std::string(files[i]->d_name);
	  if (!getenv("DYNINST_REWRITER_NO_UNLINK"))
	  {
		  dprintf("%s[%d]:  unlinking %s\n", FILE__, __LINE__, full_file.c_str());
		  unlink(full_file.c_str());
	  }
      free(files[i]);
   }
   free(files);
}

static bool cdBinDir()
{
   int result = chdir(BINEDIT_DIR);
   if (result != -1) {
      return true;
   }

   result = mkdir(BINEDIT_DIR, 0700);
   if (result == -1) {
      perror("Could not mkdir " BINEDIT_DIR);
      return false;
   }
   result = chdir(BINEDIT_DIR);
   if (result == -1) {
      perror("Could not chdir " BINEDIT_DIR);
      return false;
   }
   return true;
}

static bool cdBack()
{
   int result = chdir("..");
   if (result == -1) {
      perror("Could not chdir ..");
      return false;
   }
   return true;
}

static bool waitForCompletion(int pid, bool &app_crash, int &app_return)
{
   int result, status;
   int options = __WALL;
   do {
      result = waitpid(pid, &status, options);
   } while (result == -1 && errno == EINTR);

   if (result == -1) {
      perror("Could not collect child result");
      return false;
   }

   assert(!WIFSTOPPED(status));

   if (WIFSIGNALED(status)) {
      app_crash = true;
      app_return = WTERMSIG(status);
   }
   else if (WIFEXITED(status)) {
      app_crash = false;
      app_return = WEXITSTATUS(status);
   }
   else {
      assert(0);
   }

   return true;
}

static void killWaywardChild(int pid)
{
   int result = kill(pid, SIGKILL);
   if (result == -1) {
      return;
   }

   bool dont_care1;
   int dont_care2;
   waitForCompletion(pid, dont_care1, dont_care2);
}
#else
void clearBinEditFiles()
{
   assert(0); //IMPLEMENT ME
}

static bool cdBinDir()
{
   assert(0); //IMPLEMENT ME
   return false;
}

static bool cdBack()
{
   assert(0); //IMPLEMENT ME
   return false;
}

static void killWaywardChild(int)
{
   assert(0); //IMPLEMENT ME
}

static bool waitForCompletion(int, bool &, int &)
{
   assert(0); //IMPLEMENT ME
   return false;
}
#endif

bool runBinaryTest(BPatch *bpatch, RunGroup *group,
                   BPatch_binaryEdit *binEdit,
                   char *logfilename, char *humanlogname,
                   bool verboseFormat, bool printLabels,
                   int debugPrint, char *pidfilename,
                   test_results_t &test_result)
{
   bool cd_done = false;
   bool file_written = false;
   bool file_running = false;
   bool error = true;
   bool result;
   int app_return;
   bool app_crash;
   const char **child_argv = NULL;
   int pid;
   std::string outfile;

   test_result = UNKNOWN;

   clearBinEditFiles();

#if 0
   //  runTests is clobbering (perhaps intentionally?) the user's LD_LIBRARY_PATH
   //  which means that if we cd to the bin dir, and any rewritten binary depends
   //  on a library in the test directory (one level up), the library will no
   //  be found.  So rather than cd to the directory, keep the CWD in the test 
   //  directory and generate and execute the rewritten binary via relative path
   result = cdBinDir();
   if (!result) {
      goto done;
   }
   cd_done = true;
#endif

   outfile = std::string(BINEDIT_DIR) + std::string("/rewritten_") + std::string(group->mutatee);

   if (getenv("DYNINST_REWRITER_NO_UNLINK"))
   {
     //  we may in fact generate several binaries  (one for each rungroup)
     //  sequentially rewriting over them.  If DYNINST_REWRITER_NO_UNLINK is
     //  set, add a uniqification parameter to the filename, as well as a 
     //  report file that indicates which tests are represented in the
     //  binary
     
     outfile += std::string("_") + Dyninst::utos((unsigned)clock());
     std::string reportfile = outfile + std::string(".report");
     FILE *myrep = fopen(reportfile.c_str(), "w");
     fprintf(myrep, "Test group contains:\n");
     for (unsigned int i = 0; i < group->tests.size(); ++i)
       if (shouldRunTest(group, group->tests[i])) 
	 fprintf(myrep, "%s\n", group->tests[i]->name);
     fclose(myrep);
   }

   result = binEdit->writeFile(outfile.c_str());
   if (!result) {
      goto done;
   }
   file_written = true;

   child_argv = parseArgs(group, logfilename, humanlogname,
                          verboseFormat, printLabels, 
                          debugPrint, pidfilename);
   
   dprintf("%s[%d]:  starting rewritten process '%s ", FILE__, __LINE__, outfile.c_str());
   if (child_argv)
   {
     const char **argv_iter = child_argv;
     while(argv_iter)
     {
       const char *arg = *argv_iter;
       if (arg) dprintf("%s ", arg);
       else break;
       argv_iter++;
     }
   }
   dprintf("'...\n\n\n");
   pid = startNewProcessForAttach(outfile.c_str(), child_argv,
                                  NULL, NULL, false);
   if (pid == -1) {
      goto done;
   }
   file_running = false;

   result = waitForCompletion(pid, app_crash, app_return);
   if (!result)
      goto done;
   file_running = false;

   
   dprintf("%s[%d]:  after waitForCompletion: %s, result = %d\n", FILE__, __LINE__, app_crash ? "crashed" : "no crash", app_return);

   if ((app_crash)  || (app_return != 0))
   {
     parse_mutateelog(group);
     test_result = UNKNOWN;
   }
   else {
     test_result = PASSED;
   }
   
   error = false;
 done:

   if (error)
      test_result = FAILED;
   if (cd_done)
      cdBack();
   if (file_written)
      clearBinEditFiles();
   if (file_running)
      killWaywardChild(pid);
   if (child_argv)
      delete [] child_argv;
      
   return !error;  
}
