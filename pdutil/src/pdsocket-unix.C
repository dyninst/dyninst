#include <fcntl.h>
#include <errno.h>
#include "pdutil/h/pdsocket.h"

bool
PdSocket::IsBad( void ) const
{
    errno = 0;
    return ((fcntl( s, F_GETFL ) == -1) && (errno == EBADF));
}

