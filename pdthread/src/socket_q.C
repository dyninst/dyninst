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

#include "thrtab_entries.h"

namespace pdthr
{

hashtbl<PdSocket,socket_q*> socket_q::socket_registry("PdSocket","socket_q*","socket_registry");

socket_q::socket_q( PdSocket the_sock,
                    thread_t owned_by, 
                    int (*will_block_func)(void*),
                    void* desc,
                    bool is_special)
  : io_entity(owned_by, will_block_func, desc, is_special),
    sock(the_sock)
{
    socket_registry.put(the_sock, this);
}

socket_q::~socket_q() {
    socket_registry.put(this->sock, NULL);
}

socket_q* socket_q::socket_from_desc( const PdSocket& the_sock) {
    return socket_registry.get(the_sock);
}

int socket_q::do_read(void* buf, unsigned bufsize, unsigned* count) {
    // this function completely taken from old libthread
    // --willb, 18 Feb 2002
    int ret;
    
    ret = recv( sock.s, (char*)buf, bufsize, 0 );
    if( ret != PDSOCKET_ERROR ) {
        *count = (unsigned)ret;
    }
    return (ret != PDSOCKET_ERROR) ? THR_OKAY : PDSOCKET_ERRNO;
}

int socket_q::do_write(void* buf, unsigned bufsize, unsigned* count) {
    // this function completely taken from old libthread
    // --willb, 18 Feb 2002 
    int ret;

    ret = send( sock.s, (const char*)buf, bufsize, 0 );
    if( ret != PDSOCKET_ERROR ) {
        *count = (unsigned)ret;
    }
    return (ret != PDSOCKET_ERROR) ? THR_OKAY : PDSOCKET_ERRNO;
}

} // namespace pdthr
