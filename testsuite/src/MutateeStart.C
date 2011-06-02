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

#include "MutateeStart.h"
#include "ParameterDict.h"
#include "test_info_new.h"
#include "test_lib.h"
#include <assert.h>

using namespace std;

#if !defined(os_windows_test)

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#if !defined(os_linux_test)
static pid_t fork_mutatee() {
   return fork();
}
#else
static pid_t fork_mutatee() {
   /**
    * Perform a granchild fork.  This code forks off a child, that child
    * then forks off a granchild.  The original child then exits, making
    * the granchild's new parent init.  Both the original process and the
    * granchild then exit from this function.
    *
    * This works around a linux kernel bug in kernel version 2.6.9 to
    * 2.6.11, see https://www.dyninst.org/emails/2006/5678.html for
    * details.
    **/
   int status, result;
   pid_t gchild_pid, child_pid;
   int filedes[2];

   pipe(filedes);

   child_pid = fork();
   if (child_pid < 0) { // This is an error
      close(filedes[0]);
      close(filedes[1]);
      return child_pid;
   }

   if (child_pid) {
      //Read the grandchild pid from the child.
      do {
         result = read(filedes[0], &gchild_pid, sizeof(pid_t));
      } while (result == -1 && errno == EINTR);
      if (result == -1) {
         perror("Couldn't read from pipe");
      }

      int options = 0;
      do {
         result = waitpid(child_pid, &status, options);
         if (result != child_pid) {
            perror("Couldn't join child");
            break;
         }
      } while (!WIFEXITED(status));
      close(filedes[0]);
      close(filedes[1]);
      return gchild_pid;
   }
   //Child
   
   gchild_pid = fork();
   if (gchild_pid) {
      //Child pid, send grand child pid to parent then terminate
      result = write(filedes[1], &gchild_pid, sizeof(pid_t));
      if (result == -1) {
         perror("Couldn't write to parent");
      }         
      close(filedes[0]);
      close(filedes[1]);
      exit(0);
   }   

   //Grandchild
   close(filedes[0]);
   close(filedes[1]);
   return 0;
}
#endif
#endif

char **getCParams(std::string executable, const std::vector<std::string> &args)
{
   char **argv = (char **) malloc(sizeof(char *) * (args.size()+2));
   assert(argv);

   int offset = 0;
   if (executable != string("")) {
      argv[0] = const_cast<char *>(executable.c_str());
      offset = 1;
   }

   unsigned i;
   for (i=0; i<args.size(); i++) {
      argv[i+offset] = const_cast<char *>(args[i].c_str());
   }
   argv[i+offset] = NULL;
   return argv;
}

#if !defined(os_windows)

static int fds[2];
static bool fds_set = false;

static void AddArchAttachArgs(std::vector<std::string> &args, create_mode_t cm, start_state_t gs)
{
   if (cm == USEATTACH && gs != SELFATTACH)
   {
      /* Make a pipe that we will use to signal that the mutatee has started. */
      if (pipe(fds) != 0) {
         fprintf(stderr, "*ERROR*: Unable to create pipe.\n");
         return;
      }
      
      /* Create the argv string for the child process. */
      char fdstr[32];
      snprintf(fdstr, 32,"%d", fds[1]);
      
      args.push_back("-attach");
      args.push_back(fdstr);
      
      fds_set = true;
   }
   else {
      fds_set = false;
   }
}

extern FILE *getOutputLog();
extern FILE *getErrorLog();

static std::string launchMutatee_plat(std::string exec_name, const std::vector<std::string> &args, bool needs_grand_fork)
{   
   pid_t pid;
   if (needs_grand_fork)
      pid = fork_mutatee();
   else
      pid = fork();

   if (pid < 0) {
      return std::string("");
   }
   else if (pid == 0) 
   {
      // child
      if (fds_set)
         close(fds[0]);
      
      if (getOutputLog() != NULL) {
         int outlog_fd = fileno(getOutputLog());
         if (dup2(outlog_fd, 1) == -1) {
            fprintf(stderr, "Error duplicating log fd(1)\n");
         }
      }
      if (getErrorLog() != NULL) {
         int errlog_fd = fileno(getErrorLog());
         if (dup2(errlog_fd, 2) == -1) {
            fprintf(stderr, "Error duplicating log fd(2)\n");
         }
      }
      char *ld_path = getenv("LD_LIBRARY_PATH");
      char *new_ld_path = NULL;
      if (ld_path) {
         new_ld_path = (char *) malloc(strlen(ld_path) + strlen("./binaries:") + 2);
         strcpy(new_ld_path, "./binaries:");
         strcat(new_ld_path, ld_path);
         setenv("LD_LIBRARY_PATH", new_ld_path, 1);
      }
      else {
         setenv("LD_LIBRARY_PATH", "./binaries", 1);
      }

      char **argv = getCParams(exec_name, args);
      const char *c_exec_name = exec_name.c_str();

      execvp(exec_name.c_str(), (char * const *) argv);
      exec_name = std::string("./") + exec_name;
      execvp(exec_name.c_str(), (char * const *) argv);

      fprintf(stderr, "%s[%d]:  Exec failed!\n", __FILE__, __LINE__);
      exit(-1);
   }
   else 
   {
      // parent
      if (fds_set) 
      {
         close(fds[1]);  // We don't need the write side
      
         // Wait for the child to write to the pipe
         char ch;
         ssize_t result = read(fds[0], &ch, sizeof(char));
         if (result != sizeof(char)) {
            perror("read");
            fprintf(stderr, "*ERROR*: Error reading from pipe\n");
            return string("");
         }
         if (ch != 'T') {
            fprintf(stderr, "*ERROR*: Child didn't write expected value to pipe.\n");
            return string("");
         }
       
         /* Random Linux-ism: it's possible to close the pipe before the 
            mutatee returns from the write() system call matching the above
            read().  In this case, rather than ignore the close() because the
            write() is on its way out the kernel, Linux sends a SIGPIPE
            to the mutatee, which causes us no end of trouble.  read()ing
            an EOF from the pipe seems to alleviate this problem, and seems
            more reliable than a sleep(1).  The condition test if we somehow
            got any /extra/ bytes on the pipe. */
         if( read( fds[0], & ch, sizeof( char ) ) != 0 ) {
            fprintf(stderr, "*ERROR*: Shouldn't have read anything here.\n");
            return string("");
         }
      
         close( fds[0] ); // We're done with the pipe
      }
      char ret[32];
      snprintf(ret, 32, "%d", pid);
      return std::string(ret);
   }
}
#endif

bool shouldLaunch(RunGroup *group, ParameterDict &params)
{
   bool in_runtests = params["in_runtests"]->getInt();
   if (!in_runtests)
      return true;
   create_mode_t cmode = (create_mode_t) params["createmode"]->getInt();
#if defined(os_linux_test)
   return (cmode == USEATTACH);
#endif
}

std::string launchMutatee(std::string executable, std::vector<std::string> &args, RunGroup *group, ParameterDict &params)
{
   char group_num[32];
   snprintf(group_num, 32, "%d", group->index);

   bool in_runtests = params["in_runtests"]->getInt();
   if (!shouldLaunch(group, params))
      return std::string(group_num) + ":-1";

   bool needs_grand_fork = false;
#if defined(os_linux)
   need_grand_fork = (group->createmode == USEATTACH);
#endif

   string ret = launchMutatee_plat(executable, args, needs_grand_fork);
   if (ret == string(""))
      return string("");

   return string(group_num) + ":" + ret;   
}

std::string launchMutatee(std::string executable, RunGroup *group, ParameterDict &params)
{
   string exec_name;
   vector<string> args;

   bool result = getMutateeParams(group, params, exec_name, args);
   if (!result) 
      return string("");

   if (executable != string(""))
      exec_name = executable;

   return launchMutatee(exec_name, args, group, params);
}

string launchMutatee(RunGroup *group, ParameterDict &params)
{
   return launchMutatee(string(""), group, params);
}

bool getMutateeParams(RunGroup *group, ParameterDict &params, std::string &exec_name,
                      std::vector<std::string> &args)
{
   exec_name = group->mutatee;
   args.push_back(exec_name);
   char *logfilename = params["logfilename"]->getString();
   if (logfilename) {
      args.push_back("-log");
      args.push_back(logfilename);
   }

   char *human_logfilename = params["humanlogname"]->getString();
   if (logfilename) {
      args.push_back("-humanlog");
      args.push_back(human_logfilename);
   }

   if (params["debugPrint"]->getInt()) {
      args.push_back("-verbose");
   }

   if (params["dboutput"]->getString()) {
      args.push_back("-dboutput");
   }

   create_mode_t cm = (create_mode_t) params["createmode"]->getInt();
   start_state_t gs = group->state;
   AddArchAttachArgs(args, cm, gs);

   if (cm == USEATTACH && gs == SELFATTACH) {
      args.push_back("-customattach");
   }

   test_procstate_t ps = (test_procstate_t) params["processMode"]->getInt();
   if (ps == SingleProcess) 
      args.push_back("-sp");
   else if (ps == MultiProcess)
      args.push_back("-mp");

   test_threadstate_t ts = (test_threadstate_t) params["threadMode"]->getInt();
   if (ts == SingleThreaded)
      args.push_back("-st");
   else if (ts == MultiThreaded) {
      args.push_back("-mt");
      char s[64];
      snprintf(s, 64, "%d", getNumThreads(params));
      args.push_back(s);
   }

   int signal_fd = params.find("signal_fd_out") != params.end() ? params["signal_fd_out"]->getInt() : -1;
   if (signal_fd != -1) {
      char s[64];
      snprintf(s, 64, "%d", signal_fd);
      args.push_back("-signal_fd");
      args.push_back(std::string(s));
   }

   for (int i = 0; i < group->tests.size(); i++) {
      if (!group->tests[i]->disabled) {
         args.push_back("-run");
         args.push_back(group->tests[i]->name);
      }
   }

   return true;
}

#if defined(os_bg_test)
#include <set>

static std::set<int> spawned_mutatees;
void registerMutatee(std::string mutatee_string)
{
   int pid;
   sscanf(mutatee_string.c_str(), "%d", &pid);
   assert(pid != -1);
   spawned_mutatees.insert(pid);
}

int getMutateePid(RunGroup *)
{
   std::set<int>::iterator i = spawned_mutatees.begin();
   assert(i != spawned_mutatees.end());
   int pid = *i;
   spawned_mutatees.erase(i);
   return pid;
}
#else
static std::map<int, std::string> spawned_mutatees;
void registerMutatee(std::string mutatee_string)
{
   int group_id;
   int pid;
   sscanf(mutatee_string.c_str(), "%d:%d", &group_id, &pid);
   if (pid != -1)
      spawned_mutatees[group_id] = mutatee_string;
}

int getMutateePid(RunGroup *group)
{
   std::map<int, std::string>::iterator i = spawned_mutatees.find(group->index);
   if (i == spawned_mutatees.end()) {
      i = spawned_mutatees.find(-1);
   }
   if (i == spawned_mutatees.end())
      return 0;

   std::string mutatee_string = i->second;
   int group_id;
   int pid;

   sscanf(mutatee_string.c_str(), "%d:%d", &group_id, &pid);
   assert(group->index == group_id || group_id == -1);

   spawned_mutatees.erase(i);

   return pid;
}
#endif

   
