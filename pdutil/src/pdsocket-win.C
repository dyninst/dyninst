#include "pdutil/h/pdsocket.h"

bool
PdSocket::IsBad( void ) const
{
    DWORD dwflags = 0;
    return (GetHandleInformation( (HANDLE)s, &dwflags ) == FALSE);
}

