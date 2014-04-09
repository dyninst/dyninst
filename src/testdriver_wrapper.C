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
#include <string.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "remotetest.h"

static char *hostname = NULL;
static int port = 0;
static char **args = NULL;

static void parse_args(int argc, char *argv[]);
static void parse_env(char *buffer);
static void parse_execargs(char *buffer);
static void parse_ldd(char *buffer);
static void parse_go();
static Connection *connection;
static bool setcwd();

static int gargc;
static char **gargv;

FILE *debug_log = stderr;
FILE *getDebugLog()
{
   return debug_log;
}

#if !defined(os_freebsd_test)
#include <link.h>
void copy_iolibs()
{
  char cmd_line[4096], *last_slash;
  struct link_map *lm;
  for (lm = _r_debug.r_map; lm != NULL; lm = lm->l_next) {
    if (!lm->l_name || !lm->l_name[0]) 
      continue;
    snprintf(cmd_line, 4096, "mkdir -p io_libs%s", lm->l_name);
    last_slash = strrchr(cmd_line, '/');
    if (last_slash) *last_slash = '\0';
    fprintf(debug_log, "%s\n", cmd_line);
    system(cmd_line);
    snprintf(cmd_line, 4096, "cp -u %s io_libs%s", lm->l_name, lm->l_name);
    last_slash = strrchr(cmd_line, '/');
    if (last_slash) *last_slash = '\0';
    fprintf(debug_log, "%s\n", cmd_line);
    system(cmd_line);
  }
}
#else
void copy_iolibs()
{
}
#endif

int main(int argc, char *argv[])
{
   struct rlimit infin;
   int result;

// static volatile int loop = 0;
// while (loop == 0);
   
   infin.rlim_cur = RLIM_INFINITY;
   infin.rlim_max = RLIM_INFINITY;
   result = setrlimit(RLIMIT_CORE, &infin);

   gargc = argc;
   gargv = argv;

   setcwd();
   debug_log = fopen("./wrapper_output", "w");
   
//   copy_iolibs();

   parse_args(argc, argv);

   connection = new Connection(hostname, port);
   assert(!connection->hasError());

   setcwd();   

   char *buffer = NULL;
   for (;;) {
      bool result = connection->recv_message(buffer);
      if (!result) {
         fprintf(stderr, "Timeout waiting for message in wrapper\n");
         return -1;
      }

      if (buffer[0] == 'E' && buffer[1] == ':') {
         parse_env(buffer);
      }
      else if (buffer[0] == 'A' && buffer[1] == ':') {
         parse_execargs(buffer);
      }
      else if (buffer[0] == 'L' && buffer[1] == ':') {
         parse_ldd(buffer);
      }
      else if (buffer[0] == 'G' && buffer[1] == ':') {
         parse_go();
      }
   }

   return 0;
}

#if !defined(PATH_MAX)
#define PATH_MAX 4096
#endif

static bool setcwd()
{
   const char *exe = "/proc/self/exe";
   char newcwd[PATH_MAX+1];
   char *result = realpath(exe, newcwd);
   if (!result) {
      perror("Couldn't set cwd.  realpath error");
      return false;
   }
   char *end = strrchr(result, '/');
   if (!end || end == result) {
      fprintf(stderr, "Unexpected return from realpath: %s\n", result);
      return false;
   }

   *end = '\0';
   int iresult = chdir(result);
   if (iresult == -1) {
      perror("Couldn't set cwd.  chdir error");
      return false;
   }
   return true;
}

static void parse_env(char *buffer)
{
   char *cur = buffer+2;
   int env_count = atoi(cur);
   assert(env_count);
   while (*(cur++) != ':');
   
   for (int i=0; i<env_count; i++) {
      char *evar, *eval;
      assert(*cur != '\0');
      evar = cur;
      while (*(cur++) != '\0');
      eval = cur;
      while (*(cur++) != '\0');
      setenv(evar, eval, 1);
   }
}

static void parse_execargs(char *buffer) {
   char *cur = buffer+2;
   int arg_count = atoi(cur);
   assert(arg_count);
   while (*(cur++) != ':');

   args = (char **) malloc(sizeof(char *) * (arg_count+gargc+7));
   int i, j=0;
   //args[j++] = "/g/g0/legendre/tools/dyninst/githead/ppc32_bgp_ion/lib/ld.so.1";
   //args[j++] = "/g/g0/legendre/tools/valgrind/bin/valgrind";
   //args[j++] = "--tool=memcheck";
   for (i=0; i<arg_count; i++) {
      args[j++] = strdup(cur);
      while (*(cur++) != '\0');
   }
   
   args[j++] = const_cast<char *>("-socket_fd");
   char *socket_fd = (char *) malloc(16);
   snprintf(socket_fd, 16, "%d", connection->getFD());
   args[j++] = socket_fd;

   for (i=1; i<gargc; i++) {
      args[j++] = gargv[i];
   }

   args[j++] = NULL;
}

static void parse_go()
{
   int error;

   assert(args);
   execv(args[0], args);
   error = errno;
   if (debug_log)
     fprintf(debug_log, "Failed to execv %s: %s\n", args[0], strerror(error));
   assert(0);
}

static void runLdd(std::string filename, MessageBuffer &buf)
{
   char cmd_line[4092];
   snprintf(cmd_line, 4092, "ldd %s", filename.c_str());

   buf.add(cmd_line, strlen(cmd_line));
   buf.add("\n", 1);
   FILE *f = popen(cmd_line, "r");
   if (!f) {
      buf.add("ldd error\n", strlen("ldd error\n")+1);
      return;
   }
   while (!feof(f)) {
      char buffer[257];
      ssize_t num_read = fread(buffer, 1, 256, f);
      buf.add(buffer, num_read);
   }
   pclose(f);   
}

static void parse_ldd(char *buffer)
{
   assert(args);
   char *libname = buffer+2;

   MessageBuffer result;
   runLdd(args[0], result);
   result.add("\n", 1);
   runLdd(std::string(libname), result);
   result.add("\n", 2);

   bool bresult = connection->send_message(result);
   assert(bresult);
}

static void parse_args(int argc, char *argv[])
{
   for (int i = 0; i < argc; i++) {
      if (strcmp(argv[i], "-redirect-debug") == 0) {
         char *filename = argv[i+1];
         assert(filename);
         
         int fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 0666);
         if (fd == -1) {
            perror("Could not open file for debug output");
            continue;
         }
         dup2(fd, 1);
         dup2(fd, 2);
         break;
      }
      else if (strcmp(argv[i], "-hostname") == 0) {
         assert(i+1 < argc);
         hostname = argv[++i];
      }
      else if (strcmp(argv[i], "-port") == 0) {
         assert(i+1 < argc);
         port = atoi(argv[++i]);
      }
   }

   if (!hostname && argc == 4) {
     //LaunchMON sometimes compacts all arguments into a space separated argv[1]
     int num_spaces = 0, i = 0;
     char *str = argv[1];
     char **new_argv = NULL, *entry = NULL;
     do {
       if (str[i] == ' ') num_spaces++;
     } while (str[i++]);
     new_argv = (char **) malloc(sizeof(char *) * (num_spaces+3));
     i = 0;
     for (entry = strtok(str, " "); entry != NULL; entry = strtok(NULL, " ")) {
       new_argv[i++] = strdup(entry);
     }
     new_argv[i++] = argv[2];
     new_argv[i++] = argv[3];
     new_argv[i] = NULL;
     parse_args(i, new_argv);
     if (hostname) {
       gargc = i;
       gargv = new_argv;
     }
   }
}

void handle_message(char *buffer) {
   assert(0);
}

