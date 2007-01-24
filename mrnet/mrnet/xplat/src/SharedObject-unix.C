/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

// $Id: SharedObject-unix.C,v 1.3 2007/01/24 19:34:22 darnold Exp $
#include <stdlib.h>
#include "SharedObject-unix.h"

namespace XPlat
{

UnixSharedObject::UnixSharedObject( const char* path )
  : SharedObject( path ),
    handle( NULL )
{
    // try to load the shared object
    handle = dlopen( path, RTLD_LAZY | RTLD_GLOBAL );
}

SharedObject*
SharedObject::Load( const char* path )
{
    UnixSharedObject* ret = new UnixSharedObject( path );
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

