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

#ifndef __libthread_thrtab_h__
#define __libthread_thrtab_h__

#include "../h/thread.h"
#include "refarray.h"
#include "dllist.C"
#include "thrtab_entries.h"
#include "hashtbl.C"
#include "xplat/Monitor.h"

namespace pdthr
{

class thrtab {
  private:
    static refarray<entity> entries;
    static hashtbl<entity*,thread_t> entity_registry;
    static dllist<thread_t> joinables;
    static unsigned initialized;
    static unsigned create_main_thread();
    
  public:
    static thread_t create_thread(lwp::task_t func,
                                  lwp::value_t arg,
                                  bool start);
    static thread_t create_file( PdFile fd, thread_t owner,
                                int (*will_block_func)(void*), void* desc,
                                bool is_special);
    static thread_t create_socket( PdSocket sock, thread_t owner,
                                  int (*will_block_func)(void*), void* desc,
                                  bool is_special);
#if defined(i386_unknown_nt4_0)
    static thread_t create_wmsg( thread_t owned_by );
#endif // defined(i386_unknown_nt4_0)
    
    static entity* get_entry(thread_t tid);
    static thread_t get_tid(entity* e);
    static void remove(thread_t tid);
    static bool is_valid(thread_t tid) {
        return entries[tid] != NULL;
    }
    static unsigned size() {
        return entries.capacity();
    }
    static bool is_io_entity(thread_t tid);
    static void register_joinable(thread_t tid);
    static void unregister_joinable(thread_t tid);
    static thread_t get_any_joinable();
};

} // namespace pdthr

#endif /*  __libthread_thrtab_h__ */
