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
/************************************************************************
 * $Id: thread.h,v 1.15 2005/01/11 22:45:23 legendre Exp $
************************************************************************/
#if !defined(_thread_h_thread_h_)
#define _thread_h_thread_h_

/************************************************************************
 * header files.
************************************************************************/

#if !defined(i386_unknown_nt4_0)
#include <sys/param.h>
#else
#include <winsock2.h>
#include <windows.h>
#include <wtypes.h>
#endif /* !defined(i386_unknown_nt4_0) */

#include "pdutil/h/pdsocket.h"
#include "pdutil/h/pddesc.h"

    
/************************************************************************
 * constants.
************************************************************************/

#define THR_OKAY    0
#define THR_ERR     -1

/* message tag values */
#define	MSG_TAG_UNSPEC	0
#define	MSG_TAG_ANY		0
#define	MSG_TAG_THREAD	1
#define	MSG_TAG_FILE	2
#define	MSG_TAG_SIG		3
#define	MSG_TAG_SOCKET	4
#if defined(i386_unknown_nt4_0)
#define	MSG_TAG_WMSG	5
#endif /* i386_unknown_nt4_0) */
#define	MSG_TAG_USER	20

/* special thread ID values */
#define	THR_TID_UNSPEC	0


/************************************************************************
 * structures and types.
************************************************************************/

typedef enum {
	THR_SUSPENDED = 0x1
} Thr_Flags;

typedef enum {
	item_t_unknown,
	item_t_thread,
	item_t_file,
	item_t_signal,
	item_t_socket
#if defined(i386_unknown_nt4_0)
	,
	item_t_wmsg
#endif /* defined(i386_uknown_nt4_0) */
}	item_t;

typedef enum {
        action_read,
        action_write
}       action_t;

typedef enum {
        rwlock_favor_read,
        rwlock_favor_write
}       pref_t;


typedef unsigned thread_t;
typedef unsigned tag_t;
typedef void* thread_monitor_t;
typedef void* thread_rwlock_t;    
typedef void* thread_key_t;


/************************************************************************
 * function prototypes.
************************************************************************/

/* thread management functions */
int			thr_create(void* stack, unsigned stack_size, 
						void* (*func)(void*), void* arg, 
						unsigned thr_flags, thread_t* tid);
void		thr_exit(void* result);
int			thr_join(thread_t tid, thread_t* departed, void** result);
thread_t	thr_self(void);
thread_t	thr_parent(void);
const char*	thr_name(const char* name);
void		thr_yield(void);
int			thr_kill(thread_t tid, int sig);
int			thr_suspend(thread_t tid);
int			thr_continue(thread_t tid);

/* thread-local storage functions */
int			thr_keycreate(thread_key_t* key);
int			thr_setspecific(thread_key_t key, void* value);
int			thr_getspecific(thread_key_t, void** value);

/* message functions */
typedef bool (*pollcallback_t)(PDSOCKET sock);

int			msg_send(thread_t tid, tag_t tag, void* buf, unsigned size = 0);
int			msg_poll(thread_t* tid, tag_t* tag, unsigned block, pollcallback_t pcb = NULL);
int			msg_poll_preference(thread_t* tid, tag_t* tag, unsigned block, unsigned fd_first, pollcallback_t pcb = NULL);
int			msg_recv(thread_t* tid, tag_t* tag, void* buf, unsigned* bufsize);
int			msg_recv(thread_t* tid, tag_t* tag, void** buf);

int			msg_bind_file(PDDESC fd, unsigned special, int (*will_block)(void*), void* arg, thread_t* tid);
int			msg_bind_socket(PDSOCKET s, unsigned special, int (*will_block)(void*), void* arg, thread_t* tid);
#if defined(i386_unknown_nt4_0)
int			msg_bind_wmsg(thread_t* tid);
#endif /* defined(i386_unknown_nt4_0) */
int			msg_unbind(thread_t tid);

#if defined(THREADLIB_DEBUG)
void		msg_dump_state(void);
#endif // defined(THREADLIB_DEBUG)

/* run-time type and value discovery functions */
item_t		thr_type(thread_t tid);
PDDESC		thr_file(thread_t tid);
PDSOCKET	thr_socket(thread_t tid);

/* tracing functions */
void		thr_do_trace(const char* format, ...);
void		thr_trace_on(void);
void		thr_trace_off(void);

/* error handling and reporting functions */
void		thr_perror(const char* msg);
const char*	thr_strerror(void);

/* state maintenance functions */
void	clear_ready_sock( PDSOCKET sock );
void	clear_ready_file( PDDESC sock );

    
/*
  thread performance measurement collection/display types/functions/constants
*/

void thr_library_cleanup(void);
    
    
#if DO_LIBPDTHREAD_MEASUREMENTS == 1

struct thr_perf_data_t {
    unsigned long long num_lock_acquires;
    unsigned long long num_lock_blocks;

    unsigned long long lock_contention_time;
    unsigned long long lock_timer_start;
    
    unsigned long long num_msg_ops;
    unsigned long long msg_time;
    unsigned long long msg_timer_start;
};

void thr_collect_measurements (int);
    
#define THR_LOCK_ACQ 0
#define THR_LOCK_BLOCK 1
#define THR_LOCK_TIMER_START 20
#define THR_LOCK_TIMER_STOP 21
#define THR_MSG_SEND 4
#define THR_MSG_RECV 5
#define THR_MSG_POLL 6
#define THR_MSG_TIMER_START 30
#define THR_MSG_TIMER_STOP 31
    
#define COLLECT_MEASUREMENT(x) thr_collect_measurements(x)

#else
#define COLLECT_MEASUREMENT(x) while(0)
#endif

    
/* synchronization primitives */

thread_monitor_t thr_monitor_create();
int thr_monitor_destroy(thread_monitor_t mon);
int thr_monitor_enter(thread_monitor_t mon);
int thr_monitor_leave(thread_monitor_t mon);

int thr_cond_register(thread_monitor_t mon, unsigned cond_no);
int thr_cond_wait(thread_monitor_t mon, unsigned cond_no);
int thr_cond_signal(thread_monitor_t mon, unsigned cond_no);
int thr_cond_broadcast(thread_monitor_t mon, unsigned cond_no);

thread_rwlock_t thr_rwlock_create(pref_t pref);
int thr_rwlock_destroy(thread_rwlock_t rw);

int thr_rwlock_acquire(thread_rwlock_t rw, action_t action);
int thr_rwlock_release(thread_rwlock_t rw, action_t action);


/*
 * THREADrpc
 * 
 * The THREADrpc class is a base class for igen-generated RPC classes
 * that communicate via RPC.
 */

class THREADrpc {
public:
  THREADrpc(thread_t t) { tid = t; }
  // ~THREADrpc() { }
  void setTid(thread_t id) { tid = id; }
  thread_t getTid() const { return tid;}

  // see not on requestingThread, the use of this may be unsafe
  thread_t getRequestingThread() const { return requestingThread; }
  void setRequestingThread(thread_t t) { requestingThread = t;}
  thread_t net_obj() const { return tid;}

 private:
  thread_t tid;
  // these are only to be used by implmentors of thread RPCs.
  //   the value is only valid during a thread RPC.
  thread_t requestingThread;
};

#endif /* !defined(_thread_h_thread_h_) */
