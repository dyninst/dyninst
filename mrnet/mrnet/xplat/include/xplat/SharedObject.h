/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: SharedObject.h,v 1.2 2004/03/23 01:12:22 eli Exp $
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
    virtual void* GetSymbol( const char* name ) const = NULL;
};

} // namespace XPlat

#endif // XPLAT_SHARED_OBJECT_H
