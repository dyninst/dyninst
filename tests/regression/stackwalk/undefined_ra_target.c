/* Walked by undefined_ra_check. The worker blocks in poll(2), so its stack
 * ends in glibc's thread_start, whose CFI marks the return address undefined.
 * The main thread blocks in pthread_join, so its stack ends in _start. */
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
