/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: TLSKey.h,v 1.3 2004/05/17 00:50:50 pcroth Exp $
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
        virtual void* Get( void ) const = 0;
        virtual int Set( void* val ) = 0;
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
