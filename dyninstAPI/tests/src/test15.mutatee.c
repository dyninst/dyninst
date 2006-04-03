#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <assert.h>
#include <unistd.h>
#include <limits.h>

#define NTHRD 5
pthread_t thrds[NTHRD];

int thr_ids[NTHRD] = {0, 1, 2, 3, 4};
int ok_to_exit[NTHRD] = {0, 0, 0, 0, 0};

/* oneTimeCodes will set these to the tid for the desired thread */
volatile pthread_t sync_test = 0;
volatile pthread_t async_test = 0;
int sync_failure = 0;
int async_failure = 0;

volatile unsigned thr_exits;

void check_sync()
{
   pthread_t tid = pthread_self();
   int id = -1, i;
   for(i = 0; i < NTHRD; i++) {
      if(thrds[i] == tid) {
         id = i;
         break;
      }
   }

   if(tid == sync_test) {
      printf("Thread %d [tid %lu] - oneTimeCode completed successfully\n", 
             id, tid);
      ok_to_exit[id] = 1;
      return;
   }
   else if(sync_test != 0)
      fprintf(stderr, "%s[%d]: ERROR: Thread %d [tid %lu] - mistakenly ran oneTimeCode for thread with tid %lu\n", __FILE__, __LINE__, id, tid, sync_test);
   else
      fprintf(stderr, "%s[%d]: ERROR: Thread %d [tid %lu] - sync_test is 0\n", __FILE__, __LINE__, id, tid);
   sync_failure++;
}

void check_async()
{
   pthread_t tid = pthread_self();
   int id = -1, i;
   for(i = 0; i < NTHRD; i++) {
      if(thrds[i] == tid) {
         id = i;
         break;
      }
   }

   if(tid == async_test) {
      printf("Thread %d [tid %lu] - oneTimeCodeAsync completed successfully\n",
             id, tid);
      return;
   }
   else if(async_test != 0)
      fprintf(stderr, 
              "%s[%d]: ERROR: Thread %d [tid %lu] - mistakenly ran oneTimeCodeAsync for thread with tid %lu\n", 
              __FILE__, __LINE__, id, tid, async_test);
   else
      fprintf(stderr, 
              "%s[%d]: ERROR: Thread %d [tid %lu] - async_test is 0\n", 
              __FILE__, __LINE__, id, tid);
   async_failure++;
}

void thr_loop(int id, pthread_t tid)
{
   unsigned long timeout = 0;
   while( (! ok_to_exit[id]) && (timeout != 50000000) ) {
      timeout++;
      sched_yield();
   }
   if(timeout == 50000000)
      fprintf(stderr, 
              "%s[%d]: ERROR: Thread %d [tid %lu] - timed-out in thr_loop\n", 
              __FILE__, __LINE__, id, (unsigned long)tid);
}

void thr_func(void *arg)
{
   unsigned busy_work = 0;
   int id = *((int*)arg);
   pthread_t tid = pthread_self();
   thr_loop(id, tid);
   while(++busy_work != UINT_MAX/10);
   thr_exits++;
}

void *init_func(void *arg)
{
   assert(arg != NULL);
   thr_func(arg);
   return NULL;
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
   void *ret_val;
   char c = 'T';
   
   pthread_attr_t attr;
   pthread_attr_init(&attr);
   pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

   thr_exits = 0;

   parse_args(argc, argv);

   if (attached_fd) {
      if (write(attached_fd, &c, sizeof(char)) != sizeof(char)) {
         fprintf(stderr, "*ERROR*: Writing to pipe\n");
         exit(-1);
      }
      close(attached_fd);
      sleep(5); /* wait for mutator to attach */
   }
   
   /* create the workers */
   for (i=1; i<NTHRD; i++)
   {
      pthread_create(&(thrds[i]), &attr, init_func, &(thr_ids[i]));
   }
   thrds[0] = pthread_self();

   /* give time for workers to run thr_loop */
   while(thr_exits == 0)
      sched_yield();

   /* wait for worker exits */
   for (i=1; i<NTHRD; i++)
   {
      pthread_join(thrds[i], &ret_val);
   }
   
   if(sync_failure)
      fprintf(stderr, 
              "%s[%d]: ERROR: oneTimeCode failed for %d threads\n", 
              __FILE__, __LINE__, sync_failure);
   if(async_failure)
      fprintf(stderr, 
              "%s[%d]: ERROR: oneTimeCodeAsync failed for %d threads\n", 
              __FILE__, __LINE__, async_failure);

   /* let mutator do final work after noticing all workers exit */
   sleep(5);

   return 0;
}
