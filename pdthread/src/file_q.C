#include "thrtab_entries.h" 
#if !defined(i386_unknown_nt4_0)
#include <unistd.h>
#endif

hashtbl<PDDESC,file_q*,pthread_sync> file_q::file_registry;

file_q::file_q(PDDESC the_fd, thread_t owned_by, int (*will_block_func)(void*), void* desc)
    // FIXME: don't take care of non-special files -- do we have to?
        : io_entity(owned_by, will_block_func, desc), fd(the_fd) {
    file_registry.put(the_fd, this);
}

file_q::~file_q() {
    file_registry.put(this->fd, NULL);
}

file_q* file_q::file_from_desc(PDDESC the_fd) {
    return file_registry.get(the_fd);
}

int file_q::do_read(void* buf, unsigned bufsize, unsigned* count) {
    // this function completely taken from old libthread
    // --willb, 18 Feb 2002

    int ret;
    
#if defined(i386_unknown_nt4_0)
    DWORD bytes_written;
    if( ReadFile( fd, buf, bufsize, &bytes_written, NULL ) ) {
        *count = (unsigned)bytes_written;
        ret = (int)*count;
    } else {
        ret = PDDESC_ERROR;
    }
#else // defined(i386_unkown_nt4_0)
    ret = read( fd, buf, bufsize);
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
    if( WriteFile( fd, buf, bufsize, &bytes_written, NULL ) ) {
        *count = (unsigned)bytes_written;
        ret = (int)*count;
    } else {
        ret = PDDESC_ERROR;     
    }
#else
    ret = write( fd, buf, bufsize );
    if( ret != PDDESC_ERROR ) {
        *count = (unsigned)ret;
    }
#endif // defined(i386_unknown_nt4_0)
    
    return ( ret != PDDESC_ERROR) ? THR_OKAY : THR_ERR;
}
