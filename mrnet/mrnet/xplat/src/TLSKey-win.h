/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: TLSKey-win.h,v 1.1 2003/11/14 19:27:04 pcroth Exp $
#ifndef XPLAT_WINTLSKEY_H
#define XPLAT_WINTLSKEY_H

#include <windows.h>
#include "xplat/TLSKey.h"

namespace XPlat
{

class WinTLSKeyData : public TLSKey::Data
{
private:
    DWORD index;
    bool initialized;

public:
    WinTLSKeyData( void );
    virtual ~WinTLSKeyData( void );

    virtual void* Get( void ) const
    {
        return TlsGetValue( index );
    }

    virtual int Set( void* data )
    {
        return (TlsSetValue( index, data ) ? 0 : -1);
    }
};

} // namespace XPlat

#endif // XPLAT_WINTLSKEY_H
