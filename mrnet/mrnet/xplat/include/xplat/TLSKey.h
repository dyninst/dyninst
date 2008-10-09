/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

// $Id: TLSKey.h,v 1.6 2008/10/09 19:54:08 mjbrim Exp $
#ifndef XPLAT_TLSKEY_H
#define XPLAT_TLSKEY_H

#include "xplat/Monitor.h"
namespace XPlat
{

class TLSKey
{
public:
    class Data
    {
    public:
        Data( void ) { }
        virtual ~Data( void ) { }
        virtual void* Get( void ) const = 0;
        virtual int Set( void* val ) = 0;
    };

private:
    Data* data;
    mutable XPlat::Monitor data_sync;

public:
    TLSKey( void );
    virtual ~TLSKey( void ) {
        data_sync.Lock();
        delete data;
        data = NULL;
        data_sync.Unlock();
    }
    virtual void* Get( void ) const {
        void* ret = NULL;
        data_sync.Lock();
        if( data != NULL )
            ret = data->Get();
        data_sync.Unlock();
        return ret;
    }
    virtual int Set( void* val ) {
        int ret = 0;
        data_sync.Lock();
        if( data != NULL )
            ret = data->Set( val );
        data_sync.Unlock();
        return ret;
    }
};

} // namespace XPlat

#endif // XPLAT_TLSKEY_H
