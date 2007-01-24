/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

// $Id: SharedObject.h,v 1.4 2007/01/24 19:33:50 darnold Exp $
#ifndef XPLAT_SHARED_OBJECT_H
#define XPLAT_SHARED_OBJECT_H

#include <string.h>


namespace XPlat
{

class SharedObject
{
private:
    char* path;

protected:
    SharedObject( const char* _path )
      : path( NULL )
    {
        if( _path != NULL )
        {
            path = new char[strlen(_path)+1];
            strcpy( path, _path );
        }
    }

public:
    static SharedObject* Load( const char* path );
    static const char* GetErrorString( void );

    virtual ~SharedObject( void )               { delete[] path; }

    virtual const char* GetPath( void ) const   { return path; }
    virtual void* GetSymbol( const char* name ) const = 0;
};

} // namespace XPlat

#endif // XPLAT_SHARED_OBJECT_H
