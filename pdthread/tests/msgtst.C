#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include "../pthread_sync.h"
#include "../../h/thread.h"

#define	THR_SUCCEEDED(x)	((x) >= THR_OKAY)
#define FOO_READY 0
#define BAR_READY 1

thread_t tfoo, tbar;
pthread_sync monitor;

tag_t START  = MSG_TAG_USER;
tag_t FIRST  = MSG_TAG_USER+1;
tag_t SECOND = MSG_TAG_USER+2;
tag_t END    = MSG_TAG_USER+3;

static void*
foo(void* arg)
{
    unsigned msg1, msg2;
    thread_t tid;
    tag_t tag;
    int err;
    
    fprintf(stderr,"thread foo with arg %p\n", arg);
//    thr_name("foo");

    monitor.lock();
    monitor.wait(BAR_READY);
    monitor.unlock();

    tid = THR_TID_UNSPEC;
    tag = START;
    fprintf(stderr, "foo: receiving START from anyone...\n");    
    msg_recv(&tid, &tag, 0, 0);
    fprintf(stderr, "foo: received START from %d\n", tid);
        
    msg1 = 10; msg2 = 20;
    fprintf(stderr, "foo: sending FIRST to %d...\n", tbar);    
    err = msg_send(tbar, FIRST, &msg1, sizeof msg1);
    fprintf(stderr, "foo: sent FIRST to %d!\n", tbar);    
    if( !THR_SUCCEEDED( err ) )
    {
        fprintf(stderr, "MSG_SEND FAILED FOR FOO!!!!\n\n\n");
        return 0;
    }
    fprintf(stderr, "foo: sending SECOND to %d...\n", tbar);    
    err = msg_send(tbar, SECOND, &msg2, sizeof msg2);
    fprintf(stderr, "foo: sent SECOND to %d!\n", tbar);    
    if( !THR_SUCCEEDED( err ) )
    {
        fprintf(stderr, "msg2");
        return 0;
    }
    
    return 0;
}

static void*
bar(void* arg)
{
    thread_t frm;
    tag_t    tag;
    unsigned msg;
    unsigned size;
    int err;

    fprintf(stderr,"thread bar with arg %p\n", arg);
//    thr_name("bar");

    monitor.signal(BAR_READY);

    frm = THR_TID_UNSPEC;
    tag = SECOND;
    size = sizeof msg;
    err = msg_poll(&frm, &tag, 1);
    fprintf(stderr,"frm=%u, tag=%u\n", frm, tag);
    err = msg_recv(&frm, &tag, &msg, &size);
    fprintf(stderr,"frm=%u, tag=%u, msg=%u, size=%u\n", frm, tag, msg, size);
    
    frm = THR_TID_UNSPEC;
    tag = FIRST;
    size = sizeof msg;
    err = msg_poll(&frm, &tag, 1);
    fprintf(stderr,"frm=%u, tag=%u\n", frm, tag);
    err = msg_recv(&frm, &tag, &msg, &size);
    fprintf(stderr,"frm=%u, tag=%u, msg=%u, size=%u\n", frm, tag, msg, size);
    
    return 0;
}

int
main(void)
{
//    thr_name("main");
    monitor.register_cond(FOO_READY);
    monitor.register_cond(BAR_READY);
    
    thr_create(0, 0, foo, 0, 0, &tfoo);
    thr_create(0, 0, bar, 0, 0, &tbar);

    fprintf(stderr, "sending START to foo\n");
    msg_send(tfoo, START, 0, 0);
    fprintf(stderr, "START sent to foo\n");
    thr_join(tbar, 0, 0);
    thr_join(tfoo, 0, 0);

    return 0;
}
