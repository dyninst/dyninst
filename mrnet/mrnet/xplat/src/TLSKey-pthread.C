/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: TLSKey-pthread.C,v 1.1 2003/11/14 19:27:04 pcroth Exp $
#include <assert.h>
#include "xplat/src/TLSKey-pthread.h"

namespace XPlat
{

TLSKey::TLSKey( void )
  : data( new PthreadTLSKeyData )
{
    // nothing else to do
}


PthreadTLSKeyData::PthreadTLSKeyData( void )
  : initialized( false )
{
    int cret = pthread_key_create( &key, NULL );
    if( cret == 0 )
    {
        initialized = true;
    }
}


PthreadTLSKeyData::~PthreadTLSKeyData( void )
{
    if( initialized )
    {
        pthread_key_delete( key );
    }
}

} // namespace XPlat

