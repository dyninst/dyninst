#include <assert.h>
#include "pdthread/h/thread.h"
#include "pdthread/src/wmsg_q.h"

#define CURRENT_FILE wmsg_q_C
#include "thr_debug.h"


namespace pdthr
{

int
wmsg_q::do_read( void* buf, unsigned int bufsize, unsigned int* count )
{
    thr_debug_msg(CURRENT_FUNCTION,
        "calling do_read on Windows message queue\n" );
    assert( false );
    return 0;
}

int
wmsg_q::do_write( void* buf, unsigned int bufsize, unsigned int* count )
{
    thr_debug_msg(CURRENT_FUNCTION,
        "calling do_read on Windows message queue\n" );
    assert( false );
    return 0;
}

} // namespace pdthr

