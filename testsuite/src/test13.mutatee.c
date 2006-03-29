#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mutatee_util.h"

#if !defined(os_windows)
#include <unistd.h>
#endif

#define NTHRD 4

volatile int done;
volatile int proc_current_state;


void *init_func(void *arg)
{
   while (!done)
   {
      /*      sleep(1);*/
   }
   return arg;
}

int attached_fd;
void parse_args(int argc, char *argv[])
{
   int i;
   for (i=0; i<argc; i++)
   {
      if (strstr(argv[i], "-attach"))
      {
         if (++i == argc) break;
         attached_fd = atoi(argv[i]);
      }
   }
}

int main(int argc, char *argv[])
{
   unsigned i;
   char c = 'T';
   void *threads[NTHRD];

#if defined(os_osf)
   return 0;
#endif

   initThreads();

   parse_args(argc, argv);

   for (i=0; i<NTHRD; i++)  {
       threads[i] = spawnNewThread((void *) init_func, (void *) i);
   }

#if !defined(i386_unknown_nt4_0)
   if (attached_fd)
      write(attached_fd, &c, sizeof(char));
#endif
   fprintf(stderr, "[%s:%d]: stage 1 - all threads created\n", __FILE__, __LINE__);
   while (proc_current_state == 0);
   fprintf(stderr, "[%s:%d]: stage 2 - allowing threads to exit\n", __FILE__, __LINE__);
   done = 1;
   for (i=0; i<NTHRD; i++)
   {
      joinThread(threads[i]);
   }
   fprintf(stderr, "[%s:%d]: stage 3 - all threads joined\n", __FILE__, __LINE__);
   return 0;
}
