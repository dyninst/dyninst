#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#ifdef __cplusplus
int mutateeCplusplus = 1;
#else
int mutateeCplusplus = 0;
#endif

#ifndef COMPILER
#define COMPILER ""
#endif
const char *Builder_id=COMPILER; /* defined on compile line */

#define NTHRD 4
volatile int done;
volatile int proc_current_state;

void *init_func(void *arg)
{
   while (!done);
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

   if(argc == 1) {
      printf("Mutatee %s [%s]:\"%s\"\n", argv[0], 
             mutateeCplusplus ? "C++" : "C", Builder_id);
      return 0;
   }

   pthread_attr_init(&attr);
   pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

   parse_args(argc, argv);

   if (attached_fd) {
      if (write(attached_fd, &c, sizeof(char)) != sizeof(char)) {
         fprintf(stderr, "*ERROR*: Writing to pipe\n");
         exit(-1);
      }
      close(attached_fd);
      sleep(5); /* wait for mutator to attach */
   }

   for (i=0; i<NTHRD; i++)
   {
      pthread_create(&threads[i], &attr, init_func, (void *) i);
   }
   
   fprintf(stderr, "[%s:%d]: stage 1 - all threads created\n", __FILE__, __LINE__);

   while (proc_current_state == 0) sched_yield();

   fprintf(stderr, "[%s:%d]: stage 2 - allowing threads to exit\n", __FILE__, __LINE__);
   done = 1;

   for (i=0; i<NTHRD; i++)
   {
      pthread_join(threads[i], &ret_val);
   }
   fprintf(stderr, "[%s:%d]: stage 3 - all threads joined\n", __FILE__, __LINE__);
   return 0;
}
