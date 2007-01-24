/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

// $Id: TLSKey-win.C,v 1.3 2007/01/24 19:34:30 darnold Exp $
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

