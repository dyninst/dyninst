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

// $Id: test_lib_mutateeStart.C,v 1.1 2008/10/30 19:21:46 legendre Exp $
// Functions Dealing with mutatee Startup

#if !defined(COMPLIB_DLL_BUILD)
#define COMPLIB_DLL_BUILD
#endif

#include "dyninst_comp.h"
#include "test_lib.h"
#include "util.h"
#include "ResumeLog.h"
#include "MutateeStart.h"
#include <stdlib.h>
#include <string>
using namespace std;

#if defined(os_linux_test) || defined(os_freebsd_test)

#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

static void clearBinEditFiles()
{
   struct dirent **files;
   const char *binedit_dir = get_binedit_dir();
   int result = scandir(binedit_dir, &files, NULL, NULL);
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
      std::string full_file = std::string(binedit_dir) + std::string("/") +
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
   const char *binedit_dir = get_binedit_dir();
   int result = chdir(binedit_dir);
   if (result != -1) {
      return true;
   }

   result = mkdir(binedit_dir, 0700);
   if (result == -1) {
      perror("Could not mkdir binaries");
      return false;
   }
   result = chdir(binedit_dir);
   if (result == -1) {
      perror("Could not chdir binaries");
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
   int options = 0;

#if defined(__WALL)
   options = __WALL;
#endif

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

bool runBinaryTest(RunGroup *group, ParameterDict &params, test_results_t &test_result)
{
   bool cd_done = false;
   bool file_written = false;
   bool file_running = false;
   bool error = true;
   bool result;
   int app_return;
   Dyninst::PID pid;
   bool app_crash;
   const char **child_argv = NULL;
   std::string outfile, mutatee_string;
   BPatch_binaryEdit *binEdit;

   int unique_id = params["unique_id"]->getInt();
   
   const char *binedit_dir = get_binedit_dir();
   if (unique_id) {
      unsigned buffer_len = strlen(BINEDIT_BASENAME) + 32;
      char *buffer = (char *) malloc(buffer_len);
      snprintf(buffer, buffer_len-1, "%s.%d", BINEDIT_BASENAME, unique_id);
      if (strcmp(buffer, binedit_dir) == 0) {
         free(buffer);
      }
      else {	    
         binedit_dir = buffer;
         set_binedit_dir(buffer);
      }
   }

   test_result = UNKNOWN;

   clearBinEditFiles();

   result = cdBinDir();
   if (!result) {
      goto done;
   }
   cd_done = true;

   outfile = std::string("rewritten_") + std::string(group->mutatee);

#if !defined(os_windows_test)
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
#endif
   
   binEdit = (BPatch_binaryEdit *) params["appBinaryEdit"]->getPtr();
   result = binEdit->writeFile(outfile.c_str());
   if (!result) {
      goto done;
   }
   file_written = true;

   if (cd_done) {
      cdBack();
      cd_done = false;
   }

   outfile = binedit_dir + std::string("/") + outfile;
   dprintf("%s[%d]:  starting rewritten process '%s ", FILE__, __LINE__, outfile.c_str());
   mutatee_string = launchMutatee(outfile, group, params);
   if (mutatee_string == string(""))
      goto done;

   registerMutatee(mutatee_string);
   pid = getMutateePid(group);
   assert(pid != NULL_PID);

   result = waitForCompletion(pid, app_crash, app_return);
   if (!result)
      goto done;
   file_running = false;

   
   dprintf("%s[%d]:  after waitForCompletion: %s, result = %d\n", FILE__, __LINE__, app_crash ? "crashed" : "no crash", app_return);

   if ((app_crash)  || (app_return != 0))
   {
      parse_mutateelog(group, params["mutatee_resumelog"]->getString());
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
   if (file_written && !params["noClean"]->getInt())
      clearBinEditFiles();
   if (file_running)
      killWaywardChild(pid);
   if (child_argv)
      delete [] child_argv;
      
   return !error;  
}
