#include <windows.h>
#include "pdutil/h/pddesc.h"

bool
PdFile::IsBad( void ) const
{
    DWORD dwflags = 0;
    return (GetHandleInformation( fd, &dwflags ) == FALSE);
}

