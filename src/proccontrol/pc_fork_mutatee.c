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

#if defined(os_freebsd_test)
#include <assert.h>
#endif

#include "pcontrol_mutatee_tools.h"
#include "solo_mutatee_boilerplate.h"

#define EXIT_CODE 4
static testlock_t fork_lock;
static testlock_t init_lock;
static int myerror = 0;
static unsigned max_forker = 0;
volatile int pc_fork_here = 0;

typedef struct {
   pid_t pid;
   int is_threaded;
} fork_results_t;
fork_results_t fork_results[MAX_POSSIBLE_THREADS];

static void bp_func()
{
   pc_fork_here++;
}

#if defined(os_linux_test) || defined(os_freebsd_test)
static int check_if_threaded()
{
   return 0;
}
#else
static int check_if_threaded()
{
   assert(0);
}
#endif

static int threadFunc(int myid, void *data)
{
   int filedes[2];
   int result = 0;
   int is_threaded;
   int status;
   pid_t pid;

   data = NULL;
   testLock(&init_lock);
   testUnlock(&init_lock);

   result = pipe(filedes);
   if (result == -1) {
      perror("pipe");
      myerror = 1;
      return -1;
   }

   testLock(&fork_lock);
   if (((unsigned) myid) > max_forker) 
      max_forker = myid;
   pid = fork();
   if (pid)
      testUnlock(&fork_lock);

   if (pid == -1) {
      perror("fork");
      myerror = 1;
      close(filedes[0]);
      close(filedes[1]);
      return -1;
   }
   if (!pid) {
      /*Child*/
      close(filedes[0]);
      is_threaded = check_if_threaded();
      result = write(filedes[1], &is_threaded, sizeof(int));
      close(filedes[1]);
      if (result == -1) {
         myerror = -1;
         perror("write");
      }
      bp_func();
      exit(EXIT_CODE);
   }
   /*Parent*/
   close(filedes[1]);
   do {
      result = read(filedes[0], &is_threaded, sizeof(int));
      if (result == -1 && errno != EINTR) {
         perror("read");
         myerror = 1;
         return -1;
      }
   } while (result == -1 && errno == EINTR);
   close(filedes[0]);
   result = waitpid(pid, &status, 0);
   if (result == -1) {
      perror("waitpid");
      myerror = 1;
   }
   if (!WIFEXITED(status)) {
      if( WIFSIGNALED(status) ) {
          output->log(STDERR, "Child received unexpected signal: %d\n", 
                  WTERMSIG(status));
      }else{
          output->log(STDERR, "Unexpected waitpid return: %x\n", status);
      }
      myerror = 1;
      return -1;
   }
   if (WEXITSTATUS(status) != EXIT_CODE) {
      output->log(STDERR, "Bad return code from child process\n");
      myerror = 1;
      return -1;
   }
   fork_results[myid].pid = pid;
   fork_results[myid].is_threaded = is_threaded;
   
   return 0;
}

//Basic test for create/attach and exit.
int pc_fork_mutatee()
{
   int result;
   unsigned i;
   send_addr addr_msg;
   forkinfo fork_msg;
   syncloc msg;

   initLock(&fork_lock);
   initLock(&init_lock);
   testLock(&init_lock);
   memset(fork_results, 0, sizeof(fork_results));

   result = initProcControlTest(threadFunc, NULL);
   if (result != 0) {
      output->log(STDERR, "Initialization failed\n");
      testUnlock(&init_lock);
      return -1;
   }

   addr_msg.code = SENDADDR_CODE;
   addr_msg.addr = getFunctionPtr((intptr_t *)bp_func);
   result = send_message((unsigned char *) &addr_msg, sizeof(addr_msg));
   if (result == -1) {
      output->log(STDERR, "Failed to send addr message\n");
      return -1;
   }

   result = recv_message((unsigned char *) &msg, sizeof(syncloc));
   if (result == -1) {
      output->log(STDERR, "Failed to recv sync message\n");
      testUnlock(&init_lock);
      return -1;
   }
   if (msg.code != SYNCLOC_CODE) {
      output->log(STDERR, "Recieved unexpected sync message\n");
      testUnlock(&init_lock);
      return -1;
   }

   testUnlock(&init_lock);
   threadFunc(0, NULL);
          
   result = finiProcControlTest(0);
   if (result != 0) {
      output->log(STDERR, "Finalization failed\n");
      return -1;
   }
   
   fork_msg.code = FORKINFO_CODE;
   fork_msg.is_done = 0;
   for (i = 0; i <= max_forker; i++)
   {
      fork_msg.pid = fork_results[i].pid;
      fork_msg.is_threaded = fork_results[i].is_threaded;
      if (i == max_forker)
         fork_msg.is_done = 1;
      result = send_message((unsigned char *) &fork_msg, sizeof(fork_msg));
      if (result == -1) {
         output->log(STDERR, "Failed to send fork message\n");
         return -1;
      }
   }

   if (myerror == 0) {
      test_passes(testname);
      return 0;
   }
   return -1;
}
