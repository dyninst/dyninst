#ifndef __libthread_wmsg_q_h__
#define __libthread_wmsg_q_h__

#ifndef __in_thrtabentries__
#error You should not include this file directly
#endif

#if !defined(i386_unknown_nt4_0)
#error fatal: compiling windows message queue support on non-windows
#endif

class wmsg_q : public entity
{
public:
    virtual item_t gettype() { return item_t_wmsg; }
}; /* end of class tid_wmsg */

#endif 
