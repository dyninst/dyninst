#ifndef __libthread_thr_mailbox_h__
#define __libthread_thr_mailbox_h__

#include "message.h"
#include "mailbox.h"
#include "refarray.C"
#include "dllist.C"
#include "predicate.h"
#include "DummyMonitor.h"
#include "MsgAvailablePipe.h"
#include "../h/thread.h"


namespace pdthr
{

class WaitSet;

class thr_mailbox : public mailbox
{
	friend class SocketWaitSetPopulator;
	friend class FileWaitSetPopulator;
	friend class SocketReadySetPopulator;
	friend class FileReadySetPopulator;

private:
    MsgAvailablePipe msg_avail_pipe;
    

    // sets of bound sockets, ready-to-read sockets, and a mutex 
    // for access control
    dllist<PdSocket,DummyMonitor>* bound_socks;
    dllist<PdSocket,DummyMonitor>* ready_socks;
    XPlat::Mutex sockq_mutex;

    // sets of bound files, ready-to-read files, and a mutex 
    // for access control
    dllist<PdFile,DummyMonitor>* bound_files;
    dllist<PdFile,DummyMonitor>* ready_files;
    XPlat::Mutex fileq_mutex;
    
    dllist<message*,DummyMonitor>* messages;
    dllist<message*,DummyMonitor>* sock_messages;
    dllist<message*,DummyMonitor>* file_messages;
    
    bool check_for(thread_t* sender, tag_t* type,
                          bool do_block = false, bool do_yank = false,
                          message** m = NULL, unsigned io_first = 1);
	void wait_for_input( void );

	void raise_msg_avail( void );
	void clear_msg_avail( void );
    
	bool is_buffered_special_ready( thread_t* sender, tag_t* type );

protected:
    virtual void populate_wait_set( WaitSet* wset );
    virtual void handle_wait_set_input( WaitSet* wset );

public:
    thr_mailbox(thread_t owner);
    virtual ~thr_mailbox();

    /* put() delivers message m into this mailbox */
    virtual int put(message* m);

    /* put_sock() delivers socket message m into this mailbox */
    virtual int put_sock(message* m);

    /* put_file() delivers file message m into this mailbox */
    virtual int put_file(message* m);

    /* recv() receives mail from this mailbox into buf;
       see the libthread documentation for the full
       meanings of the args */
    virtual int recv(thread_t* sender, tag_t* tagp, void* buf, unsigned* countp);

    /* poll() checks for suitable available messages */
    virtual int poll(thread_t* from,
                        tag_t* tagp,
                        unsigned block,
                        unsigned fd_first=0);

    void bind( PdFile fd,
                    unsigned special,
                    int (*wb)(void*), void* arg,
                    thread_t* ptid);
    void unbind( PdFile fd, bool getlock = true );
    bool is_bound( PdFile fd );

    void bind( PdSocket s,
                    unsigned special,
                    int (*wb)(void*),
                    void* arg,
                    thread_t* ptid);
    void unbind( PdSocket s, bool getlock = true );
    bool is_bound( PdSocket s);

#if defined(i386_unknown_nt4_0)
    void bind_wmsg( void );
    void unbind_wmsg( void );
    bool is_wmsg_bound( void );
#endif // defined(i386_unknown_nt4_0)

    void clear_ready( PdSocket sock );
    void clear_ready( PdFile fd );
    
    virtual void dump_state();
};

} // namespace pdthr

#endif


