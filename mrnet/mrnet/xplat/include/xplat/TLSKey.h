/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

// $Id: TLSKey.h,v 1.5 2007/01/24 19:33:52 darnold Exp $
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
        data_sync.Unlock();
    }
    virtual void* Get( void ) const {
        data_sync.Lock();
        void * ret = data->Get();
        data_sync.Unlock();
        return ret;
    }
    virtual int Set( void* val ) {
        data_sync.Lock();
        int ret = data->Set( val );
        data_sync.Unlock();
        return ret;
    }
};

} // namespace XPlat

#endif // XPLAT_TLSKEY_H
