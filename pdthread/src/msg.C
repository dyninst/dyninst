#include "../h/thread.h"
#include "thrtab.h"
#include "thrtab_entries.h"
#include "message.h"
#include "mailbox.h"
#include "thr_mailbox.h"

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>

#ifndef __GNUC__
#define __PRETTY_FUNCTION__ ((const char*)0)
#endif

#if DO_DEBUG_LIBPDTHREAD_MSGS == 1
inline static void thr_debug_msg(const char* func_name, const char* format, ...) {
    va_list ap;
    va_start(ap, format);

    static const char* preamble_format = "[tid: %d; func: %s] ";
    unsigned len = 2048 + strlen(preamble_format) + (func_name ? strlen(func_name) : 0) + strlen(format);
    char* real_format = 
        new char[len];
    unsigned ct = snprintf(real_format, 64, preamble_format, thr_self(), func_name);
    strncat(real_format, format, len - ct - 1);
    vfprintf(stderr, real_format, ap);
    delete [] real_format;
}
#else
inline static void thr_debug_msg(const char* func_name, const char* format, ...) { }
#endif

int msg_send(thread_t tid, tag_t tag, void* buf, unsigned size) {
    thr_debug_msg(__PRETTY_FUNCTION__, "tid = %d, tag = %d, buf = %p, size = %d\n", tid, tag, buf, size);

    thread_t sender = lwp::get_self();
    unsigned ret;
    
    if(!thrtab::is_valid(tid)) {
        ret = THR_ERR;
    } else {
        
        message* m = new message(sender, tag, buf, size);
        mailbox* recipient = mailbox::for_thread(tid);
        
        assert(recipient);
        
        ret = recipient->put(m);
    }

    thr_debug_msg(__PRETTY_FUNCTION__, "returning %d\n", ret);
    return ret;
}

int msg_poll(thread_t* tid, tag_t* tag, unsigned block) {

    thr_debug_msg(__PRETTY_FUNCTION__, "tid = %d, tag = %d, block = %d\n", tid, tag, block);

    mailbox* mbox = lwp::get_mailbox();
    int ret = mbox->poll(tid, tag, block);

    thr_debug_msg(__PRETTY_FUNCTION__, "returning %d\n", ret);
    return ret;
}

int msg_poll_preference(thread_t* tid, tag_t* tag, unsigned block, unsigned fd_first) {
    thr_debug_msg(__PRETTY_FUNCTION__, "tid = %d, tag = %d, block = %d, fd_first = %d\n", tid, tag, block, fd_first);

    mailbox* mbox = lwp::get_mailbox();
    int ret = mbox->poll(tid, tag, block, fd_first);

  done:
    thr_debug_msg(__PRETTY_FUNCTION__, "returning %d\n", ret);
    return ret;
}


int msg_recv(thread_t* tid, tag_t* tag, void* buf, unsigned* bufsize) {
    thr_debug_msg(__PRETTY_FUNCTION__, "tid = %d, tag = %d, buf = %p, bufsize = %d\n", *tid, *tag, buf, *bufsize);

    mailbox* mbox = lwp::get_mailbox();
    if(!mbox) {
        fprintf(stderr, "panic:  can't get mailbox for lwp %d\n", thr_self());
        assert(mbox);
    }
    
    int ret = mbox->recv(tid,tag,buf,bufsize);
    
  done:
    thr_debug_msg(__PRETTY_FUNCTION__, "returning %d\n", ret);
    return ret;
}

int msg_bind(PDDESC fd, unsigned special, int (*will_block)(void*), void* arg, thread_t* tid) {
    thr_debug_msg(__PRETTY_FUNCTION__, "fd = %d, special = %d, will_block = %p, arg = %p\n", fd, special, will_block, arg);

    fprintf(stderr,"binding files to message queues not implemented; aborting...\n");
    assert(false);
}


int msg_bind_sig(int sig, thread_t* tid) {
    fprintf(stderr,"binding signals to message queues not implemented; aborting...\n");
    assert(false);
}


int msg_bind_socket(PDSOCKET s, unsigned special, int (*will_block)(void*), void* arg, thread_t* tid) {
    thr_debug_msg(__PRETTY_FUNCTION__, "s = %d, special = %d, will_block = %p, arg = %p\n", s, special, will_block, arg);
    int ret = THR_OKAY;

    thr_mailbox* my_mail = (thr_mailbox*)lwp::get_mailbox();
    thread_t me = thr_self();
    my_mail->bind_sock(s, special, will_block, (void*)(&s), &me);
    
  done:
    thr_debug_msg(__PRETTY_FUNCTION__, "returning %d\n", ret);
    return ret;
}

#if defined(i386_unknown_nt4_0)
int msg_bind_wmsg(thread_t* tid);
#endif /* defined(i386_unknown_nt4_0) */

int msg_unbind(thread_t tid) {
    ;
    return -1;
}


