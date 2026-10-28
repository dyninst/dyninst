/* Walked by undefined_ra_check. The worker writes one byte once it is
 * running, then loops in poll(2); the main thread waits in pthread_join. The
 * checker may walk either thread before it gets there, but the worker's stack
 * always ends in glibc's thread_start, whose CFI marks the return address
 * undefined, and the main thread's in _start. */
#include <poll.h>
#include <pthread.h>
#include <unistd.h>

static int ready_fd;

static void *worker(void *arg) {
    (void)arg;
    char c = 'r';
    if (write(ready_fd, &c, 1) != 1)
        return NULL;
    for (;;)
        poll(NULL, 0, 1000);
    return NULL;
}

int main(void) {
    ready_fd = STDOUT_FILENO;
    pthread_t t;
    if (pthread_create(&t, NULL, worker, NULL) != 0)
        return 1;
    pthread_join(t, NULL);
    return 0;
}
