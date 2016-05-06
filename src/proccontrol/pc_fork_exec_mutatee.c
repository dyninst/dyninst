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
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#include "pcontrol_mutatee_tools.h"
#include "solo_mutatee_boilerplate.h"

#define EXIT_CODE 4
static testlock_t fork_lock;
static int myerror = 0;

static char *exec_name;
static int threadFunc(int myid, void *data)
{
   int result = 0;
   int status;
   pid_t pid;

   data = NULL;
   myid = 0;

   testLock(&fork_lock);
   pid = fork();
   if (pid)
      testUnlock(&fork_lock);

   char *args[4];
   args[0] = exec_name;
   args[1] = strdup("-run");
   args[2] = strdup("pc_exec_targ");
   args[3] = NULL;

   if (pid == -1) {
      perror("fork");
      myerror = 1;
      return -1;
   }
   if (!pid) {
      /*Child*/
      if (result == -1) {         
         perror("write");
         exit(-1);
      }
      execv(exec_name, args);
      perror("execv");
      exit(-1);
   }
   /*Parent*/
   result = waitpid(pid, &status, 0);
   if (result == -1) {
      perror("waitpid");
      myerror = 1;
   }
   if (!WIFEXITED(status)) {
      output->log(STDERR, "Unexpected waitpid return\n");
      myerror = 1;
      return -1;
   }
   if (WEXITSTATUS(status) != EXIT_CODE) {
      output->log(STDERR, "Bad return code from child process\n");
      myerror = 1;
      return -1;
   }
   
   return 0;
}

#if defined(os_linux_test) || defined(os_freebsd_test)
#include <dlfcn.h>
#include <assert.h>
static void loadTestA()
{
   void *result;
   result = dlopen("./libtestA.so", RTLD_LAZY);
   if (result) 
      return;
   result = dlopen("./libtestA_m32.so", RTLD_LAZY);
   assert(result);
}
#endif


int pc_fork_exec_mutatee()
{
   int result;
   syncloc msg;

   exec_name = strdup(gargv[0]);
   strncpy(exec_name, "pc_exec_targ", strlen("pc_exec_targ"));

   initLock(&fork_lock);

   loadTestA();

   result = initProcControlTest(threadFunc, NULL);
   if (result != 0) {
      output->log(STDERR, "Initialization failed\n");
      return -1;
   }

   threadFunc(0, NULL);
        
   result = finiProcControlTest(0);
   if (result != 0) {
      output->log(STDERR, "Finalization failed\n");
      return -1;
   }

   msg.code = SYNCLOC_CODE;
   result = send_message((unsigned char *) &msg, sizeof(msg));
   if (result == -1) {
      output->log(STDERR, "Failed to send addr message\n");
      return -1;
   }
   
   if (myerror == 0) {
      test_passes(testname);
      return 0;
   }
   return -1;
}
