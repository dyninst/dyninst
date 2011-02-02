/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

#include <sys/types.h>
#include <unistd.h>
#include "ParameterDict.h"
#include "test_info_new.h"

static pid_t run_local(char **args)
{
   pid_t pid = fork();
   if (pid != 0) {
      return pid;
   }

   execvp(args[0], args);
   exit(-1);
}

#if defined(cap_launchmon)
#include "lmon_api/lmon_fe.h"

static char **getLaunchParams(char *executable, char *args[], const char *num);
static bool init_lmon();

int LMONInvoke(RunGroup *, ParameterDict params, char *test_args[], char *daemon_args[], bool attach)
{
   lmon_rc_e rc;
   int session;
   unsigned proctable_size, actual_proctable_size;
   MPIR_PROCDESC_EXT *proctable = NULL;

   if (!init_lmon()) {
      fprintf(stderr, "Failed to init launchmon\n");
      return -1;
   }

   char **new_test_args = getLaunchParams(test_args[0], test_args+1, "1");

   rc = LMON_fe_createSession(&session);
   if (rc != LMON_OK) {
      fprintf(stderr, "Failed to create session\n");
      return -1;
   }

   char *launcher_host = NULL;
   if (params.find("launcher_host") != params.end())
      launcher_host = params["launcher_host"]->getString();

   if (attach) {
      pid_t mpirun_pid = run_local(new_test_args);
      fprintf(stderr, "Executing mpirun from test_driver\n");
      sleep(10); //Want some better way to let mpirun get running
      fprintf(stderr, "Attaching to mpirun from test_driver\n");
      rc = LMON_fe_attachAndSpawnDaemons(session,
                                         launcher_host,
                                         mpirun_pid,
                                         daemon_args[0],
                                         daemon_args,
                                         NULL, NULL);
   }
   else {
      rc = LMON_fe_launchAndSpawnDaemons(session, 
                                         launcher_host,
                                         new_test_args[0],
                                         new_test_args,
                                         daemon_args[0],
                                         daemon_args,
                                         NULL, NULL);
   }
   
   free(new_test_args);

   return session;
}

static bool init_lmon()
{
   lmon_rc_e rc;

   static bool initialized = false;
   if (initialized)
      return true;
   initialized = true;

   rc = LMON_fe_init( LMON_VERSION);
   return (rc == LMON_OK);
}

#if defined(os_cnl_test)
static char **getLaunchParams(char *executable, char *args[], char *num)
{
   
   int count = 0;
   for (char *counter[] = args; *counter; counter++, count++);
   char **new_args = (char **) malloc(sizeof(char *) * (count+5));
   new_args[0] = "aprun";
   new_args[1] = "-n";
   new_args[2] = num;
   for (unsigned i=0; i<=count; i++)
      new_args[3+i] = args[i];
   return new_args;
}
#elif defined(os_linux_test)
static char **getLaunchParams(char *executable, char *args[], const char *num)
{
   
   int count = 0;
   for (char **counter = args; *counter; counter++, count++);
   char **new_args = (char **) malloc(sizeof(char *) * (count+5));
   new_args[0] = "orterun";
   new_args[1] = "-n";
   new_args[2] = const_cast<char *>(num);
   for (unsigned i=0; i<=count; i++)
      new_args[3+i] = args[i];
   return new_args;
}
#elif defined(os_bg_test)
static char **getLaunchParams(char *executable, char *args[], const char *num)
{   
   int count = 0;
   for (char **counter = args; *counter; counter++, count++);
   char **new_args = (char **) malloc(sizeof(char *) * (count+7));
   new_args[0] = "mpirun";
   new_args[1] = "-np";
   new_args[2] = const_cast<char *>(num);
   for (unsigned i=0; i<=count; i++)
      new_args[3+i] = args[i];
   return new_args;
}
#endif

#else

//This is a test function, for testing the launchmon infrastructure on systems without
// actually using launchmon.  It shouldn't see use in normal circumstances.
int LMONInvoke(RunGroup *group, ParameterDict params, char *test_args[], char *daemon_args[], bool attach)
{
   if (attach) {
      pid_t mutatee_pid = run_local(test_args);
      if (mutatee_pid == -1) {
         return false;
      }
      
      int count, i;
      char **cur;
      for (count = 0, cur = daemon_args; *cur; count++, cur++);
      char **new_daemon_args = (char **) malloc(sizeof(char *) * (count+2));
      for (i=0; i<count; i++) {
         new_daemon_args[i] = daemon_args[i];
      }
      char newbuffer[64];
      snprintf(newbuffer, 64, "%d:%d", group->index, mutatee_pid);
      
      new_daemon_args[i++] = "-given_mutatee";
      new_daemon_args[i++] = newbuffer;
      new_daemon_args[i++] = NULL;

      pid_t mutator_pid = run_local(new_daemon_args);
      return mutator_pid;
   }
   else {
      pid_t mutator_pid = run_local(daemon_args);
      return mutator_pid;
   }
}

#endif
