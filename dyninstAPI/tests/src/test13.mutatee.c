#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
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
   pthread_t threads[NTHRD];
   pthread_attr_t attr;
   void *ret_val;

   pthread_attr_init(&attr);
   parse_args(argc, argv);

   for (i=0; i<NTHRD; i++)
   {
      pthread_create(&threads[i], &attr, init_func, (void *) i);
   }
   if (attached_fd)
      write(attached_fd, &c, sizeof(char));
   fprintf(stderr, "stage 1\n");
   while (proc_current_state == 0);
   fprintf(stderr, "stage 2\n");
   done = 1;
   for (i=0; i<NTHRD; i++)
   {
      pthread_join(threads[i], &ret_val);
   }
   fprintf(stderr, "stage 3\n");
   while (proc_current_state == 1);
   fprintf(stderr, "stage 4\n");
   return 0;
}
