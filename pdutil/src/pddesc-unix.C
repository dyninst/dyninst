#include <fcntl.h>
#include <errno.h>
#include "pdutil/h/pddesc.h"

bool
PdFile::IsBad( void ) const
{
    errno = 0;
    return ((fcntl( fd, F_GETFL ) == -1) && (errno == EBADF));
}

