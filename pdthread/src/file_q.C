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
#if !defined(i386_unknown_nt4_0)
#include <unistd.h>
#endif

namespace pdthr
{

hashtbl<PdFile,file_q*> file_q::file_registry("PdFile","file_q*","file_registry");

file_q::file_q(PdFile the_fd,
                thread_t owned_by,
                int (*will_block_func)(void*),
                void* desc,
                bool is_special)
  : io_entity(owned_by, will_block_func, desc, is_special), fd(the_fd)
{
    file_registry.put(the_fd, this);
}

file_q::~file_q() {
    file_registry.put(this->fd, NULL);
}

file_q*
file_q::file_from_desc( PdFile the_fd)
{
    return file_registry.get(the_fd);
}

int file_q::do_read(void* buf, unsigned bufsize, unsigned* count) {
    // this function completely taken from old libthread
    // --willb, 18 Feb 2002

    int ret;
    
#if defined(i386_unknown_nt4_0)
    DWORD bytes_written;
    if( ReadFile( fd.fd, buf, bufsize, &bytes_written, NULL ) ) {
        *count = (unsigned)bytes_written;
        ret = (int)*count;
    } else {
        ret = PDDESC_ERROR;
    }
#else // defined(i386_unkown_nt4_0)
    ret = read( fd.fd, buf, bufsize);
    if( ret != PDDESC_ERROR ) {
        *count = (unsigned)ret;
    }
#endif // defined(i386_unknown_nt4_0)
    return ( ret != PDDESC_ERROR) ? THR_OKAY : THR_ERR;    
}

int file_q::do_write(void* buf, unsigned bufsize, unsigned* count) {
    // this function completely taken from old libthread
    // --willb, 18 Feb 2002 
    int ret;

#if defined(i386_unknown_nt4_0)
    DWORD bytes_written;
    if( WriteFile( fd.fd, buf, bufsize, &bytes_written, NULL ) ) {
        *count = (unsigned)bytes_written;
        ret = (int)*count;
    } else {
        ret = PDDESC_ERROR;     
    }
#else
    ret = write( fd.fd, buf, bufsize );
    if( ret != PDDESC_ERROR ) {
        *count = (unsigned)ret;
    }
#endif // defined(i386_unknown_nt4_0)
    
    return ( ret != PDDESC_ERROR) ? THR_OKAY : THR_ERR;
}

} // namespace pdthr

