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

#include "../h/thread.h"
#include "thrtab.h"
#include "thrtab_entries.h"
#include "message.h"
#include "mailbox.h"
#include "thr_mailbox.h"

#if defined(i386_unknown_nt4_0)
#include "win_thr_mailbox.h"
#endif // defined(i386_unknown_nt4_0)

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

using namespace pdthr;

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
        
        if(!recipient) {
            fprintf(stderr, "can't send message [sender = %d, tid = %d, tag = %d]\n", sender, tid, tag);
            assert (recipient);
        }
        
        ret = recipient->put(m);
    }

    thr_debug_msg(CURRENT_FUNCTION, "returning %d\n", ret);
    return ret;
}

int msg_poll(thread_t* tid, tag_t* tag, unsigned block, pollcallback_t pcb) {
    COLLECT_MEASUREMENT(THR_MSG_POLL);

    thr_debug_msg(CURRENT_FUNCTION, "tid = %d, tag = %d, block = %d\n", *tid, *tag, block);

    mailbox* mbox = lwp::get_mailbox();
    int ret = mbox->poll(tid, tag, block, 0, pcb);

    thr_debug_msg(CURRENT_FUNCTION, "returning %d; tid = %d, tag = %d\n", ret, *tid, *tag);
    return ret;
}

int msg_poll_preference(thread_t* tid, tag_t* tag, unsigned block, 
			unsigned fd_first, pollcallback_t pcb) {
    COLLECT_MEASUREMENT(THR_MSG_POLL);

    thr_debug_msg(CURRENT_FUNCTION, "tid = %d, tag = %d, block = %d, fd_first = %d\n", *tid, *tag, block, fd_first);

    mailbox* mbox = lwp::get_mailbox();
    int ret = mbox->poll(tid, tag, block, fd_first, pcb);


    thr_debug_msg(CURRENT_FUNCTION, "returning %d; tid = %d, tag = %d\n", ret, *tid, *tag);
    return ret;
}


int msg_recv(thread_t* tid, tag_t* tag, void* buf, unsigned* bufsize) {
    COLLECT_MEASUREMENT(THR_MSG_RECV);

    thr_debug_msg(CURRENT_FUNCTION,
        "tid = %d, tag = %d, buf = %p, bufsize = %d\n",
        *tid, *tag,
        buf,
        (bufsize != NULL ? *bufsize : 0) );

    mailbox* mbox = lwp::get_mailbox();
    if(!mbox) {
        fprintf(stderr, "panic:  can't get mailbox for lwp %d\n", thr_self());
        assert(mbox);
    }
    
    int ret = mbox->recv(tid,tag,buf,bufsize);
    

    thr_debug_msg(CURRENT_FUNCTION,
                    "returning %d; tid = %d, tag = %d, count = %d\n",
                    ret,
                    *tid,
                    *tag,
                    (bufsize != NULL ? *bufsize : 0) );
    return ret;
}

int msg_recv(thread_t* tid, tag_t* tag, void** buf) {
    COLLECT_MEASUREMENT(THR_MSG_RECV);

    thr_debug_msg(CURRENT_FUNCTION,
        "tid = %d, tag = %d, buf = %p\n",
        *tid, *tag,
        buf);

    mailbox* mbox = lwp::get_mailbox();
    if(!mbox) {
        fprintf(stderr, "panic:  can't get mailbox for lwp %d\n", thr_self());
        assert(mbox);
    }
    
    int ret = mbox->recv(tid,tag,buf);
    

    thr_debug_msg(CURRENT_FUNCTION,
                    "returning %d; tid = %d, tag = %d\n",
                    ret,
                    *tid,
                    *tag);
    return ret;
}

int
msg_bind_file(PDDESC fd,
                    unsigned special,
                    int (*will_block)(void*),
                    void* arg,
                    thread_t* ptid)
{
    int ret = THR_OKAY;
    thr_debug_msg(CURRENT_FUNCTION,
        "fd = %d, special = %d, will_block = %p, arg = %p\n",
        fd, special, will_block, arg);

    thr_mailbox* my_mail = (thr_mailbox*)lwp::get_mailbox();
    thread_t me = thr_self();
    my_mail->bind( PdFile(fd), special, will_block, arg, ptid);

    assert(my_mail->is_bound( PdFile(fd) ));

    thr_debug_msg(CURRENT_FUNCTION, "returning %d\n", ret);
    return ret;
}

int
msg_bind_socket(PDSOCKET s,
                unsigned special,
                int (*will_block)(void*), void* arg, thread_t* ptid)
{
    thr_debug_msg(CURRENT_FUNCTION, "s = %d, special = %d, will_block = %p, arg = %p\n", s, special, will_block, arg);
    int ret = THR_OKAY;

    thr_mailbox* my_mail = (thr_mailbox*)lwp::get_mailbox();
    thread_t me = thr_self();
    my_mail->bind( PdSocket(s), special, will_block, arg, ptid);

    assert(my_mail->is_bound( PdSocket(s) ));

    thr_debug_msg(CURRENT_FUNCTION, "returning %d\n", ret);
    return ret;
}

#if defined(i386_unknown_nt4_0)
int
msg_bind_wmsg(thread_t* ptid)
{
    int ret = THR_OKAY;
    thr_debug_msg(CURRENT_FUNCTION, "\n" );

    win_thr_mailbox* my_mail = (win_thr_mailbox*)lwp::get_mailbox();
    my_mail->bind_wmsg( ptid );
    assert(my_mail->is_wmsg_bound());

    thr_debug_msg(CURRENT_FUNCTION, "returning %d\n", ret );
    return ret;
}
#endif /* defined(i386_unknown_nt4_0) */

int msg_unbind(thread_t tid)
{
    thr_debug_msg(CURRENT_FUNCTION, "tid = %d\n", tid);
    int ret = THR_OKAY;

    thr_mailbox* my_mail = (thr_mailbox*)lwp::get_mailbox();
    thread_t me = thr_self();

    if( thr_type( tid ) == item_t_socket )
    {
        PdSocket to_unbind = thr_socket(tid);
        if( to_unbind != INVALID_PDSOCKET)
        {
            my_mail->unbind( to_unbind );
        }
        else
        {
            ret = THR_ERR;
        }
    }
    else if( thr_type( tid ) == item_t_file )
    {
        PdFile to_unbind = thr_file( tid );
        if( to_unbind != INVALID_PDDESC )
        {
            my_mail->unbind( to_unbind );
        }
        else
        {
            ret = THR_ERR;
        }
    }
#if defined(i386_unknown_nt4_0)
    else if( thr_type( tid ) == item_t_wmsg )
    {
        win_thr_mailbox* my_wmail = (win_thr_mailbox*)my_mail;
        my_wmail->unbind_wmsg();
    }
#endif // defined(i386_unknown_nt4_0)
    else
    {
        // item wasn't a socket or a file - it shouldn't have been bound
        ret = THR_ERR;
    }
    
    thr_debug_msg(CURRENT_FUNCTION, "returning %d\n", ret);
    return ret;
}

