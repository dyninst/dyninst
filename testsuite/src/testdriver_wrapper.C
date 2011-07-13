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

int main(int argc, char *argv[])
{
   struct rlimit infin;
   int result;


   static volatile int loop = 0;
//   while (loop == 0);


   infin.rlim_cur = RLIM_INFINITY;
   infin.rlim_max = RLIM_INFINITY;
   result = setrlimit(RLIMIT_CORE, &infin);

   gargc = argc;
   gargv = argv;

   setcwd();

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

   args = (char **) malloc(sizeof(char *) * (arg_count+gargc+3));
   int i, j;
   for (i=0; i<arg_count; i++) {
      args[i] = strdup(cur);
      while (*(cur++) != '\0');
   }
   
   args[i++] = const_cast<char *>("-socket_fd");
   char *socket_fd = (char *) malloc(16);
   snprintf(socket_fd, 16, "%d", connection->getFD());
   args[i++] = socket_fd;

   for (j=1; j<gargc; j++) {
      args[i++] = gargv[j];
   }

   args[i++] = NULL;
}

static void parse_go()
{
   assert(args);
   execv(args[0], args);
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
         
         int fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT);
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
}

void handle_message(char *buffer) {
   assert(0);
}

