/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include "../pthread_sync.h"
#include "../../h/thread.h"

#define	THR_SUCCEEDED(x)	((x) >= THR_OKAY)

#define NUM_ITERS 1000000
#define PP_PER_ITER 1
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
