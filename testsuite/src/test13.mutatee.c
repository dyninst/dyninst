#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mutatee_util.h"

#if !defined(os_windows)
#include <unistd.h>
#else
#include <windows.h>
#endif

#define NTHRD 4

volatile int done;
volatile int proc_current_state;

void *init_func(void *arg)
{
   while (!done);
   return arg;
}

int attached_fd = 0;
void parse_args(int argc, char *argv[])
{
   int i;
   char *logfilename = NULL;

   for (i=0; i<argc; i++)
   {
     if (!strcmp(argv[i], "-log")) {
       if ((i + 1) >= argc) {
	 fprintf(stderr, "Missing log file name\n");
	 exit(-1);
       }
       i += 1;
       logfilename = argv[i];
     } else if (strstr(argv[i], "-attach")) {
#if defined(os_windows)
        attached_fd = -1;
#else
        if (++i == argc) break;
        attached_fd = atoi(argv[i]);
#endif
     }
   }
   if ((logfilename != NULL) && (strcmp(logfilename, "-") != 0)) {
     outlog = fopen(logfilename, "a");
     if (NULL == outlog) {
       fprintf(stderr, "Error opening log file %s\n", logfilename);
       exit(-1);
     }
     errlog = outlog;
   } else {
     outlog = stdout;
     errlog = stderr;
   }
}

int isAttached = 0;
/* Check to see if the mutator has attached to us. */
int checkIfAttached()
{
    return isAttached;
}

int main(int argc, char *argv[])
{
   unsigned i;
   thread_t threads[NTHRD];

#ifndef os_windows
   char c = 'T';
#endif

#if defined(os_osf)
   return 0;
#endif

   /* *DEBUG* */
   /* fprintf(stderr, "test13.mutatee: in main()\n"); */

   initThreads();
   /* fprintf(stderr, "test13.mutatee: initialized threads\n"); /* *DEBUG* */

   parse_args(argc, argv);
   /* fprintf(stderr, "test13.mutatee: parsed args\n"); /* *DEBUG* */

   for (i=0; i<NTHRD; i++)  {
       threads[i] = spawnNewThread((void *) init_func, (void *) i);
   }
   /* fprintf(stderr, "test13.mutatee: spawned threads\n"); /* *DEBUG* */

#ifndef os_windows
   if (attached_fd) {
     /* fprintf(stderr, "test13.mutatee: writing to pipe\n"); /* *DEBUG* */
      if (write(attached_fd, &c, sizeof(char)) != sizeof(char)) {
         fprintf(stderr, "*ERROR*: Writing to pipe\n");
         exit(-1);
      }
      /* fprintf(stderr, "test13.mutatee: closing pipe\n"); /* *DEBUG* */
      close(attached_fd);
      /* fprintf(stderr, "test13.mutatee: closed pipe\n"); /* *DEBUG* */
      while (!checkIfAttached()) {
#if !defined(os_windows)
	usleep(1);
#endif
      }
   }
   /* fprintf(stderr, "test13.mutatee: wrote to pipe\n"); /* *DEBUG* */
#else
   if (attached_fd)
      while (!checkIfAttached());
#endif

   /* fprintf(stderr, "test13.mutatee: logging status\n"); /* *DEBUG* */
   logstatus("[%s:%d]: stage 1 - all threads created\n", __FILE__, __LINE__);
   /* fprintf(stderr, "test13.mutatee: logged status\n"); /* *DEBUG* */
   while (proc_current_state == 0);
   logstatus("[%s:%d]: stage 2 - allowing threads to exit\n", __FILE__, __LINE__);
   done = 1;
   for (i=0; i<NTHRD; i++)
   {
      joinThread(threads[i]);
   }
   logstatus("[%s:%d]: stage 3 - all threads joined\n", __FILE__, __LINE__);
   if ((outlog != NULL) && (outlog != stdout)) {
     fclose(outlog);
   }
   return 0;
}
