/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: TLSKey-win.C,v 1.2 2004/03/23 01:12:23 eli Exp $
#include <assert.h>
#include "xplat/src/TLSKey-win.h"


namespace XPlat
{

TLSKey::TLSKey( void )
  : data( new WinTLSKeyData )
{
    // nothing else to do
}


WinTLSKeyData::WinTLSKeyData( void )
  : initialized( false )
{
    index = TlsAlloc();
    if( index != TLS_OUT_OF_INDEXES )
    {
        initialized = true;
    }
}


WinTLSKeyData::~WinTLSKeyData( void )
{
    if( initialized )
    {
        TlsFree( index );
    }
}

} // namespace XPlat

