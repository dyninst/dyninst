#ifndef __libthread_thrtabentries_h__
#define __libthread_thrtabentries_h__
#define __in_thrtabentries__

#include "rwlock.h"
#include "mailbox.h"
#include "../h/thread.h"

/*
  This file includes interfaces for the types of entities that can be
  stored in the thread table.  Currently, there are three such types
  of entities on UNIX and four on Windows.  The three common types are
  lwp, which is a runnable thread of execution, file_q, which is
  a tid bound to a file-based message source, and socket_q, which is
  a tid bound to a socket-based message source.

  Windows also adds wmsg_q, which is a Windows message queue.
*/

#include "entity.h"
#include "io_entity.h"
#include "lwp.h"
#include "file_q.h"
#include "socket_q.h"

#if defined(i386_unknown_nt4_0)
#include "wmsg_q.h"
#endif /* i386/NT */

#undef __in_thrtabentries__
#endif
