/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

// $Id: SharedObject-unix.C,v 1.4 2007/03/20 23:20:21 darnold Exp $
#include <stdlib.h>
#include "SharedObject-unix.h"

namespace XPlat
{

UnixSharedObject::UnixSharedObject( const char* ipath )
  : SharedObject( ipath ),
    handle( NULL )
{
    // try to load the shared object
    handle = dlopen( ipath, RTLD_LAZY | RTLD_GLOBAL );
}

SharedObject*
SharedObject::Load( const char* ipath )
{
    UnixSharedObject* ret = new UnixSharedObject( ipath );
    if( ret->GetHandle() == NULL )
    {
        // we failed to open the given library
        delete ret;
        ret = NULL;
    }

    return ret;
}

const char*
SharedObject::GetErrorString( void )
{
    return dlerror();
}

} // namespace XPlat

