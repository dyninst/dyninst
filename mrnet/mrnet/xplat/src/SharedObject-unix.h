/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: SharedObject-unix.h,v 1.1 2003/11/14 19:27:04 pcroth Exp $
#ifndef XPLAT_SHARED_OBJECT_UNIX_H
#define XPLAT_SHARED_OBJECT_UNIX_H

#include <dlfcn.h>
#include "xplat/SharedObject.h"

namespace XPlat
{

class UnixSharedObject : public SharedObject
{
private:
    friend SharedObject* SharedObject::Load( const char* );

    void* handle;

    UnixSharedObject( const char* _path );

public:
    virtual ~UnixSharedObject( void )
    {
        if( handle != NULL )
        {
            dlclose( handle );
        }
    }

    virtual void* GetHandle( void ) const   { return handle; }

    virtual void* GetSymbol( const char* sym ) const
    {
        return dlsym( handle, sym );
    }
};

} // namespace XPlat

#endif // XPLAT_SHARED_OBJECT_UNIX_H
