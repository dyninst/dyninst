#include "../h/thread.h"
#include "thrtab.h"
#include "thrtab_entries.h"
#include "message.h"
#include "mailbox.h"
#include "thr_mailbox.h"

#include <assert.h>
#include <stdio.h>

#if DO_DEBUG_LIBPDTHREAD_MSGS == 1
#define DO_DEBUG_LIBPDTHREAD 1
#else
#define DO_DEBUG_LIBPDTHREAD 0
#endif

#define CURRENT_FILE msg_C
#include "thr_debug.h"

#undef DO_DEBUG_LIBPDTHREAD

#if DO_LIBPDTHREAD_MEASUREMENTS == 1

#include <time.h>
#include <unistd.h>
#include <string.h>

long long to_from_table[128][128];
unsigned t_f_table_init = 0;

#endif

void msg_dump_stats() {
#if DO_LIBPDTHREAD_MEASUREMENTS == 1
    time_t current_time = time(NULL);
    char outfilename[256];
    FILE* out_FILE;
    snprintf(outfilename, 255, "msg-stats-%d.txt", current_time);
    
    out_FILE = fopen(outfilename, "w+");
    
    for(int i = 0; i < 128; i++) {
        for(int j = 0; j < 128; j++) {
            fprintf(out_FILE, "%llu,", to_from_table[i][j]);
        }
        fprintf(out_FILE, "\n");
        fflush(out_FILE);
    }
    
    fclose(out_FILE);
#endif /* DO_LIBPDTHREAD_MEASUREMENTS == 1 */
}

int msg_send(thread_t tid, tag_t tag, void* buf, unsigned size) {
    COLLECT_MEASUREMENT(THR_MSG_SEND);

    thr_debug_msg(CURRENT_FUNCTION, "tid = %d, tag = %d, buf = %p, size = %d\n", tid, tag, buf, size);

    thread_t sender = lwp::get_self();
    int ret;

#if DO_LIBPDTHREAD_MEASUREMENTS == 1

    if(t_f_table_init == 0) {
        for(int i = 0; i < 128; i++)
            for(int j = 0; j < 128; j++)
                to_from_table[i][j] == 0;
        t_f_table_init = 1;
    }

    if(sender < 128 && tid < 128)
        to_from_table[tid][sender]++;

#endif /* DO_LIBPDTHREAD_MEASUREMENTS == 1 */
    
    if(!thrtab::is_valid(tid)) {
        ret = THR_ERR;
    } else {
        
        message* m = new message(sender, tag, buf, size);
        mailbox* recipient = mailbox::for_thread(tid);
        
        assert(recipient);
        
        ret = recipient->put(m);
    }

    thr_debug_msg(CURRENT_FUNCTION, "returning %d\n", ret);
    return ret;
}

int msg_poll(thread_t* tid, tag_t* tag, unsigned block) {
    COLLECT_MEASUREMENT(THR_MSG_POLL);

    thr_debug_msg(CURRENT_FUNCTION, "tid = %d, tag = %d, block = %d\n", *tid, *tag, block);

    mailbox* mbox = lwp::get_mailbox();
    int ret = mbox->poll(tid, tag, block);

    thr_debug_msg(CURRENT_FUNCTION, "returning %d; tid = %d, tag = %d\n", ret, *tid, *tag);
    return ret;
}

int msg_poll_preference(thread_t* tid, tag_t* tag, unsigned block, unsigned fd_first) {
    COLLECT_MEASUREMENT(THR_MSG_POLL);

    thr_debug_msg(CURRENT_FUNCTION, "tid = %d, tag = %d, block = %d, fd_first = %d\n", *tid, *tag, block, fd_first);

    mailbox* mbox = lwp::get_mailbox();
    int ret = mbox->poll(tid, tag, block, fd_first);

  done:
    thr_debug_msg(CURRENT_FUNCTION, "returning %d; tid = %d, tag = %d\n", ret, *tid, *tag);
    return ret;
}


int msg_recv(thread_t* tid, tag_t* tag, void* buf, unsigned* bufsize) {
    COLLECT_MEASUREMENT(THR_MSG_RECV);

    thr_debug_msg(CURRENT_FUNCTION, "tid = %d, tag = %d, buf = %p, bufsize = %d\n", *tid, *tag, buf, *bufsize);

    mailbox* mbox = lwp::get_mailbox();
    if(!mbox) {
        fprintf(stderr, "panic:  can't get mailbox for lwp %d\n", thr_self());
        assert(mbox);
    }
    
    int ret = mbox->recv(tid,tag,buf,bufsize);
    
  done:
    thr_debug_msg(CURRENT_FUNCTION, "returning %d; tid = %d, tag = %d, count = %d\n", ret, *tid, *tag, *bufsize);
    return ret;
}

int msg_bind(PDDESC fd, unsigned special, int (*will_block)(void*), void* arg, thread_t* tid) {
    thr_debug_msg(CURRENT_FUNCTION, "fd = %d, special = %d, will_block = %p, arg = %p\n", fd, special, will_block, arg);

    fprintf(stderr,"binding files to message queues not implemented; aborting...\n");
    assert(false);
}

int msg_bind_sig(int sig, thread_t* tid) {
    fprintf(stderr,"binding signals to message queues not implemented; aborting...\n");
    assert(false);
}


int msg_bind_socket(PDSOCKET s, unsigned special, int (*will_block)(void*), void* arg, thread_t* ptid) {
    thr_debug_msg(CURRENT_FUNCTION, "s = %d, special = %d, will_block = %p, arg = %p\n", s, special, will_block, arg);
    int ret = THR_OKAY;

    thr_mailbox* my_mail = (thr_mailbox*)lwp::get_mailbox();
    thread_t me = thr_self();
    my_mail->bind_sock(s, special, will_block, arg, ptid);

    assert(my_mail->is_sock_bound(s));
  done:
    thr_debug_msg(CURRENT_FUNCTION, "returning %d\n", ret);
    return ret;
}

#if defined(i386_unknown_nt4_0)
int msg_bind_wmsg(thread_t* tid);
#endif /* defined(i386_unknown_nt4_0) */

int msg_unbind(thread_t tid) {
    thr_debug_msg(CURRENT_FUNCTION, "tid = %d\n", tid);
    int ret = THR_OKAY;
    PDSOCKET to_unbind = thr_socket(tid);

    if (to_unbind == INVALID_PDSOCKET) {
        ret = THR_ERR;
    } else {
        thr_mailbox* my_mail = (thr_mailbox*)lwp::get_mailbox();
        thread_t me = thr_self();
        my_mail->unbind_sock( to_unbind );
    }
    
  done:
    thr_debug_msg(CURRENT_FUNCTION, "returning %d\n", ret);
    return ret;
}
