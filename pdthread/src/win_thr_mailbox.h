#ifndef __libthread_win_thr_mailbox_h__
#define __libthread_win_thr_mailbox_h__

#include "thr_mailbox.h"

namespace pdthr
{

class win_thr_mailbox : public thr_mailbox
{
private:
    thread_t bound_tid;

protected:
    virtual void populate_wait_set( WaitSet* wset );
    virtual void handle_wait_set_input( WaitSet* wset );

public:
    win_thr_mailbox(thread_t owner)
      : thr_mailbox( owner ),
        bound_tid( THR_TID_UNSPEC )
    { }
    virtual ~win_thr_mailbox( void ) { }

    void bind_wmsg( thread_t* ptid );
    void unbind_wmsg( void );
    bool is_wmsg_bound( void )      { return (bound_tid != THR_TID_UNSPEC); }
};

} // namespace pdthr

#endif // __libthread_win_thr_mailbox_h__

