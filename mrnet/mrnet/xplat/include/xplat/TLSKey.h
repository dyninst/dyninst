/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: TLSKey.h,v 1.2 2004/03/23 01:12:22 eli Exp $
#ifndef XPLAT_TLSKEY_H
#define XPLAT_TLSKEY_H

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
        virtual void* Get( void ) const = NULL;
        virtual int Set( void* val ) = NULL;
    };

private:
    Data* data;

public:
    TLSKey( void );
    virtual ~TLSKey( void )         { delete data; }

    virtual void* Get( void ) const { return data->Get(); }
    virtual int Set( void* val )    { return data->Set( val ); }
};

} // namespace XPlat

#endif // XPLAT_TLSKEY_H
