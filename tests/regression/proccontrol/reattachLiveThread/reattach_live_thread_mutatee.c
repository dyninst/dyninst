/*
 * A worker thread that stays alive, blocked in pause(), while the mutator
 * detaches with Process::temporaryDetach() and attaches again with
 * Process::reAttach(). The process never exits by itself; the mutator kills
 * it. The alarm ends it if the mutator dies first.
 */
#include <pthread.h>
#include <stddef.h>
#include <unistd.h>

static void *worker(void *arg) {
  (void)arg;
  for (;;) {
    pause();
  }
  return NULL;
}

int main(void) {
  pthread_t t;
  alarm(120);
  if (pthread_create(&t, NULL, worker, NULL) != 0) {
    return 1;
  }
  pthread_join(t, NULL);
  return 0;
}
