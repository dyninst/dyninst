#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include "../pthread_sync.h"
#include "../../h/thread.h"

#define	THR_SUCCEEDED(x)	((x) >= THR_OKAY)

#define NUM_ITERS 800000
#define PP_PER_ITER 4
#define MILLION 1000000.0

thread_t tfoo, tbar, tmain = 1;

volatile int s;
volatile int r;
volatile int S;
volatile int R;

tag_t READY   = MSG_TAG_USER;
tag_t SENDTO  = MSG_TAG_USER+1;
tag_t PING    = MSG_TAG_USER+2;
tag_t PONG    = MSG_TAG_USER+3;

static void*
foo(void* arg)
{
    int i;
    unsigned size = sizeof(tag_t);
    
    tag_t ack = SENDTO;

    thread_t send_to, ost;

    struct timeval  tstart;
    struct timeval  tend;
    double mlat, usecs;

    msg_send(1, READY, 0, 0);
    msg_recv(&tmain, &ack, &send_to, &size);

    fprintf(stderr, "foo sending to %d\n", send_to);    

    gettimeofday( &tstart, NULL );

    for (i = 0; i < NUM_ITERS; i++) {
        msg_send(send_to, PING, 0, 0);
        msg_recv(&send_to, &PONG, 0, 0);
        msg_send(send_to, PING, 0, 0);
	msg_recv(&send_to, &PONG, 0, 0);
	msg_send(send_to, PING, 0, 0);
	msg_recv(&send_to, &PONG, 0, 0);
	msg_send(send_to, PING, 0, 0);
	msg_recv(&send_to, &PONG, 0, 0);
    }


    gettimeofday( &tend, NULL );

    usecs = (tend.tv_sec * MILLION + tend.tv_usec) -
        (tstart.tv_sec * MILLION + tstart.tv_usec);

    mlat = usecs / (double(NUM_ITERS) * PP_PER_ITER);

    fprintf(stderr, "%d send/recv pairs; average latency is %g us\n", NUM_ITERS * PP_PER_ITER, mlat);
    
    return 0;
}


static void*
bar(void* arg)
{
    int i;
    tag_t ack = SENDTO;
    unsigned size = sizeof(tag_t);

    thread_t send_to, ost;    

    msg_send(1, READY, 0, 0);
    msg_recv(&tmain, &ack, &send_to, &size);

    fprintf(stderr, "bar sending to %d\n", send_to);
    
    for (i = 0; i < NUM_ITERS; i++) {
        msg_recv(&send_to, &PING, 0, 0);
        msg_send(send_to, PONG, 0, 0);
        msg_recv(&send_to, &PING, 0, 0);
        msg_send(send_to, PONG, 0, 0);
        msg_recv(&send_to, &PING, 0, 0);
        msg_send(send_to, PONG, 0, 0);
	msg_recv(&send_to, &PING, 0, 0);
	msg_send(send_to, PONG, 0, 0);
    }

    return 0;
}

int
main(void)
{
//    thr_name("main");

    tag_t tag = READY;

    tmain = thr_self();

    thr_create(0, 0, foo, 0, 0, &tfoo);
    thr_create(0, 0, bar, 0, 0, &tbar);

    msg_recv(&tfoo, &tag, 0, 0);
    msg_recv(&tbar, &tag, 0, 0);
    
    msg_send(tfoo, SENDTO, &tbar, sizeof(tbar));
    msg_send(tbar, SENDTO, &tfoo, sizeof(tfoo));

    thr_join(tbar, 0, 0);
    thr_join(tfoo, 0, 0);

    return 0;
}
